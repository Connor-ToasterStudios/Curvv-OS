
#include "font.h"
#include "graphics/graphics.h"

void draw_char(char c, int x, int y, uint32_t color) {
    for (int row = 0; row < 16; row++) {
        uint8_t line = font_data[(unsigned char)c][row];
        for (int col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                graphics_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_text(const char* text, int x, int y, uint32_t color) {
    int pos_x = x;
    while (*text) {
        draw_char(*text, pos_x, y, color);
        pos_x += 8;
        text++;
    }
}
