/*
 * window_manager.c - AMOS Desktop Window Manager Implementation
 *
 * This file implements the window manager component of the AMOS desktop environment.
 * It handles window creation, destruction, rendering, and event dispatch.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/input.h>

#include "window_manager.h"
#include "../ui/ui_toolkit.h"
#include "../state/state_manager.h"

/* Maximum number of windows the window manager can handle */
#define MAX_WINDOWS 64

/* Window flags */
#define WINDOW_FLAG_VISIBLE     0x01
#define WINDOW_FLAG_MINIMIZED   0x02
#define WINDOW_FLAG_MAXIMIZED   0x04
#define WINDOW_FLAG_FULLSCREEN  0x08
#define WINDOW_FLAG_DECORATED   0x10
#define WINDOW_FLAG_RESIZABLE   0x20
#define WINDOW_FLAG_FOCUSED     0x40

/* Window structure (private implementation) */
struct window {
    int id;                     /* Unique window ID */
    int x, y;                   /* Window position */
    int width, height;          /* Window dimensions */
    int min_width, min_height;  /* Minimum window size */
    int max_width, max_height;  /* Maximum window size (0 = no limit) */
    char* title;                /* Window title */
    int flags;                  /* Window state flags */
    
    /* Original dimensions (for restore from maximized/fullscreen) */
    int saved_x, saved_y;
    int saved_width, saved_height;
    
    /* Event handler */
    window_event_handler_t event_handler;
    void* user_data;
    
    /* Drawing buffer */
    char* buffer;               /* Window-specific drawing buffer */
};

/* Window manager structure */
struct window_manager {
    bool running;               /* Main loop running flag */
    char* fb_mem;               /* Framebuffer memory */
    int width, height, depth;   /* Screen dimensions and color depth */
    int bytes_per_pixel;        /* Bytes per pixel (calculated from depth) */
    
    /* Window management */
    struct window* windows[MAX_WINDOWS];  /* Array of window pointers */
    int window_count;                     /* Number of windows */
    struct window* active_window;         /* Currently active window */
    
    /* Input devices */
    int kbd_fd;                 /* Keyboard device file descriptor */
    int mouse_fd;               /* Mouse device file descriptor */
    int mouse_x, mouse_y;       /* Current mouse position */
    int mouse_buttons;          /* Current mouse button state */
};

/* Global window manager instance */
static struct window_manager* wm = NULL;

/* Forward declarations for internal functions */
static void wm_process_input(void);
static void wm_render(void);
static bool wm_dispatch_event(wm_event_t* event);
static struct window* wm_find_window_at(int x, int y);
static void wm_activate_window(struct window* window);

/*
 * Initialize the window manager
 */
int window_manager_init(char* fb_mem, int width, int height, int depth) {
    /* Allocate window manager structure */
    wm = (struct window_manager*)malloc(sizeof(struct window_manager));
    if (wm == NULL) {
        fprintf(stderr, "Failed to allocate window manager structure\n");
        return -1;
    }
    
    /* Initialize window manager state */
    memset(wm, 0, sizeof(struct window_manager));
    wm->fb_mem = fb_mem;
    wm->width = width;
    wm->height = height;
    wm->depth = depth;
    wm->bytes_per_pixel = depth / 8;
    wm->running = false;
    wm->window_count = 0;
    wm->active_window = NULL;
    
    /* Open input devices */
    wm->kbd_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (wm->kbd_fd == -1) {
        perror("Failed to open keyboard device");
        /* Not fatal, continue without keyboard */
    }
    
    wm->mouse_fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    if (wm->mouse_fd == -1) {
        perror("Failed to open mouse device");
        /* Not fatal, continue without mouse */
    }
    
    printf("Window manager initialized: %dx%d, %d bpp\n", width, height, depth);
    return 0;
}

/*
 * Start the window manager main loop
 */
void window_manager_run(void) {
    struct timeval tv;
    wm->running = true;
    
    printf("Window manager started\n");
    
    /* Main loop */
    while (wm->running) {
        /* Process input events */
        wm_process_input();
        
        /* Render windows */
        wm_render();
        
        /* Sleep for a short time to avoid using 100% CPU */
        tv.tv_sec = 0;
        tv.tv_usec = 16666; /* ~60 FPS */
        select(0, NULL, NULL, NULL, &tv);
    }
    
    printf("Window manager stopped\n");
}

/*
 * Exit the window manager main loop
 */
void window_manager_exit(void) {
    wm->running = false;
}

/*
 * Clean up window manager resources
 */
void window_manager_cleanup(void) {
    if (wm == NULL) {
        return;
    }
    
    /* Close input devices */
    if (wm->kbd_fd != -1) {
        close(wm->kbd_fd);
    }
    
    if (wm->mouse_fd != -1) {
        close(wm->mouse_fd);
    }
    
    /* Destroy all windows */
    for (int i = 0; i < wm->window_count; i++) {
        window_destroy(wm->windows[i]);
    }
    
    /* Free window manager structure */
    free(wm);
    wm = NULL;
}

/*
 * Create a new window
 */
struct window* window_create(int x, int y, int width, int height, const char* title) {
    struct window* window;
    
    /* Check if we can create another window */
    if (wm->window_count >= MAX_WINDOWS) {
        fprintf(stderr, "Maximum number of windows reached\n");
        return NULL;
    }
    
    /* Allocate window structure */
    window = (struct window*)malloc(sizeof(struct window));
    if (window == NULL) {
        fprintf(stderr, "Failed to allocate window structure\n");
        return NULL;
    }
    
    /* Initialize window state */
    memset(window, 0, sizeof(struct window));
    window->id = wm->window_count;
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->min_width = 100;
    window->min_height = 50;
    window->max_width = 0;  /* No maximum */
    window->max_height = 0; /* No maximum */
    
    /* Save initial dimensions for restore */
    window->saved_x = x;
    window->saved_y = y;
    window->saved_width = width;
    window->saved_height = height;
    
    /* Set default flags */
    window->flags = WINDOW_FLAG_VISIBLE | WINDOW_FLAG_DECORATED | WINDOW_FLAG_RESIZABLE;
    
    /* Copy window title */
    if (title != NULL) {
        window->title = strdup(title);
    } else {
        window->title = strdup("Untitled");
    }
    
    /* Allocate window buffer */
    window->buffer = (char*)malloc(width * height * wm->bytes_per_pixel);
    if (window->buffer == NULL) {
        fprintf(stderr, "Failed to allocate window buffer\n");
        free(window->title);
        free(window);
        return NULL;
    }
    
    /* Clear window buffer */
    memset(window->buffer, 0, width * height * wm->bytes_per_pixel);
    
    /* Add window to window manager */
    wm->windows[wm->window_count] = window;
    wm->window_count++;
    
    /* Activate new window */
    wm_activate_window(window);
    
    printf("Window created: %s (%d,%d,%d,%d)\n", window->title, x, y, width, height);
    return window;
}

/*
 * Destroy a window
 */
void window_destroy(struct window* window) {
    int i;
    
    if (window == NULL) {
        return;
    }
    
    /* Find window in window manager array */
    for (i = 0; i < wm->window_count; i++) {
        if (wm->windows[i] == window) {
            break;
        }
    }
    
    /* If window found, remove it from array */
    if (i < wm->window_count) {
        /* If this is the active window, clear it */
        if (wm->active_window == window) {
            wm->active_window = NULL;
        }
        
        /* Remove window from array and compact the array */
        for (int j = i; j < wm->window_count - 1; j++) {
            wm->windows[j] = wm->windows[j + 1];
        }
        wm->window_count--;
        
        /* Activate the next window if available */
        if (wm->window_count > 0 && wm->active_window == NULL) {
            wm_activate_window(wm->windows[wm->window_count - 1]);
        }
    }
    
    /* Free window resources */
    if (window->title != NULL) {
        free(window->title);
    }
    
    if (window->buffer != NULL) {
        free(window->buffer);
    }
    
    free(window);
    
    printf("Window destroyed\n");
}

/*
 * Set window event handler
 */
void window_set_event_handler(struct window* window, window_event_handler_t handler, void* user_data) {
    if (window != NULL) {
        window->event_handler = handler;
        window->user_data = user_data;
    }
}

/*
 * Set window visibility
 */
void window_set_visible(struct window* window, bool visible) {
    if (window == NULL) {
        return;
    }
    
    if (visible) {
        window->flags |= WINDOW_FLAG_VISIBLE;
    } else {
        window->flags &= ~WINDOW_FLAG_VISIBLE;
    }
}

/*
 * Move window to new position
 */
void window_move(struct window* window, int x, int y) {
    if (window == NULL) {
        return;
    }
    
    window->x = x;
    window->y = y;
}

/*
 * Resize window
 */
void window_resize(struct window* window, int width, int height) {
    char* new_buffer;
    
    if (window == NULL) {
        return;
    }
    
    /* Enforce minimum and maximum size constraints */
    if (width < window->min_width) {
        width = window->min_width;
    }
    
    if (height < window->min_height) {
        height = window->min_height;
    }
    
    if (window->max_width > 0 && width > window->max_width) {
        width = window->max_width;
    }
    
    if (window->max_height > 0 && height > window->max_height) {
        height = window->max_height;
    }
    
    /* Skip if size hasn't changed */
    if (width == window->width && height == window->height) {
        return;
    }
    
    /* Allocate new buffer */
    new_buffer = (char*)malloc(width * height * wm->bytes_per_pixel);
    if (new_buffer == NULL) {
        fprintf(stderr, "Failed to allocate new window buffer\n");
        return;
    }
    
    /* Clear new buffer */
    memset(new_buffer, 0, width * height * wm->bytes_per_pixel);
    
    /* Copy as much of the old buffer as will fit */
    for (int y = 0; y < height && y < window->height; y++) {
        int line_width = (width < window->width) ? width : window->width;
        memcpy(
            new_buffer + (y * width * wm->bytes_per_pixel),
            window->buffer + (y * window->width * wm->bytes_per_pixel),
            line_width * wm->bytes_per_pixel
        );
    }
    
    /* Free old buffer and update window */
    free(window->buffer);
    window->buffer = new_buffer;
    window->width = width;
    window->height = height;
    
    /* Dispatch resize event */
    wm_event_t event;
    event.type = WM_EVENT_WINDOW_RESIZE;
    event.window = window;
    event.width = width;
    event.height = height;
    wm_dispatch_event(&event);
}

/*
 * Set window title
 */
void window_set_title(struct window* window, const char* title) {
    if (window == NULL || title == NULL) {
        return;
    }
    
    if (window->title != NULL) {
        free(window->title);
    }
    
    window->title = strdup(title);
}

/*
 * Get window dimensions
 */
void window_get_size(struct window* window, int* width, int* height) {
    if (window != NULL) {
        if (width != NULL) {
            *width = window->width;
        }
        
        if (height != NULL) {
            *height = window->height;
        }
    }
}

/*
 * Get window position
 */
void window_get_position(struct window* window, int* x, int* y) {
    if (window != NULL) {
        if (x != NULL) {
            *x = window->x;
        }
        
        if (y != NULL) {
            *y = window->y;
        }
    }
}

/*
 * Set window as active (bring to front)
 */
void window_activate(struct window* window) {
    if (window != NULL) {
        wm_activate_window(window);
    }
}

/*
 * Make window fullscreen
 */
void window_set_fullscreen(struct window* window, bool fullscreen) {
    if (window == NULL) {
        return;
    }
    
    if (fullscreen && !(window->flags & WINDOW_FLAG_FULLSCREEN)) {
        /* Save current window state for restore */
        window->saved_x = window->x;
        window->saved_y = window->y;
        window->saved_width = window->width;
        window->saved_height = window->height;
        
        /* Set fullscreen dimensions */
        window->x = 0;
        window->y = 0;
        window_resize(window, wm->width, wm->height);
        
        /* Set fullscreen flag */
        window->flags |= WINDOW_FLAG_FULLSCREEN;
    } else if (!fullscreen && (window->flags & WINDOW_FLAG_FULLSCREEN)) {
        /* Restore window dimensions */
        window->x = window->saved_x;
        window->y = window->saved_y;
        window_resize(window, window->saved_width, window->saved_height);
        
        /* Clear fullscreen flag */
        window->flags &= ~WINDOW_FLAG_FULLSCREEN;
    }
}

/*
 * Minimize window
 */
void window_minimize(struct window* window) {
    if (window == NULL) {
        return;
    }
    
    /* Set minimized flag */
    window->flags |= WINDOW_FLAG_MINIMIZED;
    
    /* If this is active window, activate another */
    if (wm->active_window == window) {
        wm->active_window = NULL;
        
        /* Find next visible window to activate */
        for (int i = wm->window_count - 1; i >= 0; i--) {
            if (wm->windows[i] != window && 
                (wm->windows[i]->flags & WINDOW_FLAG_VISIBLE) && 
                !(wm->windows[i]->flags & WINDOW_FLAG_MINIMIZED)) {
                wm_activate_window(wm->windows[i]);
                break;
            }
        }
    }
}

/*
 * Maximize window
 */
void window_maximize(struct window* window) {
    if (window == NULL) {
        return;
    }
    
    if (!(window->flags & WINDOW_FLAG_MAXIMIZED)) {
        /* Save current window state for restore */
        window->saved_x = window->x;
        window->saved_y = window->y;
        window->saved_width = window->width;
        window->saved_height = window->height;
        
        /* Set maximized dimensions */
        window->x = 0;
        window->y = 0;
        window_resize(window, wm->width, wm->height);
        
        /* Set maximized flag */
        window->flags |= WINDOW_FLAG_MAXIMIZED;
    }
}

/*
 * Restore window from minimized/maximized state
 */
void window_restore(struct window* window) {
    if (window == NULL) {
        return;
    }
    
    if (window->flags & WINDOW_FLAG_MINIMIZED) {
        /* Clear minimized flag */
        window->flags &= ~WINDOW_FLAG_MINIMIZED;
        
        /* Activate window */
        wm_activate_window(window);
    } else if (window->flags & WINDOW_FLAG_MAXIMIZED || window->flags & WINDOW_FLAG_FULLSCREEN) {
        /* Restore window dimensions */
        window->x = window->saved_x;
        window->y = window->saved_y;
        window_resize(window, window->saved_width, window->saved_height);
        
        /* Clear maximized and fullscreen flags */
        window->flags &= ~(WINDOW_FLAG_MAXIMIZED | WINDOW_FLAG_FULLSCREEN);
    }
}

/*
 * Check if window is visible
 */
bool window_is_visible(struct window* window) {
    if (window == NULL) {
        return false;
    }
    
    return (window->flags & WINDOW_FLAG_VISIBLE) && !(window->flags & WINDOW_FLAG_MINIMIZED);
}

/*
 * Process input events from keyboard and mouse
 */
static void wm_process_input(void) {
    struct input_event event;
    ssize_t bytes_read;
    
    /* Process keyboard input */
    if (wm->kbd_fd != -1) {
        while ((bytes_read = read(wm->kbd_fd, &event, sizeof(event))) > 0) {
            if (bytes_read == sizeof(event)) {
                /* Process key event... */
                /* TODO: Implement keyboard input handling */
            }
        }
    }
    
    /* Process mouse input */
    if (wm->mouse_fd != -1) {
        while ((bytes_read = read(wm->mouse_fd, &event, sizeof(event))) > 0) {
            if (bytes_read == sizeof(event)) {
                /* Process mouse event... */
                /* TODO: Implement mouse input handling */
            }
        }
    }
}

/*
 * Render all windows to the framebuffer
 */
static void wm_render(void) {
    /* Clear framebuffer (background) */
    /* TODO: Replace with actual desktop background rendering */
    memset(wm->fb_mem, 0, wm->width * wm->height * wm->bytes_per_pixel);
    
    /* Render each visible window (from back to front) */
    for (int i = 0; i < wm->window_count; i++) {
        struct window* window = wm->windows[i];
        
        /* Skip invisible windows */
        if (!window_is_visible(window)) {
            continue;
        }
        
        /* Calculate visible window area */
        int x1 = (window->x < 0) ? 0 : window->x;
        int y1 = (window->y < 0) ? 0 : window->y;
        int x2 = (window->x + window->width > wm->width) ? wm->width : window->x + window->width;
        int y2 = (window->y + window->height > wm->height) ? wm->height : window->y + window->height;
        
        /* Skip if window is entirely off-screen */
        if (x1 >= x2 || y1 >= y2) {
            continue;
        }
        
        /* Render window decorations if needed */
        if (window->flags & WINDOW_FLAG_DECORATED) {
            /* TODO: Implement window decoration rendering */
        }
        
        /* Render window content */
        for (int y = y1; y < y2; y++) {
            for (int x = x1; x < x2; x++) {
                /* Calculate source and destination offsets */
                int src_offset = ((y - window->y) * window->width + (x - window->x)) * wm->bytes_per_pixel;
                int dst_offset = (y * wm->width + x) * wm->bytes_per_pixel;
                
                /* Copy pixel from window buffer to framebuffer */
                memcpy(wm->fb_mem + dst_offset, window->buffer + src_offset, wm->bytes_per_pixel);
            }
        }
    }
}

/*
 * Dispatch an event to the appropriate window
 * Returns true if the event was handled, false otherwise
 */
static bool wm_dispatch_event(wm_event_t* event) {
    struct window* window = event->window;
    
    if (window != NULL && window->event_handler != NULL) {
        return window->event_handler(window, event, window->user_data);
    }
    
    return false;
}

/*
 * Find the topmost window containing the given point
 */
static struct window* wm_find_window_at(int x, int y) {
    /* Search from front to back */
    for (int i = wm->window_count - 1; i >= 0; i--) {
        struct window* window = wm->windows[i];
        
        /* Skip invisible windows */
        if (!window_is_visible(window)) {
            continue;
        }
        
        /* Check if point is inside window */
        if (x >= window->x && x < window->x + window->width &&
            y >= window->y && y < window->y + window->height) {
            return window;
        }
    }
    
    /* No window found */
    return NULL;
}

/*
 * Activate a window (bring to front and give keyboard focus)
 */
static void wm_activate_window(struct window* window) {
    if (window == NULL) {
        return;
    }
    
    /* Skip if already active */
    if (wm->active_window == window) {
        return;
    }
    
    /* Deactivate previous active window */
    if (wm->active_window != NULL) {
        wm->active_window->flags &= ~WINDOW_FLAG_FOCUSED;
        
        /* Dispatch blur event */
        wm_event_t event;
        event.type = WM_EVENT_WINDOW_BLUR;
        event.window = wm->active_window;
        wm_dispatch_event(&event);
    }
    
    /* Set new active window */
    wm->active_window = window;
    window->flags |= WINDOW_FLAG_FOCUSED;
    
    /* Reorder windows to bring active to front */
    for (int i = 0; i < wm->window_count; i++) {
        if (wm->windows[i] == window) {
            /* Move window to end of array (top of stack) */
            for (int j = i; j < wm->window_count - 1; j++) {
                wm->windows[j] = wm->windows[j + 1];
            }
            wm->windows[wm->window_count - 1] = window;
            break;
        }
    }
    
    /* Dispatch focus event */
    wm_event_t event;
    event.type = WM_EVENT_WINDOW_FOCUS;
    event.window = window;
    wm_dispatch_event(&event);
}