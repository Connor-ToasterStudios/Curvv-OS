/*
 * window.h - Window management functions for the window manager
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>
#include "../ui/themes.h"

// Window frame parts
typedef enum {
    FRAME_NONE,
    FRAME_TITLEBAR,
    FRAME_BORDER_N,
    FRAME_BORDER_E,
    FRAME_BORDER_S,
    FRAME_BORDER_W,
    FRAME_CORNER_NW,
    FRAME_CORNER_NE,
    FRAME_CORNER_SW,
    FRAME_CORNER_SE
} FramePart;

// Window structure to keep track of window state
typedef struct {
    Window window;
    Window frame;
    Window titlebar;
    Window close_button;
    Window min_button;
    Window max_button;
    Window icon;
    int x, y;
    int width, height;
    int border_width;
    int titlebar_height;
    char *title;
    int is_focused;
    int is_fullscreen;
    int is_minimized;
    int is_shaded;
    int tab_id;
    int group_id;
} WMWindow;

// Initialize window management
void init_window_management(Display *dpy, Window root, int scr, Theme *theme);

// Create a frame for a window
Window create_window_frame(Window win, XWindowAttributes *attr);

// Setup the window decoration and frame
void setup_window_frame(Window win, XWindowAttributes *attr);

// Draw the window titlebar
void draw_window_titlebar(Window win, int is_focused);

// Get the frame part that a point is in
FramePart get_frame_part(Window frame, int x, int y);

// Find the window management structure for a window
WMWindow *find_window(Window win);

// Resize a window
void resize_window(Window win, int width, int height);

// Move a window
void move_window(Window win, int x, int y);

// Minimize a window
void minimize_window(Window win);

// Maximize a window
void maximize_window(Window win);

// Restore a window
void restore_window(Window win);

// Close a window
void close_window(Window win);

// Focus a window
void focus_window(Window win);

// Unfocus a window
void unfocus_window(Window win);

// Update window title
void update_window_title(Window win, const char *title);

// Add a window to a tab group
void add_window_to_tab_group(Window win, int group_id);

// Remove a window from a tab group
void remove_window_from_tab_group(Window win);

// Switch to tab in a group
void switch_to_tab(int group_id, int tab_id);

// Get the number of windows we're managing
int get_window_count();

// Get the window at a specific index
Window get_window_at_index(int index);

// Update window geometry in state management
void update_window_geometry(Window win, int x, int y, int width, int height);

#endif /* WINDOW_H */
