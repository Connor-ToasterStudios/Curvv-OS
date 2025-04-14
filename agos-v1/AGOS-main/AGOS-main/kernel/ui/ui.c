
#include "ui.h"
#include "../graphics/graphics.h"
#include "../font.h"

void ui_draw_button(int x, int y, int width, int height, const char* text, uint32_t color) {
    graphics_draw_rect(x, y, width, height, color);
    graphics_draw_rect(x+1, y+1, width-2, height-2, color & 0xEEEEEE);
    int text_x = x + (width - strlen(text) * 8) / 2;
    int text_y = y + (height - 16) / 2;
    draw_text(text, text_x, text_y, 0x000000);
}

void ui_draw_textbox(int x, int y, int width, int height, const char* text) {
    graphics_draw_rect(x, y, width, height, 0xFFFFFF);
    graphics_draw_rect(x+1, y+1, width-2, height-2, 0xEEEEEE);
    draw_text(text, x + 4, y + (height - 16) / 2, 0x000000);
}
