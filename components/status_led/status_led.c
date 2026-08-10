#include "status_led.h"

#include "board_config.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "STATUS_LED";
static int led_gpio = BOARD_CONFIG_PIN_TODO;
static bool led_level;
static status_led_pattern_t current_pattern = STATUS_LED_PATTERN_OFF;
static int64_t last_toggle_us;

static uint32_t interval_ms_for_pattern(status_led_pattern_t pattern)
{
    switch (pattern) {
    case STATUS_LED_PATTERN_BOOT:
        return 120;
    case STATUS_LED_PATTERN_IDLE:
        return 900;
    case STATUS_LED_PATTERN_TX:
        return 80;
    case STATUS_LED_PATTERN_RX:
        return 160;
    case STATUS_LED_PATTERN_FORWARDING:
        return 300;
    case STATUS_LED_PATTERN_ERROR:
        return 70;
    case STATUS_LED_PATTERN_TEST:
        return 220;
    default:
        return 0;
    }
}

esp_err_t status_led_init(void)
{
    const board_config_t *board = board_config_get();
    led_gpio = board->gpio_pins.status_led;
    if (led_gpio == BOARD_CONFIG_PIN_TODO) {
        ESP_LOGW(TAG, "status LED GPIO not available");
        return ESP_OK;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << led_gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "configure LED");
    last_toggle_us = esp_timer_get_time();
    return status_led_off();
}

esp_err_t status_led_on(void)
{
    if (led_gpio == BOARD_CONFIG_PIN_TODO) {
        return ESP_OK;
    }
    led_level = true;
    return gpio_set_level(led_gpio, 1);
}

esp_err_t status_led_off(void)
{
    if (led_gpio == BOARD_CONFIG_PIN_TODO) {
        return ESP_OK;
    }
    led_level = false;
    return gpio_set_level(led_gpio, 0);
}

esp_err_t status_led_toggle(void)
{
    return led_level ? status_led_off() : status_led_on();
}

esp_err_t status_led_set_pattern(status_led_pattern_t pattern)
{
    current_pattern = pattern;
    last_toggle_us = esp_timer_get_time();
    if (pattern == STATUS_LED_PATTERN_ON) {
        return status_led_on();
    }
    if (pattern == STATUS_LED_PATTERN_OFF) {
        return status_led_off();
    }
    return status_led_on();
}

esp_err_t status_led_tick(void)
{
    uint32_t interval_ms = interval_ms_for_pattern(current_pattern);
    if (interval_ms == 0) {
        return ESP_OK;
    }

    int64_t now = esp_timer_get_time();
    if ((now - last_toggle_us) >= (int64_t)interval_ms * 1000) {
        last_toggle_us = now;
        return status_led_toggle();
    }
    return ESP_OK;
}
