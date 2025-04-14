#include "graphics.h"
#include "mouse.h"
#include <string.h>
#include "ui_toolkit.h"
#include "../font.h"
#include <stdlib.h>

Button* create_button(int x, int y, int width, int height, const char* text) {
    Button* btn = malloc(sizeof(Button));
    btn->x = x;
    btn->y = y;
    btn->width = width;
    btn->height = height;
    btn->text = text;
    btn->color = 0x444444;
    btn->hover_color = 0x666666;
    btn->is_pressed = false;
    return btn;
}

void draw_button(Button* button) {
    uint32_t color = button->color;
    if (is_mouse_over(button->x, button->y, button->width, button->height)) {
        color = button->hover_color;
    }
    graphics_draw_rect(button->x, button->y, button->width, button->height, color);
    int text_x = button->x + (button->width - strlen(button->text) * 8) / 2;
    int text_y = button->y + (button->height - 16) / 2;
    draw_text(button->text, text_x, text_y, 0xFFFFFF);
}

bool button_clicked(Button* button) {
    return is_mouse_over(button->x, button->y, button->width, button->height) && mouse_clicked();
}

void ui_draw_textbox(int x, int y, int w, const char* text) {
    graphics_draw_rect(x, y, w, 20, 0x222222);
    graphics_draw_rect(x, y, w, 20, 0x444444);
    graphics_draw_text(text, x + 4, y + 4, 0xCCCCCC);
}