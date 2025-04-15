#ifndef UI_TOOLKIT_H
#define UI_TOOLKIT_H

typedef struct {
    int width;
    int height;
    void* theme_data;
    void* widget_list;
} UIToolkit;

int ui_init(UIToolkit* ui, int width, int height);
void ui_update(UIToolkit* ui);
void ui_cleanup(UIToolkit* ui);
void draw_char(void* window, int x, int y, char c, int color);
void draw_text(void* window, int x, int y, const char* text, int color);
void draw_button(void* window, int x, int y, int width, int height, const char* text);
void draw_rect(void* window, int x, int y, int width, int height, int color);
void fill_rect(void* window, int x, int y, int width, int height, int color);

#endif /* UI_TOOLKIT_H */
