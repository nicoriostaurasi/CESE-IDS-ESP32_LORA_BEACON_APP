#include "board_feedback.h"

#include "esp_check.h"
#include "esp_log.h"
#include "oled_display.h"
#include "status_led.h"

#include <stdio.h>

static const char *TAG = "BOARD_FEEDBACK";
static board_state_t current_state = BOARD_STATE_BOOT;
static bool status_screen_active = false;

static const char *state_to_string(board_state_t state)
{
    switch (state) {
    case BOARD_STATE_BOOT:
        return "BOOT";
    case BOARD_STATE_IDLE:
        return "IDLE";
    case BOARD_STATE_TX:
        return "TX";
    case BOARD_STATE_RX:
        return "RX";
    case BOARD_STATE_FORWARDING:
        return "FWD";
    case BOARD_STATE_ERROR:
        return "ERROR";
    case BOARD_STATE_TEST_RUNNING:
        return "TEST";
    default:
        return "UNKNOWN";
    }
}

static status_led_pattern_t led_pattern_for_state(board_state_t state)
{
    switch (state) {
    case BOARD_STATE_BOOT:
        return STATUS_LED_PATTERN_BOOT;
    case BOARD_STATE_IDLE:
        return STATUS_LED_PATTERN_IDLE;
    case BOARD_STATE_TX:
        return STATUS_LED_PATTERN_TX;
    case BOARD_STATE_RX:
        return STATUS_LED_PATTERN_RX;
    case BOARD_STATE_FORWARDING:
        return STATUS_LED_PATTERN_FORWARDING;
    case BOARD_STATE_ERROR:
        return STATUS_LED_PATTERN_ERROR;
    case BOARD_STATE_TEST_RUNNING:
        return STATUS_LED_PATTERN_TEST;
    default:
        return STATUS_LED_PATTERN_OFF;
    }
}

esp_err_t board_feedback_init(void)
{
    ESP_RETURN_ON_ERROR(status_led_init(), TAG, "status LED init");
    esp_err_t oled_err = oled_display_init();
    if (oled_err != ESP_OK) {
        ESP_LOGW(TAG, "OLED init failed: %s", esp_err_to_name(oled_err));
    }
    ESP_RETURN_ON_ERROR(board_feedback_set_state(BOARD_STATE_BOOT), TAG, "set boot state");
    return ESP_OK;
}

esp_err_t board_feedback_set_state(board_state_t state)
{
    current_state = state;
    ESP_RETURN_ON_ERROR(status_led_set_pattern(led_pattern_for_state(state)), TAG, "set LED pattern");

    /* Once runtime metrics are available, TX/RX state changes only affect the
     * LED. Otherwise the generic state view would erase the beacon data every
     * time the radio returns to receive mode. Errors remain visible on OLED. */
    if (status_screen_active && state != BOARD_STATE_ERROR) {
        return ESP_OK;
    }

    return oled_display_show_lines("ESP32 + SX1262", "IDS BEACON",
                                   state_to_string(state), "INITIALIZING");
}

esp_err_t board_feedback_show_status(const board_status_t *status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    status_screen_active = true;

    char line1[24];
    char line2[24];
    char line3[24];
    char line4[24];

    snprintf(line1, sizeof(line1), "ESP32 + SX1262");
    snprintf(line2, sizeof(line2), "IDS BEACON N:%u", status->node_addr);
    snprintf(line3, sizeof(line3), "RX:%03lu TX:%03lu",
             (unsigned long)status->last_rx_number,
             (unsigned long)status->last_tx_number);
    snprintf(line4, sizeof(line4), "DT:%lums P:%ddBm",
             (unsigned long)status->packet_interval_ms,
             status->last_rssi_dbm);
    return oled_display_show_lines(line1, line2, line3, line4);
}

esp_err_t board_feedback_show_message(const char *line1, const char *line2, const char *line3)
{
    return oled_display_show_lines(line1, line2, line3, state_to_string(current_state));
}

esp_err_t board_feedback_tick(void)
{
    return status_led_tick();
}
