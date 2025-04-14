#ifndef WINDOW_H
#define WINDOW_H

typedef struct {
    int x, y;
    int width, height;
    const char* title;
    void (*draw)(void*);
    int is_dragging;
    int drag_start_x, drag_start_y;
} Window;

void window_init(Window* win, const char* title, int x, int y, int w, int h, void (*draw)(void*));
void window_draw_frame(Window* win);
void window_handle(Window* win);

#endif