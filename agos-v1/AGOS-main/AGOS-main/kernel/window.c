#include "window.h"
#include "graphics/graphics.h"
#include "mouse.h"
#include "events.h"
#include "themes.h"

void window_init(Window* win, const char* title, int x, int y, int w, int h, void (*draw)(void*)) {
    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    win->title = title;
    win->draw = draw;
    win->is_dragging = 0;
}

void window_draw_frame(Window* win) {
    Theme theme = get_active_theme();
    graphics_draw_rect(win->x, win->y, win->width, 20, theme.title_bg);
    graphics_draw_text(win->title, win->x + 5, win->y + 5, theme.title_fg);
    graphics_draw_rect(win->x, win->y + 20, win->width, win->height - 20, theme.body_bg);
    graphics_draw_rect(win->x, win->y, win->width, win->height, theme.border);
}

void window_handle(Window* win) {
    if (mouse_get_left()) {
        if (is_mouse_over(win->x, win->y, win->width, 20)) {
            if (!win->is_dragging) {
                win->is_dragging = 1;
                win->drag_start_x = mouse_get_x() - win->x;
                win->drag_start_y = mouse_get_y() - win->y;
            }
        }
    } else {
        win->is_dragging = 0;
    }

    if (win->is_dragging) {
        win->x = mouse_get_x() - win->drag_start_x;
        win->y = mouse_get_y() - win->drag_start_y;
    }
}