#include "display_graphics.h"

#include <ctype.h>
#include <string.h>

static void glyph_for_char(char ch, uint8_t glyph[5])
{
    static const uint8_t blank[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t *src = blank;

    switch ((char)toupper((unsigned char)ch)) {
    case '0': { static const uint8_t g[5] = {0x3E, 0x51, 0x49, 0x45, 0x3E}; src = g; break; }
    case '1': { static const uint8_t g[5] = {0x00, 0x42, 0x7F, 0x40, 0x00}; src = g; break; }
    case '2': { static const uint8_t g[5] = {0x42, 0x61, 0x51, 0x49, 0x46}; src = g; break; }
    case '3': { static const uint8_t g[5] = {0x21, 0x41, 0x45, 0x4B, 0x31}; src = g; break; }
    case '4': { static const uint8_t g[5] = {0x18, 0x14, 0x12, 0x7F, 0x10}; src = g; break; }
    case '5': { static const uint8_t g[5] = {0x27, 0x45, 0x45, 0x45, 0x39}; src = g; break; }
    case '6': { static const uint8_t g[5] = {0x3C, 0x4A, 0x49, 0x49, 0x30}; src = g; break; }
    case '7': { static const uint8_t g[5] = {0x01, 0x71, 0x09, 0x05, 0x03}; src = g; break; }
    case '8': { static const uint8_t g[5] = {0x36, 0x49, 0x49, 0x49, 0x36}; src = g; break; }
    case '9': { static const uint8_t g[5] = {0x06, 0x49, 0x49, 0x29, 0x1E}; src = g; break; }
    case 'A': { static const uint8_t g[5] = {0x7E, 0x11, 0x11, 0x11, 0x7E}; src = g; break; }
    case 'B': { static const uint8_t g[5] = {0x7F, 0x49, 0x49, 0x49, 0x36}; src = g; break; }
    case 'C': { static const uint8_t g[5] = {0x3E, 0x41, 0x41, 0x41, 0x22}; src = g; break; }
    case 'D': { static const uint8_t g[5] = {0x7F, 0x41, 0x41, 0x22, 0x1C}; src = g; break; }
    case 'E': { static const uint8_t g[5] = {0x7F, 0x49, 0x49, 0x49, 0x41}; src = g; break; }
    case 'F': { static const uint8_t g[5] = {0x7F, 0x09, 0x09, 0x09, 0x01}; src = g; break; }
    case 'G': { static const uint8_t g[5] = {0x3E, 0x41, 0x49, 0x49, 0x7A}; src = g; break; }
    case 'H': { static const uint8_t g[5] = {0x7F, 0x08, 0x08, 0x08, 0x7F}; src = g; break; }
    case 'I': { static const uint8_t g[5] = {0x00, 0x41, 0x7F, 0x41, 0x00}; src = g; break; }
    case 'J': { static const uint8_t g[5] = {0x20, 0x40, 0x41, 0x3F, 0x01}; src = g; break; }
    case 'K': { static const uint8_t g[5] = {0x7F, 0x08, 0x14, 0x22, 0x41}; src = g; break; }
    case 'L': { static const uint8_t g[5] = {0x7F, 0x40, 0x40, 0x40, 0x40}; src = g; break; }
    case 'M': { static const uint8_t g[5] = {0x7F, 0x02, 0x0C, 0x02, 0x7F}; src = g; break; }
    case 'N': { static const uint8_t g[5] = {0x7F, 0x04, 0x08, 0x10, 0x7F}; src = g; break; }
    case 'O': { static const uint8_t g[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E}; src = g; break; }
    case 'P': { static const uint8_t g[5] = {0x7F, 0x09, 0x09, 0x09, 0x06}; src = g; break; }
    case 'Q': { static const uint8_t g[5] = {0x3E, 0x41, 0x51, 0x21, 0x5E}; src = g; break; }
    case 'R': { static const uint8_t g[5] = {0x7F, 0x09, 0x19, 0x29, 0x46}; src = g; break; }
    case 'S': { static const uint8_t g[5] = {0x46, 0x49, 0x49, 0x49, 0x31}; src = g; break; }
    case 'T': { static const uint8_t g[5] = {0x01, 0x01, 0x7F, 0x01, 0x01}; src = g; break; }
    case 'U': { static const uint8_t g[5] = {0x3F, 0x40, 0x40, 0x40, 0x3F}; src = g; break; }
    case 'V': { static const uint8_t g[5] = {0x1F, 0x20, 0x40, 0x20, 0x1F}; src = g; break; }
    case 'W': { static const uint8_t g[5] = {0x7F, 0x20, 0x18, 0x20, 0x7F}; src = g; break; }
    case 'X': { static const uint8_t g[5] = {0x63, 0x14, 0x08, 0x14, 0x63}; src = g; break; }
    case 'Y': { static const uint8_t g[5] = {0x07, 0x08, 0x70, 0x08, 0x07}; src = g; break; }
    case 'Z': { static const uint8_t g[5] = {0x61, 0x51, 0x49, 0x45, 0x43}; src = g; break; }
    case ':': { static const uint8_t g[5] = {0x00, 0x36, 0x36, 0x00, 0x00}; src = g; break; }
    case '-': { static const uint8_t g[5] = {0x08, 0x08, 0x08, 0x08, 0x08}; src = g; break; }
    case '>': { static const uint8_t g[5] = {0x41, 0x22, 0x14, 0x08, 0x00}; src = g; break; }
    case '/': { static const uint8_t g[5] = {0x20, 0x10, 0x08, 0x04, 0x02}; src = g; break; }
    case '.': { static const uint8_t g[5] = {0x00, 0x60, 0x60, 0x00, 0x00}; src = g; break; }
    case '%': { static const uint8_t g[5] = {0x63, 0x13, 0x08, 0x64, 0x63}; src = g; break; }
    default:
        break;
    }

    memcpy(glyph, src, 5);
}

void display_graphics_init(display_graphics_t *display)
{
    if (display == NULL) {
        return;
    }
    display->width = DISPLAY_GRAPHICS_WIDTH;
    display->height = DISPLAY_GRAPHICS_HEIGHT;
    display->cursor_x = 0;
    display->cursor_y = 0;
    display_graphics_fill(display, false);
}

void display_graphics_fill(display_graphics_t *display, bool on)
{
    if (display == NULL) {
        return;
    }
    memset(display->buffer, on ? 0xFF : 0x00, sizeof(display->buffer));
}

void display_graphics_draw_pixel(display_graphics_t *display, int x, int y, bool on)
{
    if (display == NULL || x < 0 || y < 0 || x >= display->width || y >= display->height) {
        return;
    }
    size_t index = (size_t)x + (size_t)(y / 8) * display->width;
    uint8_t mask = (uint8_t)(1U << (y & 0x07));
    if (on) {
        display->buffer[index] |= mask;
    } else {
        display->buffer[index] &= (uint8_t)~mask;
    }
}

void display_graphics_draw_hline(display_graphics_t *display, int x, int y, int w, bool on)
{
    for (int i = 0; i < w; ++i) {
        display_graphics_draw_pixel(display, x + i, y, on);
    }
}

void display_graphics_draw_vline(display_graphics_t *display, int x, int y, int h, bool on)
{
    for (int i = 0; i < h; ++i) {
        display_graphics_draw_pixel(display, x, y + i, on);
    }
}

void display_graphics_draw_rect(display_graphics_t *display, int x, int y, int w, int h, bool on)
{
    display_graphics_draw_hline(display, x, y, w, on);
    display_graphics_draw_hline(display, x, y + h - 1, w, on);
    display_graphics_draw_vline(display, x, y, h, on);
    display_graphics_draw_vline(display, x + w - 1, y, h, on);
}

void display_graphics_goto_xy(display_graphics_t *display, uint8_t x, uint8_t y)
{
    if (display == NULL) {
        return;
    }
    display->cursor_x = x;
    display->cursor_y = y;
}

static void draw_char(display_graphics_t *display, int x, int y, char ch)
{
    uint8_t glyph[5];
    glyph_for_char(ch, glyph);
    for (int col = 0; col < 5; ++col) {
        for (int row = 0; row < 7; ++row) {
            display_graphics_draw_pixel(display, x + col, y + row, (glyph[col] & (1U << row)) != 0);
        }
    }
}

void display_graphics_puts(display_graphics_t *display, const char *text)
{
    if (display == NULL || text == NULL) {
        return;
    }
    while (*text != '\0') {
        draw_char(display, display->cursor_x, display->cursor_y, *text);
        display->cursor_x = (uint8_t)(display->cursor_x + 6);
        if (display->cursor_x > display->width - 6) {
            display->cursor_x = 0;
            display->cursor_y = (uint8_t)(display->cursor_y + 8);
        }
        ++text;
    }
}

void display_graphics_draw_string(display_graphics_t *display, int x, int y, const char *text)
{
    display_graphics_goto_xy(display, (uint8_t)x, (uint8_t)y);
    display_graphics_puts(display, text);
}
