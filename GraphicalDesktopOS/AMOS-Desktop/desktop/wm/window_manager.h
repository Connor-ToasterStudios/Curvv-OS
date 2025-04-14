/*
 * window_manager.h - AMOS Desktop Window Manager Interface
 *
 * This header defines the public interface for the window manager
 * component of the AMOS desktop environment.
 */

#ifndef AMOS_WINDOW_MANAGER_H
#define AMOS_WINDOW_MANAGER_H

#include <stdbool.h>

/* Forward declarations */
struct window;
struct window_manager;

/* Window event types */
typedef enum {
    WM_EVENT_NONE = 0,
    WM_EVENT_MOUSE_MOVE,
    WM_EVENT_MOUSE_PRESS,
    WM_EVENT_MOUSE_RELEASE,
    WM_EVENT_KEY_PRESS,
    WM_EVENT_KEY_RELEASE,
    WM_EVENT_WINDOW_CLOSE,
    WM_EVENT_WINDOW_RESIZE,
    WM_EVENT_WINDOW_MOVE,
    WM_EVENT_WINDOW_FOCUS,
    WM_EVENT_WINDOW_BLUR
} wm_event_type_t;

/* Window event structure */
typedef struct {
    wm_event_type_t type;
    struct window* window;
    int x;              /* Mouse X coordinate or window X position */
    int y;              /* Mouse Y coordinate or window Y position */
    int width;          /* For resize events */
    int height;         /* For resize events */
    unsigned int key;   /* For keyboard events */
    unsigned int state; /* Modifier keys state */
} wm_event_t;

/* Window event handler function type */
typedef bool (*window_event_handler_t)(struct window* window, wm_event_t* event, void* user_data);

/*
 * Initialize the window manager
 * fb_mem: Pointer to framebuffer memory
 * width: Screen width
 * height: Screen height
 * depth: Color depth in bits per pixel
 * Returns 0 on success, -1 on failure
 */
int window_manager_init(char* fb_mem, int width, int height, int depth);

/*
 * Start the window manager main loop
 * This function doesn't return until window_manager_exit is called
 */
void window_manager_run(void);

/*
 * Exit the window manager main loop
 */
void window_manager_exit(void);

/*
 * Clean up window manager resources
 */
void window_manager_cleanup(void);

/*
 * Create a new window
 * x, y: Window position
 * width, height: Window dimensions
 * title: Window title
 * Returns window handle on success, NULL on failure
 */
struct window* window_create(int x, int y, int width, int height, const char* title);

/*
 * Destroy a window
 */
void window_destroy(struct window* window);

/*
 * Set window event handler
 * The handler function will be called for events on this window
 */
void window_set_event_handler(struct window* window, window_event_handler_t handler, void* user_data);

/*
 * Set window visibility
 */
void window_set_visible(struct window* window, bool visible);

/*
 * Move window to new position
 */
void window_move(struct window* window, int x, int y);

/*
 * Resize window
 */
void window_resize(struct window* window, int width, int height);

/*
 * Set window title
 */
void window_set_title(struct window* window, const char* title);

/*
 * Get window dimensions
 */
void window_get_size(struct window* window, int* width, int* height);

/*
 * Get window position
 */
void window_get_position(struct window* window, int* x, int* y);

/*
 * Set window as active (bring to front)
 */
void window_activate(struct window* window);

/*
 * Make window fullscreen
 */
void window_set_fullscreen(struct window* window, bool fullscreen);

/*
 * Minimize window
 */
void window_minimize(struct window* window);

/*
 * Maximize window
 */
void window_maximize(struct window* window);

/*
 * Restore window from minimized/maximized state
 */
void window_restore(struct window* window);

/*
 * Check if window is visible
 */
bool window_is_visible(struct window* window);

#endif /* AMOS_WINDOW_MANAGER_H */