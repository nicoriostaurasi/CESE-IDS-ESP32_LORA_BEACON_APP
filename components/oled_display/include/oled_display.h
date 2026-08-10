#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "display_graphics.h"
#include "esp_err.h"

#ifdef _cplusplus
extern "C" {
#endif

#define OLED_DISPLAY_I2C_ADDR 0x3C

esp_err_t oled_display_init(void);
esp_err_t oled_display_clear(void);
esp_err_t oled_display_update(void);
esp_err_t oled_display_show_lines(const char *line1, const char *line2, const char *line3, const char *line4);
display_graphics_t *oled_display_get_canvas(void);
bool oled_display_is_alive(void);

#ifdef _cplusplus
}
#endif

#endif
