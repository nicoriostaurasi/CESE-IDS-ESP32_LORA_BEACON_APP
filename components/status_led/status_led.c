#include "status_led.h"

#include "board_config.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <stdbool.h>

#define STATUS_LED_RX_DURATION_MS 1000U
#define STATUS_LED_ACTIVE_LEVEL 1
#define STATUS_LED_INACTIVE_LEVEL 0

static const char *TAG = "STATUS_LED";
static int led_gpio = BOARD_CONFIG_PIN_TODO;
static bool led_is_on;
static int64_t rx_pulse_started_us;
static int64_t rx_pulse_deadline_us;

esp_err_t status_led_init(void)
{
    const board_config_t *board = board_config_get();
    led_gpio = board->gpio_pins.status_led;
    if (led_gpio == BOARD_CONFIG_PIN_TODO) {
        ESP_LOGW(TAG, "status LED GPIO not available");
        return ESP_OK;
    }

    gpio_config_t config = {
        .pin_bit_mask = 1ULL << led_gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&config), TAG, "configure status LED");
    return status_led_off();
}

esp_err_t status_led_on(void)
{
    if (led_gpio == BOARD_CONFIG_PIN_TODO) {
        return ESP_OK;
    }
    led_is_on = true;
    return gpio_set_level(led_gpio, STATUS_LED_ACTIVE_LEVEL);
}

esp_err_t status_led_off(void)
{
    rx_pulse_started_us = 0;
    rx_pulse_deadline_us = 0;
    led_is_on = false;
    if (led_gpio == BOARD_CONFIG_PIN_TODO) {
        return ESP_OK;
    }
    return gpio_set_level(led_gpio, STATUS_LED_INACTIVE_LEVEL);
}

esp_err_t status_led_set_pattern(status_led_pattern_t pattern)
{
    if (pattern != STATUS_LED_PATTERN_RX) {
        return status_led_off();
    }

    rx_pulse_started_us = esp_timer_get_time();
    rx_pulse_deadline_us = rx_pulse_started_us +
                           ((int64_t)STATUS_LED_RX_DURATION_MS * 1000LL);
    ESP_LOGI(TAG, "RX LED on: duration=%u ms", STATUS_LED_RX_DURATION_MS);
    return status_led_on();
}

esp_err_t status_led_tick(void)
{
    if (led_is_on && rx_pulse_deadline_us != 0 &&
        esp_timer_get_time() >= rx_pulse_deadline_us) {
        int64_t elapsed_ms = (esp_timer_get_time() - rx_pulse_started_us) / 1000LL;
        ESP_LOGI(TAG, "RX LED off: elapsed=%lld ms", (long long)elapsed_ms);
        return status_led_off();
    }
    return ESP_OK;
}
