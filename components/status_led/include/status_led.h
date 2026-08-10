#ifndef STATUS_LED_H
#define STATUS_LED_H

#include "esp_err.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef enum {
    STATUS_LED_PATTERN_OFF = 0,
    STATUS_LED_PATTERN_ON,
    STATUS_LED_PATTERN_BOOT,
    STATUS_LED_PATTERN_IDLE,
    STATUS_LED_PATTERN_TX,
    STATUS_LED_PATTERN_RX,
    STATUS_LED_PATTERN_FORWARDING,
    STATUS_LED_PATTERN_ERROR,
    STATUS_LED_PATTERN_TEST,
} status_led_pattern_t;

esp_err_t status_led_init(void);
esp_err_t status_led_on(void);
esp_err_t status_led_off(void);
esp_err_t status_led_toggle(void);
esp_err_t status_led_set_pattern(status_led_pattern_t pattern);
esp_err_t status_led_tick(void);

#ifdef _cplusplus
}
#endif

#endif
