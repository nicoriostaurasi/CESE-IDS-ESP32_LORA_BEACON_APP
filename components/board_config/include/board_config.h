#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

#define BOARD_CONFIG_PLATFORM_NAME "Heltec WiFi LoRa 32 V3"
#define BOARD_CONFIG_LORA_VARIANT "433 MHz"
#define BOARD_CONFIG_MCU_NAME "ESP32-S3"
#define BOARD_CONFIG_RADIO_NAME "SX1262"
#define BOARD_CONFIG_USB_UART_NAME "CP2102"
#define BOARD_CONFIG_OLED_NAME "SSD1306-compatible 128x64"

#define BOARD_CONFIG_PIN_TODO (-1)

#define BOARD_CONFIG_GPIO_OLED_SDA 17
#define BOARD_CONFIG_GPIO_OLED_SCL 18
#define BOARD_CONFIG_GPIO_OLED_RESET 21

#define BOARD_CONFIG_GPIO_SX1262_MOSI 10
#define BOARD_CONFIG_GPIO_SX1262_MISO 11
#define BOARD_CONFIG_GPIO_SX1262_SCLK 9
#define BOARD_CONFIG_GPIO_SX1262_NSS 8
#define BOARD_CONFIG_GPIO_SX1262_RESET 12
#define BOARD_CONFIG_GPIO_SX1262_BUSY 13
#define BOARD_CONFIG_GPIO_SX1262_DIO1 14

#define BOARD_CONFIG_GPIO_STATUS_LED 35
#define BOARD_CONFIG_GPIO_VEXT 36
#define BOARD_CONFIG_GPIO_USB_UART_TX 43
#define BOARD_CONFIG_GPIO_USB_UART_RX 44

typedef struct {
    int sda;
    int scl;
    int reset;
} board_i2c_oled_pins_t;

typedef struct {
    int mosi;
    int miso;
    int sclk;
    int nss;
    int reset;
    int busy;
    int dio1;
} board_sx1262_pins_t;

typedef struct {
    int status_led;
    int vext;
    int usb_uart_tx;
    int usb_uart_rx;
} board_gpio_pins_t;

typedef struct {
    const char *platform;
    const char *lora_variant;
    const char *mcu;
    const char *radio;
    const char *usb_uart;
    const char *oled;
    board_i2c_oled_pins_t oled_pins;
    board_sx1262_pins_t radio_pins;
    board_gpio_pins_t gpio_pins;
} board_config_t;

const board_config_t *board_config_get(void);
const char *board_config_get_firmware_name(void);

#ifdef _cplusplus
}
#endif

#endif
