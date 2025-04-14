#ifndef UI_TOOLKIT_H
#define UI_TOOLKIT_H

#include "../graphics/graphics.h"
#include "../events.h"

typedef struct {
    int x, y, width, height;
    const char* text;
    uint32_t color;
    uint32_t hover_color;
    bool is_pressed;
} Button;

typedef struct {
    int x, y, width, height;
    char* text;
    int cursor_pos;
    bool focused;
} TextField;

Button* create_button(int x, int y, int width, int height, const char* text);
void draw_button(Button* button);
bool button_clicked(Button* button);

TextField* create_textfield(int x, int y, int width, int height);
void draw_textfield(TextField* field);
void handle_textfield_input(TextField* field);

#endif