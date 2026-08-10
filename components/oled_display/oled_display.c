#include "oled_display.h"

#include "board_config.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>

#define OLED_CMD_PREFIX 0x00
#define OLED_DATA_PREFIX 0x40

static const char *TAG = "OLED_DISPLAY";
static i2c_master_bus_handle_t i2c_bus;
static i2c_master_dev_handle_t oled_dev;
static display_graphics_t canvas;
static bool initialized;

static esp_err_t oled_write(uint8_t prefix, const uint8_t *data, size_t len)
{
    if (oled_dev == NULL || data == NULL || len == 0) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t tx[17];
    if (len > sizeof(tx) - 1) {
        return ESP_ERR_INVALID_SIZE;
    }
    tx[0] = prefix;
    memcpy(&tx[1], data, len);
    return i2c_master_transmit(oled_dev, tx, len + 1, 100);
}

static esp_err_t oled_cmd(uint8_t cmd)
{
    return oled_write(OLED_CMD_PREFIX, &cmd, 1);
}

static esp_err_t oled_cmds(const uint8_t *cmds, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        esp_err_t err = oled_cmd(cmds[i]);
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}

static esp_err_t configure_vext(void)
{
    const board_config_t *board = board_config_get();
    if (board->gpio_pins.vext == BOARD_CONFIG_PIN_TODO) {
        return ESP_OK;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << board->gpio_pins.vext,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "configure VEXT");
    ESP_RETURN_ON_ERROR(gpio_set_level(board->gpio_pins.vext, 0), TAG, "enable VEXT");
    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

esp_err_t oled_display_init(void)
{
    const board_config_t *board = board_config_get();
    ESP_RETURN_ON_FALSE(board->oled_pins.sda != BOARD_CONFIG_PIN_TODO, ESP_ERR_INVALID_STATE, TAG, "OLED SDA missing");
    ESP_RETURN_ON_FALSE(board->oled_pins.scl != BOARD_CONFIG_PIN_TODO, ESP_ERR_INVALID_STATE, TAG, "OLED SCL missing");

    ESP_RETURN_ON_ERROR(configure_vext(), TAG, "VEXT setup");

    if (board->oled_pins.reset != BOARD_CONFIG_PIN_TODO) {
        gpio_config_t reset_cfg = {
            .pin_bit_mask = 1ULL << board->oled_pins.reset,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_RETURN_ON_ERROR(gpio_config(&reset_cfg), TAG, "configure OLED reset");
        gpio_set_level(board->oled_pins.reset, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(board->oled_pins.reset, 1);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = board->oled_pins.scl,
        .sda_io_num = board->oled_pins.sda,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &i2c_bus), TAG, "create I2C bus");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_DISPLAY_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &oled_dev), TAG, "add OLED device");

    const uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,
        0x2E, 0xAF,
    };
    ESP_RETURN_ON_ERROR(oled_cmds(init_cmds, sizeof(init_cmds)), TAG, "OLED init");

    display_graphics_init(&canvas);
    initialized = true;
    ESP_RETURN_ON_ERROR(oled_display_clear(), TAG, "clear OLED");
    ESP_LOGI(TAG, "OLED initialized at 0x%02X", OLED_DISPLAY_I2C_ADDR);
    return ESP_OK;
}

esp_err_t oled_display_clear(void)
{
    display_graphics_fill(&canvas, false);
    return oled_display_update();
}

esp_err_t oled_display_update(void)
{
    ESP_RETURN_ON_FALSE(initialized, ESP_ERR_INVALID_STATE, TAG, "OLED not initialized");
    for (uint8_t page = 0; page < 8; ++page) {
        ESP_RETURN_ON_ERROR(oled_cmd((uint8_t)(0xB0 + page)), TAG, "set page");
        ESP_RETURN_ON_ERROR(oled_cmd(0x00), TAG, "set low column");
        ESP_RETURN_ON_ERROR(oled_cmd(0x10), TAG, "set high column");
        const uint8_t *src = &canvas.buffer[page * DISPLAY_GRAPHICS_WIDTH];
        for (uint8_t col = 0; col < DISPLAY_GRAPHICS_WIDTH; col += 16) {
            ESP_RETURN_ON_ERROR(oled_write(OLED_DATA_PREFIX, &src[col], 16), TAG, "write data");
        }
    }
    return ESP_OK;
}

esp_err_t oled_display_show_lines(const char *line1, const char *line2, const char *line3, const char *line4)
{
    display_graphics_fill(&canvas, false);
    display_graphics_draw_string(&canvas, 0, 0, line1 ? line1 : "");
    display_graphics_draw_string(&canvas, 0, 16, line2 ? line2 : "");
    display_graphics_draw_string(&canvas, 0, 32, line3 ? line3 : "");
    display_graphics_draw_string(&canvas, 0, 48, line4 ? line4 : "");
    return oled_display_update();
}

display_graphics_t *oled_display_get_canvas(void)
{
    return &canvas;
}

bool oled_display_is_alive(void)
{
    return initialized && oled_dev != NULL;
}
