/*
 * window.c - Window management implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "window.h"
#include "../ui/toolkit.h"
#include "../ui/themes.h"
#include "../state/state_manager.h"

#define MAX_MANAGED_WINDOWS 128
#define TITLEBAR_HEIGHT 20
#define BORDER_WIDTH 4
#define BUTTON_SIZE 16
#define BUTTON_MARGIN 2

// Global display and root window references
static Display *display;
static Window root;
static int screen;
static Theme *current_theme;

// Atoms for window management
static Atom wm_protocols;
static Atom wm_delete;
static Atom wm_state;
static Atom wm_name;

// Window management data
static WMWindow windows[MAX_MANAGED_WINDOWS];
static int window_count = 0;

// Initialize window management
void init_window_management(Display *dpy, Window root_win, int scr, Theme *theme) {
    display = dpy;
    root = root_win;
    screen = scr;
    current_theme = theme;

    // Initialize atoms
    wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_state = XInternAtom(display, "WM_STATE", False);
    wm_name = XInternAtom(display, "WM_NAME", False);

    // Initialize the window array
    for (int i = 0; i < MAX_MANAGED_WINDOWS; i++) {
        windows[i].window = None;
        windows[i].frame = None;
        windows[i].titlebar = None;
        windows[i].close_button = None;
        windows[i].min_button = None;
        windows[i].max_button = None;
        windows[i].icon = None;
        windows[i].title = NULL;
        windows[i].is_focused = 0;
        windows[i].is_fullscreen = 0;
        windows[i].is_minimized = 0;
        windows[i].is_shaded = 0;
        windows[i].tab_id = -1;
        windows[i].group_id = -1;
    }
}

// Find a WMWindow by its window ID
WMWindow *find_window(Window win) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].window == win || windows[i].frame == win ||
            windows[i].titlebar == win || windows[i].close_button == win ||
            windows[i].min_button == win || windows[i].max_button == win) {
            return &windows[i];
        }
    }
    return NULL;
}

// Find the index of a window in our management array
int find_window_index(Window win) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].window == win) {
            return i;
        }
    }
    return -1;
}

// Create a window frame for a client window
Window create_window_frame(Window win, XWindowAttributes *attr) {
    // Create the frame window
    Window frame = XCreateSimpleWindow(
        display, root,
        attr->x, attr->y,
        attr->width, attr->height + TITLEBAR_HEIGHT,
        BORDER_WIDTH, current_theme->border_color, current_theme->bg_color
    );

    // Create the titlebar
    Window titlebar = XCreateSimpleWindow(
        display, frame,
        0, 0,
        attr->width, TITLEBAR_HEIGHT,
        0, current_theme->border_color, current_theme->title_bg_color
    );

    // Create buttons
    int button_x = attr->width - BUTTON_SIZE - BUTTON_MARGIN;
    
    // Close button
    Window close_button = XCreateSimpleWindow(
        display, titlebar,
        button_x, BUTTON_MARGIN,
        BUTTON_SIZE, BUTTON_SIZE,
        1, current_theme->border_color, current_theme->button_bg_color
    );
    button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
    
    // Maximize button
    Window max_button = XCreateSimpleWindow(
        display, titlebar,
        button_x, BUTTON_MARGIN,
        BUTTON_SIZE, BUTTON_SIZE,
        1, current_theme->border_color, current_theme->button_bg_color
    );
    button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
    
    // Minimize button
    Window min_button = XCreateSimpleWindow(
        display, titlebar,
        button_x, BUTTON_MARGIN,
        BUTTON_SIZE, BUTTON_SIZE,
        1, current_theme->border_color, current_theme->button_bg_color
    );

    // Get window name/title
    char *window_name = NULL;
    XTextProperty text_prop;
    if (XGetWMName(display, win, &text_prop) && text_prop.value) {
        window_name = strdup((char *)text_prop.value);
        XFree(text_prop.value);
    } else {
        window_name = strdup("Untitled");
    }

    // Store the window in our management array
    int idx = window_count++;
    windows[idx].window = win;
    windows[idx].frame = frame;
    windows[idx].titlebar = titlebar;
    windows[idx].close_button = close_button;
    windows[idx].min_button = min_button;
    windows[idx].max_button = max_button;
    windows[idx].x = attr->x;
    windows[idx].y = attr->y;
    windows[idx].width = attr->width;
    windows[idx].height = attr->height;
    windows[idx].border_width = BORDER_WIDTH;
    windows[idx].titlebar_height = TITLEBAR_HEIGHT;
    windows[idx].title = window_name;
    windows[idx].is_focused = 0;
    windows[idx].is_fullscreen = 0;
    windows[idx].is_minimized = 0;
    windows[idx].is_shaded = 0;

    // Select events for the frame and its components
    XSelectInput(display, frame, SubstructureRedirectMask | SubstructureNotifyMask |
                              ButtonPressMask | ButtonReleaseMask | 
                              PointerMotionMask | EnterWindowMask | LeaveWindowMask);
    
    XSelectInput(display, titlebar, ButtonPressMask | ButtonReleaseMask | 
                               PointerMotionMask | ExposureMask);
    
    XSelectInput(display, close_button, ButtonPressMask | ButtonReleaseMask | 
                                  EnterWindowMask | LeaveWindowMask | ExposureMask);
    
    XSelectInput(display, max_button, ButtonPressMask | ButtonReleaseMask | 
                                EnterWindowMask | LeaveWindowMask | ExposureMask);
    
    XSelectInput(display, min_button, ButtonPressMask | ButtonReleaseMask | 
                                EnterWindowMask | LeaveWindowMask | ExposureMask);

    // Map the subwindows
    XMapWindow(display, titlebar);
    XMapWindow(display, close_button);
    XMapWindow(display, max_button);
    XMapWindow(display, min_button);

    // Draw the window decorations
    draw_window_titlebar(win, 0);

    return frame;
}

// Set up the window frame and reparent the client window
void setup_window_frame(Window win, XWindowAttributes *attr) {
    // Create the frame
    Window frame = create_window_frame(win, attr);
    
    // Reparent the client window to the frame
    XReparentWindow(display, win, frame, 0, TITLEBAR_HEIGHT);
    
    // Add save-set to recover windows if WM crashes
    XAddToSaveSet(display, win);
    
    // Show the frame
    XMapWindow(display, frame);
}

// Draw window titlebar with title and buttons
void draw_window_titlebar(Window win, int is_focused) {
    WMWindow *w = find_window(win);
    if (!w) return;

    // Get the right colors based on focus state
    unsigned long bg_color = is_focused ? 
        current_theme->title_active_bg_color : current_theme->title_bg_color;
    unsigned long fg_color = is_focused ? 
        current_theme->title_active_fg_color : current_theme->title_fg_color;

    // Set the window's focus state
    w->is_focused = is_focused;

    // Set the background color
    XSetWindowBackground(display, w->titlebar, bg_color);
    XClearWindow(display, w->titlebar);

    // Draw the title text
    GC gc = XCreateGC(display, w->titlebar, 0, NULL);
    XSetForeground(display, gc, fg_color);
    
    if (w->title) {
        XDrawString(display, w->titlebar, gc, 5, 15, w->title, strlen(w->title));
    }

    // Draw the button symbols
    XSetForeground(display, gc, current_theme->button_fg_color);
    
    // Close button (X)
    XDrawLine(display, w->close_button, gc, 3, 3, BUTTON_SIZE-3, BUTTON_SIZE-3);
    XDrawLine(display, w->close_button, gc, 3, BUTTON_SIZE-3, BUTTON_SIZE-3, 3);
    
    // Maximize button (square)
    XDrawRectangle(display, w->max_button, gc, 3, 3, BUTTON_SIZE-6, BUTTON_SIZE-6);
    
    // Minimize button (line)
    XDrawLine(display, w->min_button, gc, 3, BUTTON_SIZE/2, BUTTON_SIZE-3, BUTTON_SIZE/2);
    
    // Free graphics context
    XFreeGC(display, gc);
}

// Get the part of the frame that the pointer is in
FramePart get_frame_part(Window frame, int x, int y) {
    WMWindow *w = find_window(frame);
    if (!w) return FRAME_NONE;

    // Check if in titlebar
    if (y < w->titlebar_height) {
        return FRAME_TITLEBAR;
    }

    // Check borders and corners
    int width = w->width + 2 * w->border_width;
    int height = w->height + w->titlebar_height + 2 * w->border_width;
    int corner_size = w->border_width * 2;

    // Check corners first
    if (x < corner_size && y < corner_size + w->titlebar_height) {
        return FRAME_CORNER_NW;
    }
    if (x >= width - corner_size && y < corner_size + w->titlebar_height) {
        return FRAME_CORNER_NE;
    }
    if (x < corner_size && y >= height - corner_size) {
        return FRAME_CORNER_SW;
    }
    if (x >= width - corner_size && y >= height - corner_size) {
        return FRAME_CORNER_SE;
    }

    // Then check borders
    if (y < w->border_width + w->titlebar_height) {
        return FRAME_BORDER_N;
    }
    if (x >= width - w->border_width) {
        return FRAME_BORDER_E;
    }
    if (y >= height - w->border_width) {
        return FRAME_BORDER_S;
    }
    if (x < w->border_width) {
        return FRAME_BORDER_W;
    }

    return FRAME_NONE;
}

// Resize a window
void resize_window(Window win, int width, int height) {
    WMWindow *w = find_window(win);
    if (!w) return;

    if (width < 50) width = 50;
    if (height < 50) height = 50;

    w->width = width;
    w->height = height;

    // Resize the frame
    XResizeWindow(display, w->frame, width, height + w->titlebar_height);
    
    // Resize the titlebar
    XResizeWindow(display, w->titlebar, width, w->titlebar_height);
    
    // Reposition the buttons
    int button_x = width - BUTTON_SIZE - BUTTON_MARGIN;
    XMoveWindow(display, w->close_button, button_x, BUTTON_MARGIN);
    button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
    XMoveWindow(display, w->max_button, button_x, BUTTON_MARGIN);
    button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
    XMoveWindow(display, w->min_button, button_x, BUTTON_MARGIN);
    
    // Resize the actual client window
    XResizeWindow(display, w->window, width, height);

    // Update the state
    save_window_state(win);
}

// Move a window
void move_window(Window win, int x, int y) {
    WMWindow *w = find_window(win);
    if (!w) return;

    w->x = x;
    w->y = y;

    // Move the frame
    XMoveWindow(display, w->frame, x, y);
    
    // Update the state
    save_window_state(win);
}

// Minimize a window
void minimize_window(Window win) {
    WMWindow *w = find_window(win);
    if (!w || w->is_minimized) return;

    // Hide the window
    XUnmapWindow(display, w->frame);
    w->is_minimized = 1;
    
    // Update the state
    save_window_state(win);
}

// Maximize a window
void maximize_window(Window win) {
    WMWindow *w = find_window(win);
    if (!w) return;

    // If already maximized, restore it
    if (w->is_fullscreen) {
        restore_window(win);
        return;
    }

    // Save current size for restoration
    save_window_state(win);

    // Get screen dimensions
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Set to fullscreen
    w->is_fullscreen = 1;
    
    // Resize and move to fill the screen
    XMoveResizeWindow(display, w->frame, 0, 0, 
                     screen_width, screen_height);
    XResizeWindow(display, w->window, 
                 screen_width, screen_height - w->titlebar_height);
    
    // Resize the titlebar
    XResizeWindow(display, w->titlebar, screen_width, w->titlebar_height);
    
    // Reposition the buttons
    int button_x = screen_width - BUTTON_SIZE - BUTTON_MARGIN;
    XMoveWindow(display, w->close_button, button_x, BUTTON_MARGIN);
    button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
    XMoveWindow(display, w->max_button, button_x, BUTTON_MARGIN);
    button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
    XMoveWindow(display, w->min_button, button_x, BUTTON_MARGIN);
    
    // Update the window dimensions in our state
    w->width = screen_width;
    w->height = screen_height - w->titlebar_height;
    w->x = 0;
    w->y = 0;
}

// Restore a window from minimized or maximized state
void restore_window(Window win) {
    WMWindow *w = find_window(win);
    if (!w) return;

    if (w->is_minimized) {
        // Restore from minimized state
        XMapWindow(display, w->frame);
        w->is_minimized = 0;
    } else if (w->is_fullscreen) {
        // Restore from maximized state
        w->is_fullscreen = 0;
        
        // Get the saved window state
        WindowState state;
        if (get_window_state(win, &state)) {
            // Restore the saved geometry
            XMoveResizeWindow(display, w->frame, 
                             state.x, state.y, 
                             state.width, state.height + w->titlebar_height);
            XResizeWindow(display, w->window, 
                         state.width, state.height);
            
            // Resize the titlebar
            XResizeWindow(display, w->titlebar, state.width, w->titlebar_height);
            
            // Reposition the buttons
            int button_x = state.width - BUTTON_SIZE - BUTTON_MARGIN;
            XMoveWindow(display, w->close_button, button_x, BUTTON_MARGIN);
            button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
            XMoveWindow(display, w->max_button, button_x, BUTTON_MARGIN);
            button_x -= (BUTTON_SIZE + BUTTON_MARGIN);
            XMoveWindow(display, w->min_button, button_x, BUTTON_MARGIN);
            
            // Update our state
            w->width = state.width;
            w->height = state.height;
            w->x = state.x;
            w->y = state.y;
        }
    }
    
    // Update the state
    save_window_state(win);
}

// Close a window
void close_window(Window win) {
    WMWindow *w = find_window(win);
    if (!w) return;

    // Try to send WM_DELETE message first
    int can_delete = 0;
    Atom *protocols;
    int num_protocols;
    
    if (XGetWMProtocols(display, w->window, &protocols, &num_protocols)) {
        for (int i = 0; i < num_protocols; i++) {
            if (protocols[i] == wm_delete) {
                can_delete = 1;
                break;
            }
        }
        XFree(protocols);
    }
    
    if (can_delete) {
        // Send WM_DELETE message
        XEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.xclient.window = w->window;
        ev.xclient.message_type = wm_protocols;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wm_delete;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(display, w->window, False, NoEventMask, &ev);
    } else {
        // Forcibly destroy the window
        XDestroyWindow(display, w->window);
    }
}

// Focus a window
void focus_window(Window win) {
    WMWindow *w = find_window(win);
    if (!w) return;

    // Unfocus currently focused windows
    for (int i = 0; i < window_count; i++) {
        if (windows[i].is_focused && windows[i].window != win) {
            draw_window_titlebar(windows[i].window, 0);
        }
    }

    // Focus the window
    XSetInputFocus(display, w->window, RevertToPointerRoot, CurrentTime);
    draw_window_titlebar(win, 1);
    XRaiseWindow(display, w->frame);
}

// Unfocus a window
void unfocus_window(Window win) {
    WMWindow *w = find_window(win);
    if (!w) return;

    draw_window_titlebar(win, 0);
}

// Update window title
void update_window_title(Window win, const char *title) {
    WMWindow *w = find_window(win);
    if (!w) return;

    // Free the old title
    if (w->title) {
        free(w->title);
    }

    // Store the new title
    w->title = strdup(title);

    // Redraw the titlebar
    draw_window_titlebar(win, w->is_focused);
}

// Add a window to a tab group
void add_window_to_tab_group(Window win, int group_id) {
    WMWindow *w = find_window(win);
    if (!w) return;

    // Count how many windows are already in this group
    int tab_count = 0;
    for (int i = 0; i < window_count; i++) {
        if (windows[i].group_id == group_id) {
            tab_count++;
        }
    }

    // Set the group and tab ID
    w->group_id = group_id;
    w->tab_id = tab_count;
    
    // Hide all windows in the group except the active one
    for (int i = 0; i < window_count; i++) {
        if (windows[i].group_id == group_id) {
            if (windows[i].tab_id == tab_count) {
                // This is the new active tab
                XMapWindow(display, windows[i].window);
                focus_window(windows[i].window);
            } else {
                // Hide other tabs
                XUnmapWindow(display, windows[i].window);
            }
        }
    }
    
    // Save the window state
    save_window_state(win);
}

// Remove a window from a tab group
void remove_window_from_tab_group(Window win) {
    WMWindow *w = find_window(win);
    if (!w || w->group_id == -1) return;

    int old_group = w->group_id;
    
    // Remove from group
    w->group_id = -1;
    w->tab_id = -1;
    
    // Reindex the tabs in the group
    int new_tab_id = 0;
    for (int i = 0; i < window_count; i++) {
        if (windows[i].group_id == old_group) {
            windows[i].tab_id = new_tab_id++;
        }
    }
    
    // Show the window
    XMapWindow(display, w->window);
    
    // Make sure at least one tab in the group is visible
    int visible_tab = 0;
    for (int i = 0; i < window_count; i++) {
        if (windows[i].group_id == old_group && windows[i].tab_id == 0) {
            XMapWindow(display, windows[i].window);
            focus_window(windows[i].window);
            visible_tab = 1;
            break;
        }
    }
    
    // Save the window state
    save_window_state(win);
}

// Switch to tab in a group
void switch_to_tab(int group_id, int tab_id) {
    Window active_window = None;
    
    // Hide all windows in the group, then show the active one
    for (int i = 0; i < window_count; i++) {
        if (windows[i].group_id == group_id) {
            if (windows[i].tab_id == tab_id) {
                // This is the target tab to show
                XMapWindow(display, windows[i].window);
                active_window = windows[i].window;
            } else {
                // Hide other tabs
                XUnmapWindow(display, windows[i].window);
            }
        }
    }
    
    // Focus the active window
    if (active_window != None) {
        focus_window(active_window);
    }
}

// Get the number of windows we're managing
int get_window_count() {
    return window_count;
}

// Get the window at a specific index
Window get_window_at_index(int index) {
    if (index >= 0 && index < window_count) {
        return windows[index].window;
    }
    return None;
}

// Update window geometry in state management
void update_window_geometry(Window win, int x, int y, int width, int height) {
    WMWindow *w = find_window(win);
    if (!w) return;

    w->x = x;
    w->y = y;
    w->width = width;
    w->height = height;
    
    // Save state
    save_window_state(win);
}
