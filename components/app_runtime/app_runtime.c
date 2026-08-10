#include "app_runtime.h"

#include "app_runtime_config.h"
#include "beacon_metrics.h"
#include "beacon_protocol.h"
#include "board_config.h"
#include "board_feedback.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sx1262_radio.h"

typedef enum {
    APP_EVENT_SET_STATE = 0,
    APP_EVENT_SHOW_STATUS,
    APP_EVENT_RADIO_ERROR,
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    board_state_t state;
    board_status_t status;
} app_event_t;

static const char *TAG = "IDS_BEACON";
static QueueHandle_t app_event_queue;

static void send_event(const app_event_t *event)
{
    if (xQueueSend(app_event_queue, event, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(TAG, "feedback event dropped: %u", (unsigned)event->type);
    }
}

static void send_status(uint32_t last_tx_sequence,
                        uint32_t last_rx_sequence,
                        uint32_t rx_interval_ms,
                        int16_t last_rssi)
{
    app_event_t event = {
        .type = APP_EVENT_SHOW_STATUS,
        .status = {
            .node_addr = BEACON_NODE_ID,
            .last_rssi_dbm = last_rssi,
            .last_rx_number = last_rx_sequence,
            .last_tx_number = last_tx_sequence,
            .packet_interval_ms = rx_interval_ms,
        },
    };
    send_event(&event);
}

static void run_api_error_self_test(void)
{
    const uint8_t probe = 0x00U;
    uint8_t rx_buffer[1] = { 0 };
    size_t rx_length = 0U;
    int16_t rssi = 0;
    int8_t snr = 0;
    bool pass = sx1262_radio_send(&probe, 1U, 0U) == ESP_ERR_INVALID_ARG &&
                sx1262_radio_receive(rx_buffer, sizeof(rx_buffer), &rx_length, 0U) == ESP_ERR_INVALID_ARG &&
                sx1262_radio_get_last_packet_metrics(&rssi, &snr) == RADIO_ERR_NOT_INITIALIZED;
    ESP_LOGI(TAG, "API ERROR SELF-TEST invalid-timeout/metrics-before-RX result=%s",
             pass ? "PASS" : "FAIL");
}

static void radio_task(void *arg)
{
    (void)arg;
    const radio_config_t radio_config = {
        .frequency_hz = BEACON_RADIO_FREQUENCY_HZ,
        .spreading_factor = BEACON_RADIO_SPREADING_FACTOR,
        .bandwidth_hz = BEACON_RADIO_BANDWIDTH_HZ,
        .coding_rate = BEACON_RADIO_CODING_RATE,
        .preamble_len = BEACON_RADIO_PREAMBLE_LEN,
        .tx_power_dbm = BEACON_RADIO_TX_POWER_DBM,
    };
    const board_config_t *board = board_config_get();
    const radio_hw_config_t hardware_config = {
        .spi_host = SPI2_HOST,
        .spi_clock_hz = BEACON_RADIO_SPI_CLOCK_HZ,
        .pin_mosi = board->radio_pins.mosi,
        .pin_miso = board->radio_pins.miso,
        .pin_sclk = board->radio_pins.sclk,
        .pin_nss = board->radio_pins.nss,
        .pin_reset = board->radio_pins.reset,
        .pin_busy = board->radio_pins.busy,
        .pin_dio1 = board->radio_pins.dio1,
        .use_dio2_rf_switch = true,
        .use_dio3_tcxo = true,
    };

    esp_err_t radio_ret = sx1262_radio_init(&hardware_config, &radio_config);
    if (radio_ret != ESP_OK) {
        ESP_LOGE(TAG, "SX1262 init failed: %s", esp_err_to_name(radio_ret));
        app_event_t error_event = { .type = APP_EVENT_RADIO_ERROR, .state = BOARD_STATE_ERROR };
        send_event(&error_event);
        vTaskDelete(NULL);
        return;
    }
    run_api_error_self_test();

    uint32_t tx_sequence = 1U;
    uint32_t last_tx_sequence = 0U;
    uint32_t last_rx_sequence = 0U;
    uint32_t rx_interval_ms = 0U;
    int16_t last_rssi = 0;
    int8_t last_snr = 0;
    int64_t previous_rx_us = 0;
    beacon_metrics_t link_metrics;
    beacon_metrics_reset(&link_metrics);
    int64_t next_tx_us = esp_timer_get_time() +
        (BEACON_NODE_ID == 0U ? 0LL : (int64_t)BEACON_NODE_1_OFFSET_MS * 1000LL);

    ESP_LOGI(TAG, "node=%u period=%u ms offset=%u ms power=%d dBm",
             BEACON_NODE_ID, BEACON_PERIOD_MS,
             BEACON_NODE_ID == 0U ? 0U : BEACON_NODE_1_OFFSET_MS,
             BEACON_RADIO_TX_POWER_DBM);

    while (true) {
        int64_t now_us = esp_timer_get_time();
        if (now_us >= next_tx_us) {
            beacon_message_t message = { .sender_id = BEACON_NODE_ID, .sequence = tx_sequence };
            uint8_t tx_buffer[BEACON_PROTOCOL_MAX_WIRE_LEN] = { 0 };
            size_t tx_length = 0U;
            beacon_protocol_result_t format_ret =
                beacon_protocol_format(&message, tx_buffer, sizeof(tx_buffer), &tx_length);

            app_event_t tx_event = { .type = APP_EVENT_SET_STATE, .state = BOARD_STATE_TX };
            send_event(&tx_event);
            if (format_ret == BEACON_PROTOCOL_OK &&
                sx1262_radio_send(tx_buffer, tx_length, BEACON_RADIO_OPERATION_TIMEOUT_MS) == ESP_OK) {
                last_tx_sequence = tx_sequence++;
                ESP_LOGI(TAG, "TX seq=%03lu payload=\"%.*s\" power=%d dBm",
                         (unsigned long)last_tx_sequence, (int)tx_length,
                         (const char *)tx_buffer, BEACON_RADIO_TX_POWER_DBM);
            } else {
                ESP_LOGW(TAG, "TX failed seq=%03lu", (unsigned long)tx_sequence);
            }

            do {
                next_tx_us += (int64_t)BEACON_PERIOD_MS * 1000LL;
            } while (next_tx_us <= esp_timer_get_time());
            send_status(last_tx_sequence, last_rx_sequence, rx_interval_ms, last_rssi);
            continue;
        }

        app_event_t rx_event = { .type = APP_EVENT_SET_STATE, .state = BOARD_STATE_RX };
        send_event(&rx_event);
        uint8_t rx_buffer[BEACON_PROTOCOL_MAX_WIRE_LEN] = { 0 };
        size_t rx_length = 0U;
        uint32_t timeout_ms = (uint32_t)((next_tx_us - now_us + 999LL) / 1000LL);
        esp_err_t rx_ret = sx1262_radio_receive(rx_buffer, sizeof(rx_buffer), &rx_length, timeout_ms);
        if (rx_ret == ESP_ERR_TIMEOUT) {
            continue;
        }
        if (rx_ret != ESP_OK) {
            ESP_LOGW(TAG, "RX failed: %s", esp_err_to_name(rx_ret));
            vTaskDelay(pdMS_TO_TICKS(BEACON_RX_IDLE_MS));
            continue;
        }

        beacon_message_t message;
        if (beacon_protocol_parse(rx_buffer, rx_length, &message) != BEACON_PROTOCOL_OK ||
            message.sender_id == BEACON_NODE_ID) {
            ESP_LOGW(TAG, "ignored invalid or local packet");
            continue;
        }

        int64_t rx_us = esp_timer_get_time();
        rx_interval_ms = previous_rx_us == 0 ? 0U : (uint32_t)((rx_us - previous_rx_us) / 1000LL);
        previous_rx_us = rx_us;
        last_rx_sequence = message.sequence;
        ESP_ERROR_CHECK_WITHOUT_ABORT(sx1262_radio_get_last_packet_metrics(&last_rssi, &last_snr));
        beacon_metrics_record(&link_metrics, message.sequence, last_rssi, last_snr, rx_interval_ms);
        ESP_LOGI(TAG, "RX sender=%u seq=%03lu dt=%lu ms RSSI=%d dBm SNR=%d dB",
                 message.sender_id, (unsigned long)message.sequence,
                 (unsigned long)rx_interval_ms, last_rssi, last_snr);
        send_status(last_tx_sequence, last_rx_sequence, rx_interval_ms, last_rssi);
        if ((link_metrics.received % BEACON_ACCEPTANCE_SAMPLE_COUNT) == 0U) {
            beacon_metrics_report_t report;
            beacon_metrics_get_report(&link_metrics, &report);
            bool pass = report.delivery_rate_permille >= BEACON_ACCEPTANCE_MIN_DELIVERY_PERMILLE &&
                        report.interval_mean_ms >= BEACON_ACCEPTANCE_MIN_INTERVAL_MS &&
                        report.interval_mean_ms <= BEACON_ACCEPTANCE_MAX_INTERVAL_MS;
            ESP_LOGI(TAG,
                     "ACCEPTANCE samples=%lu missing=%lu duplicates=%lu delivery=%lu.%lu%% "
                     "RSSI mean/min/max=%d/%d/%d dBm SNR mean/min/max=%d/%d/%d dB "
                     "DT mean/min/max=%lu/%lu/%lu ms result=%s",
                     (unsigned long)report.received,
                     (unsigned long)report.missing,
                     (unsigned long)report.duplicates_or_out_of_order,
                     (unsigned long)(report.delivery_rate_permille / 10U),
                     (unsigned long)(report.delivery_rate_permille % 10U),
                     report.rssi_mean_dbm, report.rssi_min_dbm, report.rssi_max_dbm,
                     report.snr_mean_db, report.snr_min_db, report.snr_max_db,
                     (unsigned long)report.interval_mean_ms,
                     (unsigned long)report.interval_min_ms,
                     (unsigned long)report.interval_max_ms,
                     pass ? "PASS" : "FAIL");
        }
        vTaskDelay(pdMS_TO_TICKS(BEACON_RX_IDLE_MS));
    }
}

static void feedback_task(void *arg)
{
    (void)arg;
    esp_err_t feedback_ret = board_feedback_init();
    while (true) {
        app_event_t event;
        if (xQueueReceive(app_event_queue, &event, pdMS_TO_TICKS(BEACON_FEEDBACK_PERIOD_MS)) == pdTRUE &&
            feedback_ret == ESP_OK) {
            if (event.type == APP_EVENT_SET_STATE) {
                ESP_ERROR_CHECK_WITHOUT_ABORT(board_feedback_set_state(event.state));
            } else if (event.type == APP_EVENT_SHOW_STATUS) {
                ESP_ERROR_CHECK_WITHOUT_ABORT(board_feedback_show_status(&event.status));
            } else if (event.type == APP_EVENT_RADIO_ERROR) {
                ESP_ERROR_CHECK_WITHOUT_ABORT(board_feedback_set_state(BOARD_STATE_ERROR));
            }
        }
        if (feedback_ret == ESP_OK) {
            ESP_ERROR_CHECK_WITHOUT_ABORT(board_feedback_tick());
        }
    }
}

esp_err_t app_runtime_start(void)
{
    if (app_event_queue != NULL) {
        return ESP_OK;
    }
    app_event_queue = xQueueCreate(BEACON_EVENT_QUEUE_LEN, sizeof(app_event_t));
    ESP_RETURN_ON_FALSE(app_event_queue != NULL, ESP_ERR_NO_MEM, TAG, "create event queue");
    ESP_RETURN_ON_FALSE(xTaskCreate(feedback_task, "feedback_task", BEACON_FEEDBACK_TASK_STACK_BYTES,
                                   NULL, BEACON_FEEDBACK_TASK_PRIORITY, NULL) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "create feedback task");
    ESP_RETURN_ON_FALSE(xTaskCreatePinnedToCore(radio_task, "radio_task", BEACON_RADIO_TASK_STACK_BYTES,
                                               NULL, BEACON_RADIO_TASK_PRIORITY, NULL,
                                               BEACON_RADIO_TASK_CORE) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "create radio task");
    return ESP_OK;
}
