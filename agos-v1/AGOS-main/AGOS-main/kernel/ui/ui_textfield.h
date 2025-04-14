#pragma once

typedef struct {
    int x, y, width;
    char text[128];
    int focused;
} TextField;

void textfield_draw(TextField* tf);
void textfield_handle_input(TextField* tf);
void textfield_focus(TextField* tf, int mouse_x, int mouse_y);
