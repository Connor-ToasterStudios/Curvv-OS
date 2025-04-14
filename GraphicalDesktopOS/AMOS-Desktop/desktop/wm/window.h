#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

typedef struct {
    int x;
    int y;
    int width;
    int height;
    const char* title;
    void* content;
    void* tabs;
    int is_active;
    int is_visible;
} Window;

typedef struct {
    int screen_width;
    int screen_height;
    void* window_list;
    void* active_window;
    void* taskbar;
    void* menu;
} WindowManager;

int wm_init(WindowManager* wm, int width, int height);
void wm_update(WindowManager* wm);
void wm_render(WindowManager* wm);
void wm_cleanup(WindowManager* wm);
void wm_handle_keyboard_event(WindowManager* wm, void* event);
void wm_handle_mouse_event(WindowManager* wm, void* event);
Window* create_window(const char* title, int x, int y, int width, int height);
void close_window(Window* window);
void resize_window(Window* window, int width, int height);
void move_window(Window* window, int x, int y);
void update_window(Window* window);
void fill_window(Window* window, int color);
void* load_app(const char* app_name);

#endif /* WINDOW_MANAGER_H */
