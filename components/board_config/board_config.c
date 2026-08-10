#include "board_config.h"

#define FW_NAME "IDS LoRa Beacon"

static const board_config_t board_config = {
    .platform = BOARD_CONFIG_PLATFORM_NAME,
    .lora_variant = BOARD_CONFIG_LORA_VARIANT,
    .mcu = BOARD_CONFIG_MCU_NAME,
    .radio = BOARD_CONFIG_RADIO_NAME,
    .usb_uart = BOARD_CONFIG_USB_UART_NAME,
    .oled = BOARD_CONFIG_OLED_NAME,
    .oled_pins = {
        .sda = BOARD_CONFIG_GPIO_OLED_SDA,
        .scl = BOARD_CONFIG_GPIO_OLED_SCL,
        .reset = BOARD_CONFIG_GPIO_OLED_RESET,
    },
    .radio_pins = {
        .mosi = BOARD_CONFIG_GPIO_SX1262_MOSI,
        .miso = BOARD_CONFIG_GPIO_SX1262_MISO,
        .sclk = BOARD_CONFIG_GPIO_SX1262_SCLK,
        .nss = BOARD_CONFIG_GPIO_SX1262_NSS,
        .reset = BOARD_CONFIG_GPIO_SX1262_RESET,
        .busy = BOARD_CONFIG_GPIO_SX1262_BUSY,
        .dio1 = BOARD_CONFIG_GPIO_SX1262_DIO1,
    },
    .gpio_pins = {
        .status_led = BOARD_CONFIG_GPIO_STATUS_LED,
        .vext = BOARD_CONFIG_GPIO_VEXT,
        .usb_uart_tx = BOARD_CONFIG_GPIO_USB_UART_TX,
        .usb_uart_rx = BOARD_CONFIG_GPIO_USB_UART_RX,
    },
};

const board_config_t *board_config_get(void)
{
    return &board_config;
}

const char *board_config_get_firmware_name(void)
{
    return FW_NAME;
}
