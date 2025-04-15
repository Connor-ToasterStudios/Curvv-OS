/*
 * panel.c - Panel/taskbar implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "panel.h"
#include "window.h"
#include "../ui/toolkit.h"
#include "../ui/themes.h"
#include "../state/state_manager.h"

#define MAX_PANEL_ITEMS 64
#define PANEL_HEIGHT 30
#define PANEL_ITEM_PADDING 5
#define TASKBUTTON_WIDTH 150
#define TASKBUTTON_HEIGHT 24
#define LAUNCHER_WIDTH 36
#define LAUNCHER_HEIGHT 24
#define CLOCK_WIDTH 100
#define WORKSPACE_BUTTON_WIDTH 24
#define SEPARATOR_WIDTH 10

// Global display and window references
static Display *display;
static Window root;
static int screen;
static Theme *current_theme;

// Panel data
static Window panel_window;
static PanelPosition panel_pos = PANEL_POSITION_BOTTOM;
static int panel_width;
static int panel_height = PANEL_HEIGHT;
static int panel_visible = 1;
static int panel_autohide = 0;
static int panel_item_count = 0;
static PanelItem panel_items[MAX_PANEL_ITEMS];
static int panel_next_x = 0;

// Panel clock timer
static time_t last_clock_update = 0;

// Forward declarations
static void position_panel();
static void draw_panel_item(PanelItem *item);
static void handle_panel_expose();
static int find_panel_item_at(int x, int y);
static void handle_panel_click(int button, int x, int y);
static void launch_command(const char *command);

// Initialize the panel
void init_panel(Display *dpy, Window root_win, int scr, Theme *theme) {
    display = dpy;
    root = root_win;
    screen = scr;
    current_theme = theme;
    
    // Get the screen width
    panel_width = DisplayWidth(display, screen);
    
    // Create the panel window
    panel_window = XCreateSimpleWindow(
        display, root,
        0, DisplayHeight(display, screen) - panel_height,  // Position at bottom by default
        panel_width, panel_height,
        0, current_theme->border_color, current_theme->panel_bg_color
    );
    
    // Select events for the panel
    XSelectInput(display, panel_window, ExposureMask | ButtonPressMask | 
                               ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
    
    // Map the panel window
    XMapWindow(display, panel_window);
    
    // Position the panel according to the default position
    position_panel();
    
    // Initialize panel items array
    panel_item_count = 0;
    panel_next_x = PANEL_ITEM_PADDING;
    
    printf("Panel initialized successfully\n");
}

// Set the panel position
void set_panel_position(PanelPosition position) {
    panel_pos = position;
    position_panel();
    draw_panel();
}

// Position the panel according to the current setting
static void position_panel() {
    int x = 0;
    int y = 0;
    
    switch (panel_pos) {
        case PANEL_POSITION_TOP:
            y = 0;
            break;
        case PANEL_POSITION_BOTTOM:
            y = DisplayHeight(display, screen) - panel_height;
            break;
        case PANEL_POSITION_LEFT:
            // Vertical panel
            panel_width = panel_height;  // Swap dimensions
            panel_height = DisplayHeight(display, screen);
            break;
        case PANEL_POSITION_RIGHT:
            // Vertical panel
            x = DisplayWidth(display, screen) - panel_height;
            panel_width = panel_height;  // Swap dimensions
            panel_height = DisplayHeight(display, screen);
            break;
    }
    
    // Move and resize the panel
    XMoveResizeWindow(display, panel_window, x, y, panel_width, panel_height);
}

// Add a launcher to the panel
void add_panel_launcher(const char *label, const char *command) {
    if (panel_item_count >= MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // Create the launcher button window
    Window button = XCreateSimpleWindow(
        display, panel_window,
        panel_next_x, (panel_height - LAUNCHER_HEIGHT) / 2,
        LAUNCHER_WIDTH, LAUNCHER_HEIGHT,
        1, current_theme->border_color, current_theme->button_bg_color
    );
    
    // Select events for the button
    XSelectInput(display, button, ExposureMask | ButtonPressMask | 
                               ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
    
    // Create the panel item
    PanelItem item;
    item.type = PANEL_ITEM_LAUNCHER;
    item.window = button;
    item.x = panel_next_x;
    item.y = (panel_height - LAUNCHER_HEIGHT) / 2;
    item.width = LAUNCHER_WIDTH;
    item.height = LAUNCHER_HEIGHT;
    item.label = strdup(label);
    item.associated_window = None;
    item.workspace = 0;
    
    // Set up the click handler to launch the command
    item.click_handler = NULL;  // We'll handle it in the main event handler
    
    // Store the command as user data
    XChangeProperty(display, button, XInternAtom(display, "AMOS_COMMAND", False),
                    XA_STRING, 8, PropModeReplace, (unsigned char *)command, strlen(command));
    
    // Add the item to our array
    panel_items[panel_item_count++] = item;
    
    // Update the next item position
    panel_next_x += LAUNCHER_WIDTH + PANEL_ITEM_PADDING;
    
    // Map the launcher
    XMapWindow(display, button);
    
    // Draw the launcher
    draw_panel_item(&item);
}

// Add a task button for a window
void add_panel_taskbutton(Window win, const char *label) {
    if (panel_item_count >= MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // Create the task button window
    Window button = XCreateSimpleWindow(
        display, panel_window,
        panel_next_x, (panel_height - TASKBUTTON_HEIGHT) / 2,
        TASKBUTTON_WIDTH, TASKBUTTON_HEIGHT,
        1, current_theme->border_color, current_theme->button_bg_color
    );
    
    // Select events for the button
    XSelectInput(display, button, ExposureMask | ButtonPressMask | 
                               ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
    
    // Create the panel item
    PanelItem item;
    item.type = PANEL_ITEM_TASKBUTTON;
    item.window = button;
    item.x = panel_next_x;
    item.y = (panel_height - TASKBUTTON_HEIGHT) / 2;
    item.width = TASKBUTTON_WIDTH;
    item.height = TASKBUTTON_HEIGHT;
    item.label = strdup(label ? label : "Window");
    item.associated_window = win;
    item.workspace = 0;
    item.click_handler = NULL;  // We'll handle it in the main event handler
    
    // Add the item to our array
    panel_items[panel_item_count++] = item;
    
    // Update the next item position
    panel_next_x += TASKBUTTON_WIDTH + PANEL_ITEM_PADDING;
    
    // Map the button
    XMapWindow(display, button);
    
    // Draw the button
    draw_panel_item(&item);
}

// Remove a task button when window is closed
void remove_panel_taskbutton(Window win) {
    int i;
    int found_index = -1;
    
    // Find the task button for this window
    for (i = 0; i < panel_item_count; i++) {
        if (panel_items[i].type == PANEL_ITEM_TASKBUTTON && 
            panel_items[i].associated_window == win) {
            found_index = i;
            break;
        }
    }
    
    if (found_index == -1) {
        return;  // Not found
    }
    
    // Destroy the button window
    XDestroyWindow(display, panel_items[found_index].window);
    
    // Free the label
    free(panel_items[found_index].label);
    
    // Remove the item from our array by shifting everything down
    for (i = found_index; i < panel_item_count - 1; i++) {
        panel_items[i] = panel_items[i + 1];
    }
    panel_item_count--;
    
    // Recalculate positions and redraw the panel
    panel_next_x = PANEL_ITEM_PADDING;
    for (i = 0; i < panel_item_count; i++) {
        panel_items[i].x = panel_next_x;
        XMoveWindow(display, panel_items[i].window, panel_next_x, panel_items[i].y);
        panel_next_x += panel_items[i].width + PANEL_ITEM_PADDING;
    }
    
    // Redraw the panel
    draw_panel();
}

// Update a task button label when window title changes
void update_panel_taskbutton(Window win, const char *label) {
    // Find the task button for this window
    for (int i = 0; i < panel_item_count; i++) {
        if (panel_items[i].type == PANEL_ITEM_TASKBUTTON && 
            panel_items[i].associated_window == win) {
            // Update the label
            free(panel_items[i].label);
            panel_items[i].label = strdup(label ? label : "Window");
            
            // Redraw the button
            draw_panel_item(&panel_items[i]);
            break;
        }
    }
}

// Highlight the task button for the active window
void highlight_panel_taskbutton(Window win) {
    // Reset all task buttons to normal
    for (int i = 0; i < panel_item_count; i++) {
        if (panel_items[i].type == PANEL_ITEM_TASKBUTTON) {
            XSetWindowBackground(display, panel_items[i].window, current_theme->button_bg_color);
            XClearWindow(display, panel_items[i].window);
            draw_panel_item(&panel_items[i]);
        }
    }
    
    // Highlight the active window's button
    for (int i = 0; i < panel_item_count; i++) {
        if (panel_items[i].type == PANEL_ITEM_TASKBUTTON && 
            panel_items[i].associated_window == win) {
            XSetWindowBackground(display, panel_items[i].window, current_theme->button_active_bg_color);
            XClearWindow(display, panel_items[i].window);
            draw_panel_item(&panel_items[i]);
            break;
        }
    }
}

// Add clock to the panel
void add_panel_clock() {
    if (panel_item_count >= MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // Create the clock window
    Window clock = XCreateSimpleWindow(
        display, panel_window,
        panel_next_x, (panel_height - TASKBUTTON_HEIGHT) / 2,
        CLOCK_WIDTH, TASKBUTTON_HEIGHT,
        1, current_theme->border_color, current_theme->panel_bg_color
    );
    
    // Select events for the clock
    XSelectInput(display, clock, ExposureMask);
    
    // Create the panel item
    PanelItem item;
    item.type = PANEL_ITEM_CLOCK;
    item.window = clock;
    item.x = panel_next_x;
    item.y = (panel_height - TASKBUTTON_HEIGHT) / 2;
    item.width = CLOCK_WIDTH;
    item.height = TASKBUTTON_HEIGHT;
    item.label = strdup("00:00:00");  // Will be updated
    item.associated_window = None;
    item.workspace = 0;
    item.click_handler = NULL;
    
    // Add the item to our array
    panel_items[panel_item_count++] = item;
    
    // Update the next item position
    panel_next_x += CLOCK_WIDTH + PANEL_ITEM_PADDING;
    
    // Map the clock
    XMapWindow(display, clock);
    
    // Initialize the clock
    update_panel_clock();
}

// Update the clock display
void update_panel_clock() {
    time_t now = time(NULL);
    
    // Only update if the time has changed
    if (now == last_clock_update) {
        return;
    }
    
    last_clock_update = now;
    
    // Format the time
    struct tm *tm_info = localtime(&now);
    char time_str[9];  // HH:MM:SS + null
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    
    // Find the clock item
    for (int i = 0; i < panel_item_count; i++) {
        if (panel_items[i].type == PANEL_ITEM_CLOCK) {
            // Update the label
            free(panel_items[i].label);
            panel_items[i].label = strdup(time_str);
            
            // Redraw the clock
            draw_panel_item(&panel_items[i]);
            break;
        }
    }
}

// Add system tray to the panel
void add_panel_system_tray() {
    if (panel_item_count >= MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // For now, just create a placeholder - system tray would need
    // to implement XEMBED protocol to be fully functional
    Window tray = XCreateSimpleWindow(
        display, panel_window,
        panel_next_x, (panel_height - TASKBUTTON_HEIGHT) / 2,
        TASKBUTTON_WIDTH, TASKBUTTON_HEIGHT,
        1, current_theme->border_color, current_theme->panel_bg_color
    );
    
    // Select events
    XSelectInput(display, tray, ExposureMask);
    
    // Create the panel item
    PanelItem item;
    item.type = PANEL_ITEM_SYSTEM_TRAY;
    item.window = tray;
    item.x = panel_next_x;
    item.y = (panel_height - TASKBUTTON_HEIGHT) / 2;
    item.width = TASKBUTTON_WIDTH;
    item.height = TASKBUTTON_HEIGHT;
    item.label = strdup("System Tray");
    item.associated_window = None;
    item.workspace = 0;
    item.click_handler = NULL;
    
    // Add the item to our array
    panel_items[panel_item_count++] = item;
    
    // Update the next item position
    panel_next_x += TASKBUTTON_WIDTH + PANEL_ITEM_PADDING;
    
    // Map the tray
    XMapWindow(display, tray);
    
    // Draw the tray
    draw_panel_item(&item);
}

// Add workspace switcher to the panel
void add_panel_workspace_switcher(int num_workspaces) {
    if (panel_item_count + num_workspaces > MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // Create a button for each workspace
    for (int i = 0; i < num_workspaces; i++) {
        // Create the workspace button
        Window button = XCreateSimpleWindow(
            display, panel_window,
            panel_next_x, (panel_height - TASKBUTTON_HEIGHT) / 2,
            WORKSPACE_BUTTON_WIDTH, TASKBUTTON_HEIGHT,
            1, current_theme->border_color, current_theme->button_bg_color
        );
        
        // Select events for the button
        XSelectInput(display, button, ExposureMask | ButtonPressMask | 
                                 ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
        
        // Create the panel item
        PanelItem item;
        item.type = PANEL_ITEM_WORKSPACE_SWITCHER;
        item.window = button;
        item.x = panel_next_x;
        item.y = (panel_height - TASKBUTTON_HEIGHT) / 2;
        item.width = WORKSPACE_BUTTON_WIDTH;
        item.height = TASKBUTTON_HEIGHT;
        
        // Create label with workspace number
        char workspace_label[3];
        snprintf(workspace_label, sizeof(workspace_label), "%d", i + 1);
        item.label = strdup(workspace_label);
        
        item.associated_window = None;
        item.workspace = i;
        item.click_handler = NULL;  // Handled in main event handler
        
        // Add the item to our array
        panel_items[panel_item_count++] = item;
        
        // Update the next item position
        panel_next_x += WORKSPACE_BUTTON_WIDTH + PANEL_ITEM_PADDING;
        
        // Map the button
        XMapWindow(display, button);
        
        // Draw the button
        draw_panel_item(&item);
    }
}

// Add separator to the panel
void add_panel_separator() {
    if (panel_item_count >= MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // Create the separator window
    Window separator = XCreateSimpleWindow(
        display, panel_window,
        panel_next_x, (panel_height - TASKBUTTON_HEIGHT) / 2,
        SEPARATOR_WIDTH, TASKBUTTON_HEIGHT,
        0, current_theme->border_color, current_theme->panel_bg_color
    );
    
    // Create the panel item
    PanelItem item;
    item.type = PANEL_ITEM_SEPARATOR;
    item.window = separator;
    item.x = panel_next_x;
    item.y = (panel_height - TASKBUTTON_HEIGHT) / 2;
    item.width = SEPARATOR_WIDTH;
    item.height = TASKBUTTON_HEIGHT;
    item.label = NULL;
    item.associated_window = None;
    item.workspace = 0;
    item.click_handler = NULL;
    
    // Add the item to our array
    panel_items[panel_item_count++] = item;
    
    // Update the next item position
    panel_next_x += SEPARATOR_WIDTH + PANEL_ITEM_PADDING;
    
    // Map the separator
    XMapWindow(display, separator);
    
    // Draw the separator
    draw_panel_item(&item);
}

// Add custom item to the panel
void add_panel_custom(const char *label, void (*click_handler)(int button, int x, int y)) {
    if (panel_item_count >= MAX_PANEL_ITEMS) {
        fprintf(stderr, "Maximum panel items reached\n");
        return;
    }
    
    // Determine width based on text length
    int width = strlen(label) * 8 + 10;  // Rough estimate
    
    // Create the custom item window
    Window custom = XCreateSimpleWindow(
        display, panel_window,
        panel_next_x, (panel_height - TASKBUTTON_HEIGHT) / 2,
        width, TASKBUTTON_HEIGHT,
        1, current_theme->border_color, current_theme->button_bg_color
    );
    
    // Select events for the custom item
    XSelectInput(display, custom, ExposureMask | ButtonPressMask | 
                              ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
    
    // Create the panel item
    PanelItem item;
    item.type = PANEL_ITEM_CUSTOM;
    item.window = custom;
    item.x = panel_next_x;
    item.y = (panel_height - TASKBUTTON_HEIGHT) / 2;
    item.width = width;
    item.height = TASKBUTTON_HEIGHT;
    item.label = strdup(label);
    item.associated_window = None;
    item.workspace = 0;
    item.click_handler = click_handler;
    
    // Add the item to our array
    panel_items[panel_item_count++] = item;
    
    // Update the next item position
    panel_next_x += width + PANEL_ITEM_PADDING;
    
    // Map the custom item
    XMapWindow(display, custom);
    
    // Draw the custom item
    draw_panel_item(&item);
}

// Draw a single panel item
static void draw_panel_item(PanelItem *item) {
    if (!item || !item->window) return;
    
    GC gc = XCreateGC(display, item->window, 0, NULL);
    
    // Set colors based on item type
    switch (item->type) {
        case PANEL_ITEM_SEPARATOR:
            // Draw a vertical line
            XSetForeground(display, gc, current_theme->border_color);
            XDrawLine(display, item->window, gc,
                     item->width / 2, 2,
                     item->width / 2, item->height - 2);
            break;
            
        case PANEL_ITEM_CLOCK:
            // Draw the time
            XSetForeground(display, gc, current_theme->text_color);
            if (item->label) {
                XDrawString(display, item->window, gc,
                           5, item->height / 2 + 5,
                           item->label, strlen(item->label));
            }
            break;
            
        case PANEL_ITEM_WORKSPACE_SWITCHER:
            // Draw the workspace number
            XSetForeground(display, gc, current_theme->text_color);
            if (item->label) {
                XDrawString(display, item->window, gc,
                           item->width / 2 - 4, item->height / 2 + 5,
                           item->label, strlen(item->label));
            }
            break;
            
        default:
            // Draw the label for other items
            XSetForeground(display, gc, current_theme->text_color);
            if (item->label) {
                XDrawString(display, item->window, gc,
                           5, item->height / 2 + 5,
                           item->label, strlen(item->label));
            }
            break;
    }
    
    XFreeGC(display, gc);
}

// Draw the entire panel
void draw_panel() {
    // Clear the panel background
    XSetWindowBackground(display, panel_window, current_theme->panel_bg_color);
    XClearWindow(display, panel_window);
    
    // Draw each panel item
    for (int i = 0; i < panel_item_count; i++) {
        draw_panel_item(&panel_items[i]);
    }
}

// Handle panel expose event
static void handle_panel_expose() {
    draw_panel();
}

// Find the panel item at a given position
static int find_panel_item_at(int x, int y) {
    for (int i = 0; i < panel_item_count; i++) {
        if (x >= panel_items[i].x && 
            x < panel_items[i].x + panel_items[i].width &&
            y >= panel_items[i].y &&
            y < panel_items[i].y + panel_items[i].height) {
            return i;
        }
    }
    return -1;
}

// Handle a click on the panel
static void handle_panel_click(int button, int x, int y) {
    int item_idx = find_panel_item_at(x, y);
    if (item_idx == -1) return;
    
    PanelItem *item = &panel_items[item_idx];
    
    // Handle the click based on item type
    switch (item->type) {
        case PANEL_ITEM_LAUNCHER:
            {
                // Get the command from the window property
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char *data = NULL;
                
                if (XGetWindowProperty(display, item->window, 
                                    XInternAtom(display, "AMOS_COMMAND", False),
                                    0, 1024, False, XA_STRING,
                                    &actual_type, &actual_format, &nitems, &bytes_after, 
                                    &data) == Success && data) {
                    // Launch the command
                    launch_command((char *)data);
                    XFree(data);
                }
            }
            break;
            
        case PANEL_ITEM_TASKBUTTON:
            // Activate the window
            if (item->associated_window != None) {
                // If minimized, restore it
                WMWindow *w = find_window(item->associated_window);
                if (w && w->is_minimized) {
                    restore_window(item->associated_window);
                }
                
                // Focus the window
                focus_window(item->associated_window);
            }
            break;
            
        case PANEL_ITEM_WORKSPACE_SWITCHER:
            // Switch to the workspace
            // In a real implementation, this would call switch_to_workspace()
            printf("Switching to workspace %d\n", item->workspace + 1);
            break;
            
        case PANEL_ITEM_CUSTOM:
            // Call the custom click handler if available
            if (item->click_handler) {
                item->click_handler(button, x - item->x, y - item->y);
            }
            break;
            
        default:
            break;
    }
}

// Launch a command
static void launch_command(const char *command) {
    if (!command) return;
    
    // Fork a child process to execute the command
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        
        // Close X display connection in child
        XCloseDisplay(display);
        
        // Execute the command
        execl("/bin/sh", "/bin/sh", "-c", command, NULL);
        
        // If execl fails
        fprintf(stderr, "Failed to execute command: %s\n", command);
        exit(1);
    } else if (pid < 0) {
        // Fork failed
        fprintf(stderr, "Failed to fork process for command: %s\n", command);
    }
}

// Process panel events
int handle_panel_event(XEvent *event) {
    // Check if the event is for the panel or its items
    if (event->xany.window != panel_window) {
        for (int i = 0; i < panel_item_count; i++) {
            if (event->xany.window == panel_items[i].window) {
                // This event is for a panel item
                switch (event->type) {
                    case Expose:
                        draw_panel_item(&panel_items[i]);
                        return 1;
                        
                    case ButtonPress:
                        handle_panel_click(event->xbutton.button, 
                                         panel_items[i].x + event->xbutton.x, 
                                         panel_items[i].y + event->xbutton.y);
                        return 1;
                        
                    case EnterNotify:
                        // Highlight on hover
                        if (panel_items[i].type != PANEL_ITEM_CLOCK &&
                            panel_items[i].type != PANEL_ITEM_SYSTEM_TRAY &&
                            panel_items[i].type != PANEL_ITEM_SEPARATOR) {
                            XSetWindowBackground(display, panel_items[i].window, 
                                              current_theme->button_hover_bg_color);
                            XClearWindow(display, panel_items[i].window);
                            draw_panel_item(&panel_items[i]);
                        }
                        return 1;
                        
                    case LeaveNotify:
                        // Restore normal background
                        if (panel_items[i].type != PANEL_ITEM_CLOCK &&
                            panel_items[i].type != PANEL_ITEM_SYSTEM_TRAY &&
                            panel_items[i].type != PANEL_ITEM_SEPARATOR) {
                            XSetWindowBackground(display, panel_items[i].window, 
                                              current_theme->button_bg_color);
                            XClearWindow(display, panel_items[i].window);
                            draw_panel_item(&panel_items[i]);
                        }
                        return 1;
                }
            }
        }
        return 0;  // Not our event
    }
    
    // Handle events for the main panel window
    switch (event->type) {
        case Expose:
            handle_panel_expose();
            return 1;
            
        case ButtonPress:
            handle_panel_click(event->xbutton.button, event->xbutton.x, event->xbutton.y);
            return 1;
            
        case EnterNotify:
            // Show panel if autohide is enabled
            if (panel_autohide && !panel_visible) {
                show_panel();
            }
            return 1;
            
        case LeaveNotify:
            // Hide panel if autohide is enabled
            if (panel_autohide && panel_visible) {
                hide_panel();
            }
            return 1;
    }
    
    return 0;  // Not our event
}

// Show the panel
void show_panel() {
    if (!panel_visible) {
        XMapWindow(display, panel_window);
        panel_visible = 1;
    }
}

// Hide the panel
void hide_panel() {
    if (panel_visible) {
        XUnmapWindow(display, panel_window);
        panel_visible = 0;
    }
}

// Set panel autohide mode
void set_panel_autohide(int enable) {
    panel_autohide = enable;
    
    if (enable) {
        // Initially hide the panel
        hide_panel();
    } else {
        // Make sure the panel is visible
        show_panel();
    }
}
