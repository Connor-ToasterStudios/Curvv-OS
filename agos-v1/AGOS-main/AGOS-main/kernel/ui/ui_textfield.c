#include "ui_textfield.h"
#include "graphics.h"
#include "keyboard.h"
#include "mouse.h"
#include <string.h>

void textfield_focus(TextField* tf, int mx, int my) {
    tf->focused = (mx >= tf->x && mx <= tf->x + tf->width && my >= tf->y && my <= tf->y + 20 && mouse_clicked());
}

void textfield_handle_input(TextField* tf) {
    if (!tf->focused) return;
    char ch = keyboard_get_char();
    if (ch) {
        int len = strlen(tf->text);
        if (ch == '\b' && len > 0) {
            tf->text[len - 1] = 0;
        } else if (len < sizeof(tf->text) - 1 && ch >= 32) {
            tf->text[len] = ch;
            tf->text[len + 1] = 0;
        }
    }
}

void textfield_draw(TextField* tf) {
    graphics_draw_rect(tf->x, tf->y, tf->width, 20, tf->focused ? 0x6666FF : 0x444444);
    graphics_draw_text(tf->text, tf->x + 4, tf->y + 4, 0xFFFFFF);
}
