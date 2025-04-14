/*
 * widgets.c - Widget implementations for the desktop environment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "widgets.h"
#include "toolkit.h"
#include "themes.h"

// Global display and window references
extern Display *display;
extern Window root;
extern int screen;
extern Theme current_theme;

#define MAX_DESKTOP_ICONS 64
#define ICON_WIDTH 64
#define ICON_HEIGHT 64
#define ICON_LABEL_HEIGHT 20

// Static variables
static DesktopIcon *desktop_icons[MAX_DESKTOP_ICONS];
static int desktop_icon_count = 0;

// Forward declarations
static void desktop_icon_click_handler(XEvent *event, DesktopIcon *icon);
static void launch_command(const char *command);

// Initialize widgets
void init_widgets() {
    // Initialize desktop icons
    for (int i = 0; i < MAX_DESKTOP_ICONS; i++) {
        desktop_icons[i] = NULL;
    }
    desktop_icon_count = 0;
    
    printf("Widgets initialized\n");
}

// Desktop icon implementation

DesktopIcon *create_desktop_icon(int x, int y, const char *label, 
                               const char *icon_path, const char *command) {
    if (desktop_icon_count >= MAX_DESKTOP_ICONS) {
        fprintf(stderr, "Maximum desktop icon count reached\n");
        return NULL;
    }
    
    DesktopIcon *icon = (DesktopIcon *)malloc(sizeof(DesktopIcon));
    if (!icon) {
        fprintf(stderr, "Failed to allocate memory for desktop icon\n");
        return NULL;
    }
    
    // Initialize icon properties
    icon->x = x;
    icon->y = y;
    icon->width = ICON_WIDTH;
    icon->height = ICON_HEIGHT + ICON_LABEL_HEIGHT;
    icon->label = label ? strdup(label) : NULL;
    icon->icon_path = icon_path ? strdup(icon_path) : NULL;
    icon->command = command ? strdup(command) : NULL;
    
    // Create the icon window
    icon->window = XCreateSimpleWindow(
        display, root,
        x, y, ICON_WIDTH, ICON_HEIGHT + ICON_LABEL_HEIGHT,
        0, 0, 0
    );
    
    // Make window transparent to mouse events except button presses
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;
    
    XChangeWindowAttributes(display, icon->window, CWOverrideRedirect | CWEventMask, &attrs);
    
    // Map the window
    XMapWindow(display, icon->window);
    
    // Draw the icon
    draw_desktop_icon(icon);
    
    // Add to our tracking array
    desktop_icons[desktop_icon_count++] = icon;
    
    return icon;
}

void destroy_desktop_icon(DesktopIcon *icon) {
    if (!icon) return;
    
    // Free resources
    if (icon->label) free(icon->label);
    if (icon->icon_path) free(icon->icon_path);
    if (icon->command) free(icon->command);
    
    // Destroy the window
    XDestroyWindow(display, icon->window);
    
    // Remove from our tracking array
    for (int i = 0; i < desktop_icon_count; i++) {
        if (desktop_icons[i] == icon) {
            // Shift all icons down
            for (int j = i; j < desktop_icon_count - 1; j++) {
                desktop_icons[j] = desktop_icons[j + 1];
            }
            desktop_icon_count--;
            break;
        }
    }
    
    // Free the structure
    free(icon);
}

void draw_desktop_icon(DesktopIcon *icon) {
    if (!icon) return;
    
    // Clear the window
    XClearWindow(display, icon->window);
    
    // Create a GC for drawing
    GC gc = create_gc(display, icon->window, current_theme.text_color, 0);
    
    // Draw a folder icon (since we don't load images)
    XSetForeground(display, gc, current_theme.desktop_icon_color);
    
    // Draw a simple folder icon
    XFillRectangle(display, icon->window, gc, 10, 10, ICON_WIDTH - 20, ICON_HEIGHT - 20);
    XSetForeground(display, gc, current_theme.border_color);
    XDrawRectangle(display, icon->window, gc, 10, 10, ICON_WIDTH - 20, ICON_HEIGHT - 20);
    
    // Draw the label with a background
    if (icon->label) {
        int text_width = strlen(icon->label) * 6;  // Approximate
        int text_x = (ICON_WIDTH - text_width) / 2;
        if (text_x < 2) text_x = 2;
        
        XSetForeground(display, gc, current_theme.desktop_icon_label_bg);
        XFillRectangle(display, icon->window, gc, 0, ICON_HEIGHT, ICON_WIDTH, ICON_LABEL_HEIGHT);
        
        XSetForeground(display, gc, current_theme.desktop_icon_label_fg);
        XDrawString(display, icon->window, gc, text_x, ICON_HEIGHT + 15, 
                   icon->label, strlen(icon->label));
    }
    
    // Free the GC
    free_gc(display, gc);
}

static void desktop_icon_click_handler(XEvent *event, DesktopIcon *icon) {
    if (!icon || !icon->command) return;
    
    if (event->type == ButtonRelease && event->xbutton.button == Button1) {
        // Launch the command
        launch_command(icon->command);
    }
}

// Helper function to launch commands
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

// Taskbar button implementation

TaskbarButton *create_taskbar_button(int x, int y, int width, int height, 
                                   const char *label, Window app_window) {
    TaskbarButton *button = (TaskbarButton *)malloc(sizeof(TaskbarButton));
    if (!button) {
        fprintf(stderr, "Failed to allocate memory for taskbar button\n");
        return NULL;
    }
    
    // Initialize button properties
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->label = label ? strdup(label) : NULL;
    button->app_window = app_window;
    button->is_active = 0;
    
    // Create the button window
    button->window = XCreateSimpleWindow(
        display, root,
        x, y, width, height,
        1, current_theme.border_color, current_theme.taskbar_button_bg
    );
    
    // Select events
    XSelectInput(display, button->window, ExposureMask | ButtonPressMask | 
                              ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);
    
    // Map the window
    XMapWindow(display, button->window);
    
    // Draw the button
    draw_taskbar_button(button);
    
    return button;
}

void taskbar_button_set_active(TaskbarButton *button, int active) {
    if (!button) return;
    
    button->is_active = active;
    draw_taskbar_button(button);
}

void draw_taskbar_button(TaskbarButton *button) {
    if (!button) return;
    
    // Choose background color based on active state
    unsigned long bg_color = button->is_active ? 
                             current_theme.taskbar_button_active_bg : 
                             current_theme.taskbar_button_bg;
    
    // Set the background color
    XSetWindowBackground(display, button->window, bg_color);
    XClearWindow(display, button->window);
    
    // Create a GC for drawing
    GC gc = create_gc(display, button->window, current_theme.taskbar_button_fg, bg_color);
    
    // Draw the label
    if (button->label) {
        // Truncate label if it's too long
        int max_chars = button->width / 8;  // Approximate
        char truncated[256];
        
        if (strlen(button->label) > max_chars) {
            strncpy(truncated, button->label, max_chars - 3);
            truncated[max_chars - 3] = '.';
            truncated[max_chars - 2] = '.';
            truncated[max_chars - 1] = '.';
            truncated[max_chars] = '\0';
        } else {
            strcpy(truncated, button->label);
        }
        
        XDrawString(display, button->window, gc, 5, button->height - 5, 
                   truncated, strlen(truncated));
    }
    
    // Free the GC
    free_gc(display, gc);
}

void destroy_taskbar_button(TaskbarButton *button) {
    if (!button) return;
    
    // Free resources
    if (button->label) free(button->label);
    
    // Destroy the window
    XDestroyWindow(display, button->window);
    
    // Free the structure
    free(button);
}

// Clock widget implementation

Clock *create_clock(int x, int y, int width, int height, int format, int show_seconds) {
    Clock *clock = (Clock *)malloc(sizeof(Clock));
    if (!clock) {
        fprintf(stderr, "Failed to allocate memory for clock\n");
        return NULL;
    }
    
    // Initialize clock properties
    clock->x = x;
    clock->y = y;
    clock->width = width;
    clock->height = height;
    clock->format = format;
    clock->show_seconds = show_seconds;
    
    // Create the clock window
    clock->window = XCreateSimpleWindow(
        display, root,
        x, y, width, height,
        1, current_theme.border_color, current_theme.panel_bg_color
    );
    
    // Select events
    XSelectInput(display, clock->window, ExposureMask);
    
    // Map the window
    XMapWindow(display, clock->window);
    
    // Initial update
    update_clock(clock);
    
    return clock;
}

void update_clock(Clock *clock) {
    if (!clock) return;
    
    time_t t;
    struct tm *tm_info;
    char buffer[64];
    
    time(&t);
    tm_info = localtime(&t);
    
    // Format the time according to settings
    if (clock->format == 0) {
        // 24-hour format
        if (clock->show_seconds) {
            strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
        } else {
            strftime(buffer, sizeof(buffer), "%H:%M", tm_info);
        }
    } else if (clock->format == 1) {
        // 12-hour format
        if (clock->show_seconds) {
            strftime(buffer, sizeof(buffer), "%I:%M:%S %p", tm_info);
        } else {
            strftime(buffer, sizeof(buffer), "%I:%M %p", tm_info);
        }
    } else {
        // Date + time
        if (clock->show_seconds) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        } else {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm_info);
        }
    }
    
    // Clear the window
    XClearWindow(display, clock->window);
    
    // Create a GC for drawing
    GC gc = create_gc(display, clock->window, current_theme.text_color, current_theme.panel_bg_color);
    
    // Draw the time
    XDrawString(display, clock->window, gc, 5, clock->height - 5, 
               buffer, strlen(buffer));
    
    // Free the GC
    free_gc(display, gc);
}

void destroy_clock(Clock *clock) {
    if (!clock) return;
    
    // Destroy the window
    XDestroyWindow(display, clock->window);
    
    // Free the structure
    free(clock);
}

// Menu implementation

Menu *create_menu() {
    Menu *menu = (Menu *)malloc(sizeof(Menu));
    if (!menu) {
        fprintf(stderr, "Failed to allocate memory for menu\n");
        return NULL;
    }
    
    // Initialize menu properties
    menu->x = 0;
    menu->y = 0;
    menu->width = 200;
    menu->height = 0;
    menu->items = (MenuItem **)malloc(sizeof(MenuItem *) * 32);  // Max 32 items
    menu->item_count = 0;
    menu->selected_item = -1;
    menu->parent = NULL;
    menu->window = None;
    
    if (!menu->items) {
        fprintf(stderr, "Failed to allocate memory for menu items\n");
        free(menu);
        return NULL;
    }
    
    return menu;
}

void add_menu_item(Menu *menu, const char *label, const char *command, 
                  void (*callback)(MenuItem *)) {
    if (!menu || menu->item_count >= 32) return;
    
    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (!item) {
        fprintf(stderr, "Failed to allocate memory for menu item\n");
        return;
    }
    
    // Initialize item
    item->type = 0;  // Normal
    item->label = label ? strdup(label) : NULL;
    item->command = command ? strdup(command) : NULL;
    item->submenu = NULL;
    item->enabled = 1;
    item->callback = callback;
    
    // Add to menu
    menu->items[menu->item_count++] = item;
    
    // Update menu height
    menu->height += 20;  // Each item is 20 pixels high
}

void add_submenu(Menu *menu, const char *label, Menu *submenu) {
    if (!menu || !submenu || menu->item_count >= 32) return;
    
    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (!item) {
        fprintf(stderr, "Failed to allocate memory for menu item\n");
        return;
    }
    
    // Initialize item
    item->type = 2;  // Submenu
    item->label = label ? strdup(label) : NULL;
    item->command = NULL;
    item->submenu = submenu;
    item->enabled = 1;
    item->callback = NULL;
    
    // Set parent
    submenu->parent = menu;
    
    // Add to menu
    menu->items[menu->item_count++] = item;
    
    // Update menu height
    menu->height += 20;  // Each item is 20 pixels high
}

void add_menu_separator(Menu *menu) {
    if (!menu || menu->item_count >= 32) return;
    
    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (!item) {
        fprintf(stderr, "Failed to allocate memory for menu separator\n");
        return;
    }
    
    // Initialize item
    item->type = 1;  // Separator
    item->label = NULL;
    item->command = NULL;
    item->submenu = NULL;
    item->enabled = 0;
    item->callback = NULL;
    
    // Add to menu
    menu->items[menu->item_count++] = item;
    
    // Update menu height
    menu->height += 5;  // Separators are 5 pixels high
}

void show_menu(Menu *menu, int x, int y) {
    if (!menu) return;
    
    // Create the menu window if needed
    if (menu->window == None) {
        menu->window = XCreateSimpleWindow(
            display, root,
            x, y, menu->width, menu->height,
            1, current_theme.border_color, current_theme.menu_bg_color
        );
        
        // Select events
        XSelectInput(display, menu->window, ExposureMask | ButtonPressMask | 
                                 ButtonReleaseMask | PointerMotionMask | LeaveWindowMask);
    }
    
    // Position the menu
    menu->x = x;
    menu->y = y;
    XMoveWindow(display, menu->window, x, y);
    
    // Ensure menu is on-screen
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);
    
    if (x + menu->width > screen_width) {
        menu->x = screen_width - menu->width;
        XMoveWindow(display, menu->window, menu->x, menu->y);
    }
    
    if (y + menu->height > screen_height) {
        menu->y = screen_height - menu->height;
        XMoveWindow(display, menu->window, menu->x, menu->y);
    }
    
    // Show the menu
    XMapRaised(display, menu->window);
    
    // Draw the menu
    GC gc = create_gc(display, menu->window, current_theme.menu_fg_color, current_theme.menu_bg_color);
    
    int y_pos = 0;
    for (int i = 0; i < menu->item_count; i++) {
        MenuItem *item = menu->items[i];
        
        if (item->type == 1) {
            // Separator
            XSetForeground(display, gc, current_theme.border_color);
            XDrawLine(display, menu->window, gc, 0, y_pos + 2, menu->width, y_pos + 2);
            y_pos += 5;
        } else {
            // Normal item or submenu
            XSetForeground(display, gc, current_theme.menu_bg_color);
            XFillRectangle(display, menu->window, gc, 0, y_pos, menu->width, 20);
            
            if (item->enabled) {
                XSetForeground(display, gc, current_theme.menu_fg_color);
            } else {
                XSetForeground(display, gc, current_theme.menu_disabled_fg_color);
            }
            
            if (item->label) {
                XDrawString(display, menu->window, gc, 5, y_pos + 15, 
                           item->label, strlen(item->label));
            }
            
            if (item->type == 2) {
                // Draw submenu indicator
                XSetForeground(display, gc, current_theme.menu_fg_color);
                XDrawString(display, menu->window, gc, menu->width - 10, y_pos + 15, ">", 1);
            }
            
            y_pos += 20;
        }
    }
    
    free_gc(display, gc);
}

void hide_menu(Menu *menu) {
    if (!menu || menu->window == None) return;
    
    // Hide any visible submenus first
    for (int i = 0; i < menu->item_count; i++) {
        if (menu->items[i]->type == 2 && menu->items[i]->submenu) {
            hide_menu(menu->items[i]->submenu);
        }
    }
    
    // Hide this menu
    XUnmapWindow(display, menu->window);
}

int handle_menu_event(XEvent *event) {
    // In a real implementation, we would track all open menus
    // and find which one this event is for.
    // For this example, this is just a placeholder.
    return 0;
}

// Workspace switcher implementation

WorkspaceSwitcher *create_workspace_switcher(int x, int y, int width, int height, int count) {
    WorkspaceSwitcher *switcher = (WorkspaceSwitcher *)malloc(sizeof(WorkspaceSwitcher));
    if (!switcher) {
        fprintf(stderr, "Failed to allocate memory for workspace switcher\n");
        return NULL;
    }
    
    // Initialize switcher properties
    switcher->x = x;
    switcher->y = y;
    switcher->width = width;
    switcher->height = height;
    switcher->workspace_count = count;
    switcher->current_workspace = 0;
    
    // Create the switcher window
    switcher->window = XCreateSimpleWindow(
        display, root,
        x, y, width, height,
        1, current_theme.border_color, current_theme.panel_bg_color
    );
    
    // Select events
    XSelectInput(display, switcher->window, ExposureMask | ButtonPressMask | 
                              ButtonReleaseMask);
    
    // Map the window
    XMapWindow(display, switcher->window);
    
    // Draw the switcher
    GC gc = create_gc(display, switcher->window, current_theme.text_color, current_theme.panel_bg_color);
    
    int cell_width = width / count;
    for (int i = 0; i < count; i++) {
        if (i == switcher->current_workspace) {
            XSetForeground(display, gc, current_theme.workspace_active_color);
            XFillRectangle(display, switcher->window, gc, i * cell_width, 0, cell_width, height);
        }
        
        XSetForeground(display, gc, current_theme.border_color);
        XDrawRectangle(display, switcher->window, gc, i * cell_width, 0, cell_width, height);
        
        char num[3];
        snprintf(num, sizeof(num), "%d", i + 1);
        
        XSetForeground(display, gc, current_theme.text_color);
        XDrawString(display, switcher->window, gc, i * cell_width + (cell_width / 2) - 3, 
                   height / 2 + 5, num, strlen(num));
    }
    
    free_gc(display, gc);
    
    return switcher;
}

void workspace_switcher_set_current(WorkspaceSwitcher *switcher, int workspace) {
    if (!switcher || workspace < 0 || workspace >= switcher->workspace_count) return;
    
    switcher->current_workspace = workspace;
    
    // Redraw the switcher
    GC gc = create_gc(display, switcher->window, current_theme.text_color, current_theme.panel_bg_color);
    
    XClearWindow(display, switcher->window);
    
    int cell_width = switcher->width / switcher->workspace_count;
    for (int i = 0; i < switcher->workspace_count; i++) {
        if (i == switcher->current_workspace) {
            XSetForeground(display, gc, current_theme.workspace_active_color);
            XFillRectangle(display, switcher->window, gc, i * cell_width, 0, cell_width, switcher->height);
        }
        
        XSetForeground(display, gc, current_theme.border_color);
        XDrawRectangle(display, switcher->window, gc, i * cell_width, 0, cell_width, switcher->height);
        
        char num[3];
        snprintf(num, sizeof(num), "%d", i + 1);
        
        XSetForeground(display, gc, current_theme.text_color);
        XDrawString(display, switcher->window, gc, i * cell_width + (cell_width / 2) - 3, 
                   switcher->height / 2 + 5, num, strlen(num));
    }
    
    free_gc(display, gc);
}

void destroy_workspace_switcher(WorkspaceSwitcher *switcher) {
    if (!switcher) return;
    
    // Destroy the window
    XDestroyWindow(display, switcher->window);
    
    // Free the structure
    free(switcher);
}
