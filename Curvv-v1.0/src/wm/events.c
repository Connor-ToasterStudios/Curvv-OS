/*
 * events.c - Event handling implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "events.h"
#include "window.h"
#include "panel.h"
#include "menu.h"
#include "../state/state_manager.h"

// Global display and root window references from window_manager.c
extern Display *display;
extern Window root;
extern int screen;
extern Window active_window;
extern Window focused_window;

// Resize mode and tracking from window_manager.c
extern enum ResizeMode resize_mode;
extern int drag_start_x, drag_start_y;
extern int window_start_x, window_start_y;
extern int window_start_width, window_start_height;

// Cursor definitions from window_manager.c
extern Cursor cursor_normal;
extern Cursor cursor_move;
extern Cursor cursor_resize_nw;
extern Cursor cursor_resize_ne;
extern Cursor cursor_resize_sw;
extern Cursor cursor_resize_se;
extern Cursor cursor_resize_n;
extern Cursor cursor_resize_e;
extern Cursor cursor_resize_s;
extern Cursor cursor_resize_w;

// Root menu
static Menu *root_menu = NULL;

// X error handler
int error_handler(Display *display, XErrorEvent *event) {
    char error_text[256];
    XGetErrorText(display, event->error_code, error_text, sizeof(error_text));
    fprintf(stderr, "X Error: %s\n", error_text);
    return 0;  // Continue execution
}

// Set up event handlers
void setup_event_handlers() {
    // Create the root menu
    root_menu = create_root_menu();
}

// Handle window destruction
void handle_window_destroy(Window w) {
    printf("Window destroyed: %lu\n", w);
    
    // Remove the taskbar button
    remove_panel_taskbutton(w);
    
    // Remove window state
    remove_window_state(w);
}

// Handle configure request
void handle_configure_request(XConfigureRequestEvent *e) {
    XWindowChanges changes;
    
    // Copy the requested changes
    changes.x = e->x;
    changes.y = e->y;
    changes.width = e->width;
    changes.height = e->height;
    changes.border_width = e->border_width;
    changes.sibling = e->above;
    changes.stack_mode = e->detail;
    
    // Apply the changes
    XConfigureWindow(display, e->window, e->value_mask, &changes);
    
    // Update our state management if we're managing this window
    if (e->value_mask & (CWWidth | CWHeight | CWX | CWY)) {
        // Get current values for fields not in the request
        XWindowAttributes attr;
        if (XGetWindowAttributes(display, e->window, &attr)) {
            int x = (e->value_mask & CWX) ? e->x : attr.x;
            int y = (e->value_mask & CWY) ? e->y : attr.y;
            int width = (e->value_mask & CWWidth) ? e->width : attr.width;
            int height = (e->value_mask & CWHeight) ? e->height : attr.height;
            
            update_window_geometry(e->window, x, y, width, height);
        }
    }
}

// Handle map request
void handle_map_request(Window w) {
    // Get the window attributes
    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, w, &attr)) {
        fprintf(stderr, "Failed to get window attributes for map request\n");
        return;
    }
    
    // Skip windows we don't want to manage
    if (attr.override_redirect) {
        XMapWindow(display, w);
        return;
    }
    
    // Get the window name
    char *window_name = NULL;
    XTextProperty text_prop;
    if (XGetWMName(display, w, &text_prop) && text_prop.value) {
        window_name = (char *)text_prop.value;
    }
    
    printf("Mapping window: %lu (%s)\n", w, window_name ? window_name : "Unnamed");
    
    // Set up the window frame
    setup_window_frame(w, &attr);
    
    // Add a taskbar button
    add_panel_taskbutton(w, window_name);
    
    // Set focus to this window
    set_focus(w);
    
    // Free the window name
    if (text_prop.value) {
        XFree(text_prop.value);
    }
}

// Handle button press
void handle_button_press(XButtonEvent *e) {
    // Check if the event is for the root window
    if (e->window == root) {
        if (e->button == Button3) {  // Right click
            // Show the root menu
            if (root_menu) {
                show_menu(root_menu, e->x_root, e->y_root);
            }
        }
        return;
    }
    
    // Check if the click is on a menu
    if (handle_menu_event((XEvent *)e)) {
        return;
    }
    
    // Check if the click is on the panel
    if (handle_panel_event((XEvent *)e)) {
        return;
    }
    
    // Get the window frame part that was clicked
    FramePart part = get_frame_part(e->window, e->x, e->y);
    
    // Handle window specific buttons
    WMWindow *w = find_window(e->window);
    if (w) {
        if (e->window == w->close_button) {
            // Close button clicked
            close_window(w->window);
            return;
        } else if (e->window == w->max_button) {
            // Maximize button clicked
            maximize_window(w->window);
            return;
        } else if (e->window == w->min_button) {
            // Minimize button clicked
            minimize_window(w->window);
            return;
        } else if (e->window == w->titlebar) {
            // Titlebar clicked
            if (e->button == Button1) {
                // Left click - start moving the window
                XRaiseWindow(display, w->frame);
                
                // Set the cursor to indicate moving
                XDefineCursor(display, w->frame, cursor_move);
                
                // Start the drag operation
                resize_mode = MOVE;
                drag_start_x = e->x_root;
                drag_start_y = e->y_root;
                window_start_x = w->x;
                window_start_y = w->y;
                window_start_width = w->width;
                window_start_height = w->height;
                
                // Grab pointer to get motion events outside the window
                XGrabPointer(display, w->frame, False,
                           ButtonReleaseMask | PointerMotionMask,
                           GrabModeAsync, GrabModeAsync,
                           None, cursor_move, CurrentTime);
            } else if (e->button == Button3) {
                // Right click - show window menu
                Menu *win_menu = create_window_menu(w->window);
                if (win_menu) {
                    show_menu(win_menu, e->x_root, e->y_root);
                }
            }
            return;
        }
    }
    
    // Handle resize operations based on frame part
    if (w) {
        switch (part) {
            case FRAME_BORDER_N:
                resize_mode = RESIZE_N;
                XDefineCursor(display, w->frame, cursor_resize_n);
                break;
            case FRAME_BORDER_E:
                resize_mode = RESIZE_E;
                XDefineCursor(display, w->frame, cursor_resize_e);
                break;
            case FRAME_BORDER_S:
                resize_mode = RESIZE_S;
                XDefineCursor(display, w->frame, cursor_resize_s);
                break;
            case FRAME_BORDER_W:
                resize_mode = RESIZE_W;
                XDefineCursor(display, w->frame, cursor_resize_w);
                break;
            case FRAME_CORNER_NW:
                resize_mode = RESIZE_NW;
                XDefineCursor(display, w->frame, cursor_resize_nw);
                break;
            case FRAME_CORNER_NE:
                resize_mode = RESIZE_NE;
                XDefineCursor(display, w->frame, cursor_resize_ne);
                break;
            case FRAME_CORNER_SW:
                resize_mode = RESIZE_SW;
                XDefineCursor(display, w->frame, cursor_resize_sw);
                break;
            case FRAME_CORNER_SE:
                resize_mode = RESIZE_SE;
                XDefineCursor(display, w->frame, cursor_resize_se);
                break;
            default:
                resize_mode = RESIZE_NONE;
                break;
        }
        
        if (resize_mode != RESIZE_NONE) {
            // Start the resize operation
            XRaiseWindow(display, w->frame);
            
            drag_start_x = e->x_root;
            drag_start_y = e->y_root;
            window_start_x = w->x;
            window_start_y = w->y;
            window_start_width = w->width;
            window_start_height = w->height;
            
            // Grab pointer to get motion events outside the window
            Cursor cursor = cursor_normal;
            switch (resize_mode) {
                case RESIZE_N: cursor = cursor_resize_n; break;
                case RESIZE_E: cursor = cursor_resize_e; break;
                case RESIZE_S: cursor = cursor_resize_s; break;
                case RESIZE_W: cursor = cursor_resize_w; break;
                case RESIZE_NW: cursor = cursor_resize_nw; break;
                case RESIZE_NE: cursor = cursor_resize_ne; break;
                case RESIZE_SW: cursor = cursor_resize_sw; break;
                case RESIZE_SE: cursor = cursor_resize_se; break;
                default: break;
            }
            
            XGrabPointer(display, w->frame, False,
                       ButtonReleaseMask | PointerMotionMask,
                       GrabModeAsync, GrabModeAsync,
                       None, cursor, CurrentTime);
        }
    }
    
    // Focus the window
    if (e->window != root && e->window != None) {
        set_focus(e->window);
    }
}

// Handle button release
void handle_button_release(XButtonEvent *e) {
    // Check if the event is for a menu
    if (handle_menu_event((XEvent *)e)) {
        return;
    }
    
    // Check if the event is for the panel
    if (handle_panel_event((XEvent *)e)) {
        return;
    }
    
    // End any resize or move operation
    if (resize_mode != RESIZE_NONE) {
        resize_mode = RESIZE_NONE;
        
        // Reset cursor
        WMWindow *w = find_window(e->window);
        if (w) {
            XDefineCursor(display, w->frame, cursor_normal);
        }
        
        // Ungrab pointer
        XUngrabPointer(display, CurrentTime);
    }
}

// Handle pointer motion
void handle_motion_notify(XMotionEvent *e) {
    // Check if the event is for a menu
    if (handle_menu_event((XEvent *)e)) {
        return;
    }
    
    // Check if the event is for the panel
    if (handle_panel_event((XEvent *)e)) {
        return;
    }
    
    // Handle resize/move operations
    if (resize_mode != RESIZE_NONE) {
        WMWindow *w = find_window(e->window);
        if (w) {
            int dx = e->x_root - drag_start_x;
            int dy = e->y_root - drag_start_y;
            
            if (resize_mode == MOVE) {
                // Move the window
                int new_x = window_start_x + dx;
                int new_y = window_start_y + dy;
                
                // Move the window
                XMoveWindow(display, w->frame, new_x, new_y);
                
                // Update our state
                w->x = new_x;
                w->y = new_y;
            } else {
                // Resize operation
                int new_x = w->x;
                int new_y = w->y;
                int new_width = w->width;
                int new_height = w->height;
                
                // Calculate new size based on resize mode
                switch (resize_mode) {
                    case RESIZE_N:
                        new_y = window_start_y + dy;
                        new_height = window_start_height - dy;
                        break;
                    case RESIZE_E:
                        new_width = window_start_width + dx;
                        break;
                    case RESIZE_S:
                        new_height = window_start_height + dy;
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
                
                // Enforce minimum window size
                if (new_width < 50) new_width = 50;
                if (new_height < 50) new_height = 50;
                
                // Apply the new size
                XMoveResizeWindow(display, w->frame, new_x, new_y, 
                               new_width, new_height + w->titlebar_height);
                XResizeWindow(display, w->window, new_width, new_height);
                
                // Resize the titlebar
                XResizeWindow(display, w->titlebar, new_width, w->titlebar_height);
                
                // Reposition the buttons
                int button_x = new_width - 16 - 2;
                XMoveWindow(display, w->close_button, button_x, 2);
                button_x -= (16 + 2);
                XMoveWindow(display, w->max_button, button_x, 2);
                button_x -= (16 + 2);
                XMoveWindow(display, w->min_button, button_x, 2);
                
                // Update our state
                w->x = new_x;
                w->y = new_y;
                w->width = new_width;
                w->height = new_height;
            }
            
            // Save the window state
            save_window_state(w->window);
        }
    } else {
        // Update cursor based on position in window frame
        WMWindow *w = find_window(e->window);
        if (w) {
            FramePart part = get_frame_part(e->window, e->x, e->y);
            
            // Set the appropriate cursor
            switch (part) {
                case FRAME_BORDER_N:
                    XDefineCursor(display, w->frame, cursor_resize_n);
                    break;
                case FRAME_BORDER_E:
                    XDefineCursor(display, w->frame, cursor_resize_e);
                    break;
                case FRAME_BORDER_S:
                    XDefineCursor(display, w->frame, cursor_resize_s);
                    break;
                case FRAME_BORDER_W:
                    XDefineCursor(display, w->frame, cursor_resize_w);
                    break;
                case FRAME_CORNER_NW:
                    XDefineCursor(display, w->frame, cursor_resize_nw);
                    break;
                case FRAME_CORNER_NE:
                    XDefineCursor(display, w->frame, cursor_resize_ne);
                    break;
                case FRAME_CORNER_SW:
                    XDefineCursor(display, w->frame, cursor_resize_sw);
                    break;
                case FRAME_CORNER_SE:
                    XDefineCursor(display, w->frame, cursor_resize_se);
                    break;
                default:
                    XDefineCursor(display, w->frame, cursor_normal);
                    break;
            }
        }
    }
}

// Handle key press events
void handle_key_press(XKeyEvent *e) {
    KeySym key = XLookupKeysym(e, 0);
    
    // Alt+F4 to close window
    if ((e->state & Mod1Mask) && key == XK_F4) {
        if (focused_window != None) {
            close_window(focused_window);
        }
    }
    
    // Alt+Tab to cycle windows
    if ((e->state & Mod1Mask) && key == XK_Tab) {
        // Get the number of windows
        int count = get_window_count();
        if (count <= 1) {
            return;  // Nothing to cycle through
        }
        
        // Find the current window index
        int current_idx = -1;
        for (int i = 0; i < count; i++) {
            if (get_window_at_index(i) == focused_window) {
                current_idx = i;
                break;
            }
        }
        
        // Move to the next window
        int next_idx = (current_idx + 1) % count;
        Window next_win = get_window_at_index(next_idx);
        
        // Focus the next window
        if (next_win != None) {
            // If minimized, restore it
            WMWindow *w = find_window(next_win);
            if (w && w->is_minimized) {
                restore_window(next_win);
            }
            
            set_focus(next_win);
        }
    }
    
    // Alt+F to toggle fullscreen
    if ((e->state & Mod1Mask) && key == XK_f) {
        if (focused_window != None) {
            WMWindow *w = find_window(focused_window);
            if (w) {
                if (w->is_fullscreen) {
                    restore_window(focused_window);
                } else {
                    maximize_window(focused_window);
                }
            }
        }
    }
}

// Handle enter window events
void handle_enter_window(Window w) {
    // Skip the root window
    if (w == root) {
        return;
    }
    
    // Check if this is a managed window
    WMWindow *win = find_window(w);
    if (win) {
        // Focus-follows-mouse
        set_focus(win->window);
    }
}

// Handle client messages
void handle_client_message(XClientMessageEvent *e) {
    // Handle EWMH/ICCCM messages here
    // For now, just print a message
    printf("Received client message for window %lu\n", e->window);
}
