/*
 * toolkit.c - Lightweight UI toolkit implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "toolkit.h"
#include "themes.h"
#include "widgets.h"

// Global display and window references
static Display *display;
static Window root;
static int screen;
static Theme *current_theme;

// Resources
static GC default_gc = NULL;
static XFontStruct *default_font = NULL;
static Cursor text_cursor;

// Widget registry for event handling
#define MAX_WIDGETS 1024
static void *widgets[MAX_WIDGETS];
static int widget_count = 0;

// Initialize the toolkit
void init_toolkit(Display *dpy, Window root_win, int scr) {
    display = dpy;
    root = root_win;
    screen = scr;
    
    // Load default font
    default_font = XLoadQueryFont(display, "fixed");
    if (!default_font) {
        fprintf(stderr, "Failed to load default font, using server default\n");
    }
    
    // Create default GC
    default_gc = XCreateGC(display, root, 0, NULL);
    if (default_font) {
        XSetFont(display, default_gc, default_font->fid);
    }
    
    // Create text cursor
    text_cursor = XCreateFontCursor(display, XC_xterm);
    
    printf("UI toolkit initialized\n");
}

// Process pending toolkit updates
void process_toolkit_updates() {
    // Update any animations, timers, etc.
    
    // For now, just update the panel clock if it exists
    extern void update_panel_clock();
    update_panel_clock();
}

// Register a widget for event handling
static void register_widget(void *widget) {
    if (widget_count < MAX_WIDGETS) {
        widgets[widget_count++] = widget;
    } else {
        fprintf(stderr, "Maximum widget count reached\n");
    }
}

// Unregister a widget
static void unregister_widget(void *widget) {
    for (int i = 0; i < widget_count; i++) {
        if (widgets[i] == widget) {
            // Remove by shifting remaining widgets
            for (int j = i; j < widget_count - 1; j++) {
                widgets[j] = widgets[j + 1];
            }
            widget_count--;
            break;
        }
    }
}

// Find a widget by its window
static void *find_widget_by_window(Window win) {
    // This would need more sophisticated tracking in a real implementation
    // For now, we just do a simple linear search
    for (int i = 0; i < widget_count; i++) {
        // Check if this is a button
        Button *btn = (Button *)widgets[i];
        if (btn && btn->window == win) {
            return btn;
        }
        
        // Check other widget types as needed...
    }
    
    return NULL;
}

// Color utilities
unsigned long get_color(Display *dpy, const char *color_name) {
    XColor color;
    Colormap cmap = DefaultColormap(dpy, DefaultScreen(dpy));
    
    if (!XParseColor(dpy, cmap, color_name, &color)) {
        fprintf(stderr, "Failed to parse color: %s\n", color_name);
        return BlackPixel(dpy, DefaultScreen(dpy));
    }
    
    if (!XAllocColor(dpy, cmap, &color)) {
        fprintf(stderr, "Failed to allocate color: %s\n", color_name);
        return BlackPixel(dpy, DefaultScreen(dpy));
    }
    
    return color.pixel;
}

// Create a graphics context
GC create_gc(Display *dpy, Window win, unsigned long foreground, unsigned long background) {
    GC gc;
    XGCValues values;
    
    values.foreground = foreground;
    values.background = background;
    values.line_width = 1;
    values.line_style = LineSolid;
    
    gc = XCreateGC(dpy, win, GCForeground | GCBackground | GCLineWidth | GCLineStyle, &values);
    
    // Set font if available
    if (default_font) {
        XSetFont(dpy, gc, default_font->fid);
    }
    
    return gc;
}

// Free a graphics context
void free_gc(Display *dpy, GC gc) {
    XFreeGC(dpy, gc);
}

// Drawing primitives

void draw_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height) {
    XDrawRectangle(dpy, win, gc, x, y, width, height);
}

void draw_filled_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height) {
    XFillRectangle(dpy, win, gc, x, y, width, height);
}

void draw_rounded_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height, int radius) {
    // Draw the main lines
    XDrawLine(dpy, win, gc, x + radius, y, x + width - radius, y);
    XDrawLine(dpy, win, gc, x + width, y + radius, x + width, y + height - radius);
    XDrawLine(dpy, win, gc, x + radius, y + height, x + width - radius, y + height);
    XDrawLine(dpy, win, gc, x, y + radius, x, y + height - radius);
    
    // Draw the corners
    XDrawArc(dpy, win, gc, x, y, radius * 2, radius * 2, 90 * 64, 90 * 64);
    XDrawArc(dpy, win, gc, x + width - radius * 2, y, radius * 2, radius * 2, 0, 90 * 64);
    XDrawArc(dpy, win, gc, x, y + height - radius * 2, radius * 2, radius * 2, 180 * 64, 90 * 64);
    XDrawArc(dpy, win, gc, x + width - radius * 2, y + height - radius * 2, radius * 2, radius * 2, 270 * 64, 90 * 64);
}

void draw_filled_rounded_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height, int radius) {
    // Fill the main rectangle
    XFillRectangle(dpy, win, gc, x + radius, y, width - 2 * radius, height);
    XFillRectangle(dpy, win, gc, x, y + radius, width, height - 2 * radius);
    
    // Fill the corners
    XFillArc(dpy, win, gc, x, y, radius * 2, radius * 2, 90 * 64, 90 * 64);
    XFillArc(dpy, win, gc, x + width - radius * 2, y, radius * 2, radius * 2, 0, 90 * 64);
    XFillArc(dpy, win, gc, x, y + height - radius * 2, radius * 2, radius * 2, 180 * 64, 90 * 64);
    XFillArc(dpy, win, gc, x + width - radius * 2, y + height - radius * 2, radius * 2, radius * 2, 270 * 64, 90 * 64);
}

void draw_line(Display *dpy, Window win, GC gc, int x1, int y1, int x2, int y2) {
    XDrawLine(dpy, win, gc, x1, y1, x2, y2);
}

void draw_text(Display *dpy, Window win, GC gc, int x, int y, const char *text, int alignment) {
    if (!text) return;
    
    int width = 0;
    
    // Get text width if we need to align
    if (alignment != 0) {  // Not left-aligned
        if (default_font) {
            width = XTextWidth(default_font, text, strlen(text));
        } else {
            width = strlen(text) * 8;  // Rough estimate
        }
    }
    
    // Adjust x position based on alignment
    if (alignment == 1) {  // Center
        x -= width / 2;
    } else if (alignment == 2) {  // Right
        x -= width;
    }
    
    XDrawString(dpy, win, gc, x, y, text, strlen(text));
}

// Button implementation

Button *create_button(Window parent, int x, int y, int width, int height, 
                     const char *label, void (*click_handler)(void *), void *user_data) {
    Button *button = (Button *)malloc(sizeof(Button));
    if (!button) {
        fprintf(stderr, "Failed to allocate memory for button\n");
        return NULL;
    }
    
    // Initialize button properties
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->label = label ? strdup(label) : NULL;
    button->state = 0;  // Normal state
    button->click_handler = click_handler;
    button->user_data = user_data;
    
    // Create the button window
    button->window = XCreateSimpleWindow(
        display, parent,
        x, y, width, height,
        1, current_theme->border_color, current_theme->button_bg_color
    );
    
    // Select events
    XSelectInput(display, button->window, ExposureMask | ButtonPressMask | 
                              ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
    
    // Map the window
    XMapWindow(display, button->window);
    
    // Draw initial state
    GC gc = create_gc(display, button->window, current_theme->text_color, current_theme->button_bg_color);
    
    XClearWindow(display, button->window);
    if (button->label) {
        draw_text(display, button->window, gc, width / 2, height / 2 + 5, button->label, 1);
    }
    
    free_gc(display, gc);
    
    // Register the button
    register_widget(button);
    
    return button;
}

void button_set_label(Button *button, const char *label) {
    if (!button) return;
    
    // Update the label
    if (button->label) {
        free(button->label);
    }
    button->label = label ? strdup(label) : NULL;
    
    // Redraw the button
    GC gc = create_gc(display, button->window, current_theme->text_color, current_theme->button_bg_color);
    
    XClearWindow(display, button->window);
    if (button->label) {
        draw_text(display, button->window, gc, button->width / 2, button->height / 2 + 5, button->label, 1);
    }
    
    free_gc(display, gc);
}

void button_set_state(Button *button, int state) {
    if (!button) return;
    
    // Update the state
    button->state = state;
    
    // Set colors based on state
    unsigned long bg_color;
    switch (state) {
        case 1:  // Hover
            bg_color = current_theme->button_hover_bg_color;
            break;
        case 2:  // Pressed
            bg_color = current_theme->button_active_bg_color;
            break;
        default:  // Normal
            bg_color = current_theme->button_bg_color;
            break;
    }
    
    // Update the background
    XSetWindowBackground(display, button->window, bg_color);
    XClearWindow(display, button->window);
    
    // Redraw the label
    GC gc = create_gc(display, button->window, current_theme->text_color, bg_color);
    
    if (button->label) {
        draw_text(display, button->window, gc, button->width / 2, button->height / 2 + 5, button->label, 1);
    }
    
    free_gc(display, gc);
}

void button_destroy(Button *button) {
    if (!button) return;
    
    // Free resources
    if (button->label) {
        free(button->label);
    }
    
    // Destroy the window
    XDestroyWindow(display, button->window);
    
    // Unregister the button
    unregister_widget(button);
    
    // Free the button structure
    free(button);
}

// Event handling for widgets
int handle_widget_event(XEvent *event) {
    // Find the widget this event is for
    void *widget = find_widget_by_window(event->xany.window);
    if (!widget) {
        return 0;  // Not for our widgets
    }
    
    // Determine widget type and handle appropriately
    // For now, we just handle buttons
    Button *button = (Button *)widget;
    
    switch (event->type) {
        case Expose:
            // Redraw the button
            if (button) {
                unsigned long bg_color;
                switch (button->state) {
                    case 1:  // Hover
                        bg_color = current_theme->button_hover_bg_color;
                        break;
                    case 2:  // Pressed
                        bg_color = current_theme->button_active_bg_color;
                        break;
                    default:  // Normal
                        bg_color = current_theme->button_bg_color;
                        break;
                }
                
                GC gc = create_gc(display, button->window, current_theme->text_color, bg_color);
                
                if (button->label) {
                    draw_text(display, button->window, gc, 
                             button->width / 2, button->height / 2 + 5, 
                             button->label, 1);
                }
                
                free_gc(display, gc);
            }
            return 1;
            
        case ButtonPress:
            // Button press
            if (button && event->xbutton.button == Button1) {
                button_set_state(button, 2);
            }
            return 1;
            
        case ButtonRelease:
            // Button release
            if (button && event->xbutton.button == Button1) {
                // Change to hover state
                button_set_state(button, 1);
                
                // Call click handler
                if (button->click_handler) {
                    button->click_handler(button->user_data);
                }
            }
            return 1;
            
        case EnterNotify:
            // Mouse enters widget
            if (button) {
                button_set_state(button, 1);
            }
            return 1;
            
        case LeaveNotify:
            // Mouse leaves widget
            if (button) {
                button_set_state(button, 0);
            }
            return 1;
    }
    
    return 0;  // Event not handled
}

// Label implementation

Label *create_label(Window parent, int x, int y, int width, int height, 
                   const char *text, int alignment) {
    Label *label = (Label *)malloc(sizeof(Label));
    if (!label) {
        fprintf(stderr, "Failed to allocate memory for label\n");
        return NULL;
    }
    
    // Initialize label properties
    label->x = x;
    label->y = y;
    label->width = width;
    label->height = height;
    label->text = text ? strdup(text) : NULL;
    label->alignment = alignment;
    
    // Create the label window
    label->window = XCreateSimpleWindow(
        display, parent,
        x, y, width, height,
        0, current_theme->border_color, current_theme->bg_color
    );
    
    // Select events
    XSelectInput(display, label->window, ExposureMask);
    
    // Map the window
    XMapWindow(display, label->window);
    
    // Draw initial state
    GC gc = create_gc(display, label->window, current_theme->text_color, current_theme->bg_color);
    
    XClearWindow(display, label->window);
    if (label->text) {
        draw_text(display, label->window, gc, 
                 (alignment == 0) ? 5 : ((alignment == 1) ? width / 2 : width - 5),
                 height / 2 + 5, label->text, alignment);
    }
    
    free_gc(display, gc);
    
    // Register the label
    register_widget(label);
    
    return label;
}

void label_set_text(Label *label, const char *text) {
    if (!label) return;
    
    // Update the text
    if (label->text) {
        free(label->text);
    }
    label->text = text ? strdup(text) : NULL;
    
    // Redraw the label
    GC gc = create_gc(display, label->window, current_theme->text_color, current_theme->bg_color);
    
    XClearWindow(display, label->window);
    if (label->text) {
        draw_text(display, label->window, gc, 
                 (label->alignment == 0) ? 5 : ((label->alignment == 1) ? label->width / 2 : label->width - 5),
                 label->height / 2 + 5, label->text, label->alignment);
    }
    
    free_gc(display, gc);
}

void label_set_alignment(Label *label, int alignment) {
    if (!label) return;
    
    // Update the alignment
    label->alignment = alignment;
    
    // Redraw the label
    GC gc = create_gc(display, label->window, current_theme->text_color, current_theme->bg_color);
    
    XClearWindow(display, label->window);
    if (label->text) {
        draw_text(display, label->window, gc, 
                 (alignment == 0) ? 5 : ((alignment == 1) ? label->width / 2 : label->width - 5),
                 label->height / 2 + 5, label->text, alignment);
    }
    
    free_gc(display, gc);
}

void label_destroy(Label *label) {
    if (!label) return;
    
    // Free resources
    if (label->text) {
        free(label->text);
    }
    
    // Destroy the window
    XDestroyWindow(display, label->window);
    
    // Unregister the label
    unregister_widget(label);
    
    // Free the label structure
    free(label);
}

// Simple message dialog implementation
int show_message_dialog(Window parent, const char *title, const char *message, int buttons) {
    // Create a simple modal dialog
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);
    
    int width = 300;
    int height = 150;
    int x = (screen_width - width) / 2;
    int y = (screen_height - height) / 2;
    
    // Create the dialog window
    Window dialog = XCreateSimpleWindow(
        display, root,
        x, y, width, height,
        1, current_theme->border_color, current_theme->bg_color
    );
    
    // Set window properties
    XSetStandardProperties(display, dialog, title, title, None, NULL, 0, NULL);
    
    // Create dialog contents
    Label *msg_label = create_label(dialog, 10, 20, width - 20, 60, message, 1);
    
    // Create buttons based on button type
    Button *ok_button = NULL;
    Button *cancel_button = NULL;
    
    if (buttons == 0) {  // OK only
        ok_button = create_button(dialog, width / 2 - 40, height - 50, 80, 30, "OK", NULL, NULL);
    } else {  // OK and Cancel
        ok_button = create_button(dialog, width / 2 - 90, height - 50, 80, 30, "OK", NULL, NULL);
        cancel_button = create_button(dialog, width / 2 + 10, height - 50, 80, 30, "Cancel", NULL, NULL);
    }
    
    // Select events
    XSelectInput(display, dialog, ExposureMask | KeyPressMask);
    
    // Map the window
    XMapWindow(display, dialog);
    XRaiseWindow(display, dialog);
    
    // Grab the pointer and keyboard to make it modal
    XGrabPointer(display, dialog, True,
               ButtonPressMask | ButtonReleaseMask,
               GrabModeAsync, GrabModeAsync,
               None, None, CurrentTime);
    
    XGrabKeyboard(display, dialog, True,
                GrabModeAsync, GrabModeAsync,
                CurrentTime);
    
    // Event loop
    XEvent event;
    int result = 0;
    int done = 0;
    
    while (!done) {
        XNextEvent(display, &event);
        
        switch (event.type) {
            case Expose:
                // Draw title bar
                {
                    GC gc = create_gc(display, dialog, current_theme->text_color, current_theme->title_bg_color);
                    XFillRectangle(display, dialog, gc, 0, 0, width, 20);
                    draw_text(display, dialog, gc, 10, 15, title, 0);
                    free_gc(display, gc);
                }
                break;
                
            case ButtonPress:
                if (ok_button && event.xbutton.window == ok_button->window) {
                    result = 1;
                    done = 1;
                } else if (cancel_button && event.xbutton.window == cancel_button->window) {
                    result = 0;
                    done = 1;
                }
                break;
                
            case KeyPress:
                {
                    KeySym key = XLookupKeysym(&event.xkey, 0);
                    if (key == XK_Return || key == XK_KP_Enter) {
                        result = 1;
                        done = 1;
                    } else if (key == XK_Escape) {
                        result = 0;
                        done = 1;
                    }
                }
                break;
        }
    }
    
    // Release grabs
    XUngrabPointer(display, CurrentTime);
    XUngrabKeyboard(display, CurrentTime);
    
    // Clean up
    if (msg_label) label_destroy(msg_label);
    if (ok_button) button_destroy(ok_button);
    if (cancel_button) button_destroy(cancel_button);
    
    XDestroyWindow(display, dialog);
    
    return result;
}

// Error dialog wrapper
int show_error_dialog(Window parent, const char *title, const char *message) {
    return show_message_dialog(parent, title, message, 0);
}

// Confirm dialog wrapper
int show_confirm_dialog(Window parent, const char *title, const char *message) {
    return show_message_dialog(parent, title, message, 1);
}
