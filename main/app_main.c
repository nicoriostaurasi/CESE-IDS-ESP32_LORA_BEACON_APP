#include "app_runtime.h"
#include "board_config.h"
#include "esp_chip_info.h"
#include "esp_app_desc.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "IDS_BEACON";
static const char *BOARD_TAG = "BOARD_CONFIG";

static void log_pin_placeholder(const char *name, int pin)
{
    if (pin == BOARD_CONFIG_PIN_TODO) {
        ESP_LOGI(BOARD_TAG, "%s: TODO_VERIFICAR", name);
    } else {
        ESP_LOGI(BOARD_TAG, "%s: GPIO%d", name, pin);
    }
}

void app_main(void)
{
    const board_config_t *board = board_config_get();
    const esp_app_desc_t *app_desc = esp_app_get_description();
    esp_chip_info_t chip_info;
    uint32_t flash_size = 0;

    esp_chip_info(&chip_info);
    esp_err_t flash_ret = esp_flash_get_size(NULL, &flash_size);

    ESP_LOGI(TAG, "Firmware: %s", board_config_get_firmware_name());
    ESP_LOGI(TAG, "Project: %s", app_desc->project_name);
    ESP_LOGI(TAG, "Version: %s", app_desc->version);
    ESP_LOGI(TAG, "ESP-IDF: %s", app_desc->idf_ver);
    ESP_LOGI(TAG, "Application: IDS LoRa beacon test");

    ESP_LOGI(BOARD_TAG, "Platform: %s %s", board->platform, board->lora_variant);
    ESP_LOGI(BOARD_TAG, "MCU: %s", board->mcu);
    ESP_LOGI(BOARD_TAG, "Radio: %s", board->radio);
    ESP_LOGI(BOARD_TAG, "USB-UART: %s", board->usb_uart);
    ESP_LOGI(BOARD_TAG, "OLED: %s", board->oled);

    ESP_LOGI(TAG, "CPU cores: %d", chip_info.cores);

    if (flash_ret == ESP_OK) {
        ESP_LOGI(TAG, "Flash size: %lu MB", (unsigned long)(flash_size / (1024UL * 1024UL)));
    } else {
        ESP_LOGW(TAG, "Flash size not available: %s", esp_err_to_name(flash_ret));
    }

    log_pin_placeholder("OLED SDA", board->oled_pins.sda);
    log_pin_placeholder("OLED SCL", board->oled_pins.scl);
    log_pin_placeholder("OLED RESET", board->oled_pins.reset);
    log_pin_placeholder("SX1262 MOSI", board->radio_pins.mosi);
    log_pin_placeholder("SX1262 MISO", board->radio_pins.miso);
    log_pin_placeholder("SX1262 SCLK", board->radio_pins.sclk);
    log_pin_placeholder("SX1262 NSS", board->radio_pins.nss);
    log_pin_placeholder("SX1262 RESET", board->radio_pins.reset);
    log_pin_placeholder("SX1262 BUSY", board->radio_pins.busy);
    log_pin_placeholder("SX1262 DIO1", board->radio_pins.dio1);
    log_pin_placeholder("STATUS LED", board->gpio_pins.status_led);
    log_pin_placeholder("VEXT CONTROL", board->gpio_pins.vext);
    log_pin_placeholder("USB UART TX", board->gpio_pins.usb_uart_tx);
    log_pin_placeholder("USB UART RX", board->gpio_pins.usb_uart_rx);

    esp_err_t runtime_ret = app_runtime_start();
    if (runtime_ret != ESP_OK) {
        ESP_LOGE(TAG, "FreeRTOS runtime start failed: %s", esp_err_to_name(runtime_ret));
        return;
    }

    ESP_LOGI(TAG, "app_main completed initialization; FreeRTOS tasks are running");
    vTaskDelete(NULL);
}
