/*
 * window_manager.c - Lightweight window manager for AMOS
 * 
 * This implements a Fluxbox-like window manager with tabbed window management,
 * resizable windows, and a lightweight UI toolkit integration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include "window.h"
#include "panel.h"
#include "menu.h"
#include "events.h"
#include "../state/state_manager.h"
#include "../ui/toolkit.h"
#include "../ui/themes.h"

#define MAX_WINDOWS 128

// Global display and root window
Display *display;
Window root;
int screen;
XWindowAttributes attr;

// Window management data structures
Window windows[MAX_WINDOWS];
int window_count = 0;
Window active_window = None;
Window focused_window = None;

// Theme and configuration
Theme current_theme;

// Cursor definitions for different window operations
Cursor cursor_normal;
Cursor cursor_move;
Cursor cursor_resize_nw;
Cursor cursor_resize_ne;
Cursor cursor_resize_sw;
Cursor cursor_resize_se;
Cursor cursor_resize_n;
Cursor cursor_resize_e;
Cursor cursor_resize_s;
Cursor cursor_resize_w;

// Resize mode tracking
enum ResizeMode {
    RESIZE_NONE,
    RESIZE_N,
    RESIZE_NE,
    RESIZE_E,
    RESIZE_SE,
    RESIZE_S,
    RESIZE_SW,
    RESIZE_W,
    RESIZE_NW,
    MOVE
} resize_mode = RESIZE_NONE;

// Window move/resize operation tracking
int drag_start_x, drag_start_y;
int window_start_x, window_start_y;
int window_start_width, window_start_height;

// Initialize cursors for various window operations
void init_cursors() {
    cursor_normal = XCreateFontCursor(display, XC_left_ptr);
    cursor_move = XCreateFontCursor(display, XC_fleur);
    cursor_resize_nw = XCreateFontCursor(display, XC_top_left_corner);
    cursor_resize_ne = XCreateFontCursor(display, XC_top_right_corner);
    cursor_resize_sw = XCreateFontCursor(display, XC_bottom_left_corner);
    cursor_resize_se = XCreateFontCursor(display, XC_bottom_right_corner);
    cursor_resize_n = XCreateFontCursor(display, XC_top_side);
    cursor_resize_e = XCreateFontCursor(display, XC_right_side);
    cursor_resize_s = XCreateFontCursor(display, XC_bottom_side);
    cursor_resize_w = XCreateFontCursor(display, XC_left_side);
}

// Initialize the window manager
int init_window_manager() {
    // Open display connection
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Cannot open display\n");
        return 0;
    }

    // Get the root window
    screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    XGetWindowAttributes(display, root, &attr);

    // Initialize state management
    init_state_manager();

    // Initialize UI toolkit
    init_toolkit(display, root, screen);

    // Initialize theme
    load_theme(&current_theme, "default");

    // Initialize the panel
    init_panel(display, root, screen, &current_theme);

    // Initialize the menu system
    init_menu(display, root, screen, &current_theme);

    // Initialize cursors
    init_cursors();

    // Set up event handlers
    setup_event_handlers();

    // Grab the root window events we're interested in
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask |
                              ButtonPressMask | ButtonReleaseMask | 
                              PointerMotionMask | KeyPressMask);

    // Let X know we want to handle these events
    XGrabServer(display);
    XSetErrorHandler(error_handler);
    XUngrabServer(display);
    XSync(display, False);

    // Initialize widgets
    init_widgets();

    printf("Window manager initialized successfully\n");
    return 1;
}

// Handle a new window creation
void handle_new_window(Window w) {
    if (window_count >= MAX_WINDOWS) {
        fprintf(stderr, "Maximum number of windows reached\n");
        return;
    }

    // Get window attributes
    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, w, &attr)) {
        fprintf(stderr, "Failed to get window attributes\n");
        return;
    }

    // Skip windows we don't want to manage
    if (attr.override_redirect) {
        return;
    }

    printf("Managing new window: %lu\n", w);

    // Add window to our management list
    windows[window_count++] = w;

    // Select the events we want to receive for this window
    XSelectInput(display, w, EnterWindowMask | LeaveWindowMask | PropertyChangeMask |
                           StructureNotifyMask | FocusChangeMask);

    // Setup window decoration and frame
    setup_window_frame(w, &attr);

    // Add window to state management
    add_window_state(w);

    // Map the window
    XMapWindow(display, w);

    // Set focus to this window
    set_focus(w);
}

// Handle window resize operation
void handle_resize(Window w, int pointer_x, int pointer_y) {
    if (resize_mode == RESIZE_NONE) {
        return;
    }

    // Get the current geometry
    XWindowAttributes attr;
    XGetWindowAttributes(display, w, &attr);

    int new_x = attr.x;
    int new_y = attr.y;
    int new_width = attr.width;
    int new_height = attr.height;

    int dx = pointer_x - drag_start_x;
    int dy = pointer_y - drag_start_y;

    // Apply resize operations based on the resize mode
    switch (resize_mode) {
        case MOVE:
            new_x = window_start_x + dx;
            new_y = window_start_y + dy;
            break;
        case RESIZE_N:
            new_y = window_start_y + dy;
            new_height = window_start_height - dy;
            break;
        case RESIZE_S:
            new_height = window_start_height + dy;
            break;
        case RESIZE_E:
            new_width = window_start_width + dx;
            break;
        case RESIZE_W:
            new_x = window_start_x + dx;
            new_width = window_start_width - dx;
            break;
        case RESIZE_NW:
            new_x = window_start_x + dx;
            new_y = window_start_y + dy;
            new_width = window_start_width - dx;
            new_height = window_start_height - dy;
            break;
        case RESIZE_NE:
            new_y = window_start_y + dy;
            new_width = window_start_width + dx;
            new_height = window_start_height - dy;
            break;
        case RESIZE_SW:
            new_x = window_start_x + dx;
            new_width = window_start_width - dx;
            new_height = window_start_height + dy;
            break;
        case RESIZE_SE:
            new_width = window_start_width + dx;
            new_height = window_start_height + dy;
            break;
        default:
            break;
    }

    // Ensure minimum window size
    if (new_width < 50) new_width = 50;
    if (new_height < 50) new_height = 50;

    // Apply the new geometry
    XMoveResizeWindow(display, w, new_x, new_y, new_width, new_height);

    // Update the window state
    update_window_geometry(w, new_x, new_y, new_width, new_height);
}

// Set focus to a window
void set_focus(Window w) {
    if (focused_window != None) {
        // Unfocus the previously focused window
        draw_window_titlebar(focused_window, 0);
    }

    focused_window = w;
    draw_window_titlebar(w, 1);
    XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);

    // Raise the window to the top
    XRaiseWindow(display, w);

    // Update the active window state
    active_window = w;
}

// Main event loop
void event_loop() {
    XEvent event;

    while (1) {
        XNextEvent(display, &event);

        switch (event.type) {
            case CreateNotify:
                // A new window was created
                handle_new_window(event.xcreatewindow.window);
                break;

            case DestroyNotify:
                // A window was destroyed
                handle_window_destroy(event.xdestroywindow.window);
                break;

            case ConfigureRequest:
                // A window wants to be reconfigured
                handle_configure_request(&event.xconfigurerequest);
                break;

            case MapRequest:
                // A window wants to be mapped (shown)
                handle_map_request(event.xmaprequest.window);
                break;

            case ButtonPress:
                // Mouse button was pressed
                handle_button_press(&event.xbutton);
                break;

            case ButtonRelease:
                // Mouse button was released
                handle_button_release(&event.xbutton);
                break;

            case MotionNotify:
                // Mouse was moved
                handle_motion_notify(&event.xmotion);
                break;

            case KeyPress:
                // A key was pressed
                handle_key_press(&event.xkey);
                break;

            case EnterNotify:
                // Mouse entered a window
                if (event.xcrossing.window != root) {
                    handle_enter_window(event.xcrossing.window);
                }
                break;

            case ClientMessage:
                // Client message (often used for EWMH/ICCCM compliance)
                handle_client_message(&event.xclient);
                break;
        }

        // Process any pending UI updates
        process_toolkit_updates();
    }
}

// Clean up resources before exiting
void cleanup() {
    // Free cursors
    XFreeCursor(display, cursor_normal);
    XFreeCursor(display, cursor_move);
    XFreeCursor(display, cursor_resize_nw);
    XFreeCursor(display, cursor_resize_ne);
    XFreeCursor(display, cursor_resize_sw);
    XFreeCursor(display, cursor_resize_se);
    XFreeCursor(display, cursor_resize_n);
    XFreeCursor(display, cursor_resize_e);
    XFreeCursor(display, cursor_resize_s);
    XFreeCursor(display, cursor_resize_w);

    // Save states
    save_all_window_states();

    // Close display
    XCloseDisplay(display);
}

// Main function
int main() {
    if (!init_window_manager()) {
        return 1;
    }

    // Run the main event loop
    event_loop();

    // Clean up resources
    cleanup();

    return 0;
}
