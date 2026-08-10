#ifndef DISPLAY_GRAPHICS_H
#define DISPLAY_GRAPHICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

#define DISPLAY_GRAPHICS_WIDTH 128
#define DISPLAY_GRAPHICS_HEIGHT 64
#define DISPLAY_GRAPHICS_BUFFER_SIZE (DISPLAY_GRAPHICS_WIDTH * DISPLAY_GRAPHICS_HEIGHT / 8)

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t buffer[DISPLAY_GRAPHICS_BUFFER_SIZE];
} display_graphics_t;

void display_graphics_init(display_graphics_t *display);
void display_graphics_fill(display_graphics_t *display, bool on);
void display_graphics_draw_pixel(display_graphics_t *display, int x, int y, bool on);
void display_graphics_draw_hline(display_graphics_t *display, int x, int y, int w, bool on);
void display_graphics_draw_vline(display_graphics_t *display, int x, int y, int h, bool on);
void display_graphics_draw_rect(display_graphics_t *display, int x, int y, int w, int h, bool on);
void display_graphics_goto_xy(display_graphics_t *display, uint8_t x, uint8_t y);
void display_graphics_puts(display_graphics_t *display, const char *text);
void display_graphics_draw_string(display_graphics_t *display, int x, int y, const char *text);

#ifdef _cplusplus
}
#endif

#endif
