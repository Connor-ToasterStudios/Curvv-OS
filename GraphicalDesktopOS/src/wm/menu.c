/*
 * menu.c - Menu system implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "menu.h"
#include "window.h"
#include "../ui/toolkit.h"
#include "../ui/themes.h"

#define MENU_ITEM_HEIGHT 24
#define MENU_ITEM_PADDING 5
#define MENU_BORDER_WIDTH 1
#define MENU_SEPARATOR_HEIGHT 5
#define MAX_MENU_ITEMS 64
#define DEFAULT_MENU_WIDTH 180

// Global display and window references
static Display *display;
static Window root;
static int screen;
static Theme *current_theme;

// Currently visible menus
static Menu **visible_menus = NULL;
static int visible_menu_count = 0;
static int max_visible_menus = 10;

// Forward declarations
static void draw_menu(Menu *menu);
static void draw_menu_item(Menu *menu, int index);
static void execute_menu_item(MenuItem *item);
static void position_submenu(Menu *parent, Menu *submenu);
static void handle_menu_click(Menu *menu, int x, int y);

// Initialize the menu system
void init_menu(Display *dpy, Window root_win, int scr, Theme *theme) {
    display = dpy;
    root = root_win;
    screen = scr;
    current_theme = theme;
    
    // Allocate memory for visible menus array
    visible_menus = (Menu **)malloc(max_visible_menus * sizeof(Menu *));
    visible_menu_count = 0;
    
    printf("Menu system initialized\n");
}

// Create a new menu
Menu *create_menu() {
    Menu *menu = (Menu *)malloc(sizeof(Menu));
    if (!menu) {
        fprintf(stderr, "Failed to allocate memory for menu\n");
        return NULL;
    }
    
    // Initialize the menu structure
    menu->x = 0;
    menu->y = 0;
    menu->width = DEFAULT_MENU_WIDTH;
    menu->height = 0;  // Will be calculated based on items
    menu->visible = 0;
    menu->window = None;
    menu->items = (MenuItem **)malloc(MAX_MENU_ITEMS * sizeof(MenuItem *));
    menu->item_count = 0;
    menu->selected_item = -1;
    menu->parent = NULL;
    
    if (!menu->items) {
        fprintf(stderr, "Failed to allocate memory for menu items\n");
        free(menu);
        return NULL;
    }
    
    return menu;
}

// Add an item to a menu
void add_menu_item(Menu *menu, const char *label, const char *command, void (*callback)(MenuItem *item)) {
    if (!menu || menu->item_count >= MAX_MENU_ITEMS) {
        return;
    }
    
    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (!item) {
        fprintf(stderr, "Failed to allocate memory for menu item\n");
        return;
    }
    
    // Initialize the menu item
    item->type = MENU_ITEM_NORMAL;
    item->label = label ? strdup(label) : NULL;
    item->command = command ? strdup(command) : NULL;
    item->submenu = NULL;
    item->enabled = 1;
    item->callback = callback;
    
    // Add to the menu
    menu->items[menu->item_count++] = item;
    
    // Update the menu height
    menu->height += MENU_ITEM_HEIGHT;
}

// Add a submenu item
void add_submenu_item(Menu *menu, const char *label, Menu *submenu) {
    if (!menu || !submenu || menu->item_count >= MAX_MENU_ITEMS) {
        return;
    }
    
    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (!item) {
        fprintf(stderr, "Failed to allocate memory for menu item\n");
        return;
    }
    
    // Initialize the menu item
    item->type = MENU_ITEM_SUBMENU;
    item->label = label ? strdup(label) : NULL;
    item->command = NULL;
    item->submenu = submenu;
    item->enabled = 1;
    item->callback = NULL;
    
    // Set the parent menu
    submenu->parent = menu;
    
    // Add to the menu
    menu->items[menu->item_count++] = item;
    
    // Update the menu height
    menu->height += MENU_ITEM_HEIGHT;
}

// Add a separator to a menu
void add_menu_separator(Menu *menu) {
    if (!menu || menu->item_count >= MAX_MENU_ITEMS) {
        return;
    }
    
    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (!item) {
        fprintf(stderr, "Failed to allocate memory for menu separator\n");
        return;
    }
    
    // Initialize the separator
    item->type = MENU_ITEM_SEPARATOR;
    item->label = NULL;
    item->command = NULL;
    item->submenu = NULL;
    item->enabled = 0;
    item->callback = NULL;
    
    // Add to the menu
    menu->items[menu->item_count++] = item;
    
    // Update the menu height
    menu->height += MENU_SEPARATOR_HEIGHT;
}

// Create the menu window
static void create_menu_window(Menu *menu) {
    if (!menu || menu->window != None) {
        return;
    }
    
    // Create the menu window
    menu->window = XCreateSimpleWindow(
        display, root,
        menu->x, menu->y,
        menu->width, menu->height,
        MENU_BORDER_WIDTH, current_theme->border_color, current_theme->menu_bg_color
    );
    
    // Select events for the menu
    XSelectInput(display, menu->window, ExposureMask | ButtonPressMask | 
                               ButtonReleaseMask | PointerMotionMask | 
                               LeaveWindowMask);
}

// Show a menu at a specific position
void show_menu(Menu *menu, int x, int y) {
    if (!menu) {
        return;
    }
    
    // Create the menu window if needed
    if (menu->window == None) {
        create_menu_window(menu);
    }
    
    // Position the menu
    menu->x = x;
    menu->y = y;
    XMoveWindow(display, menu->window, x, y);
    
    // Make sure the menu is on-screen
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
    
    // Map the window
    XMapRaised(display, menu->window);
    menu->visible = 1;
    
    // Add to visible menus list
    if (visible_menu_count < max_visible_menus) {
        visible_menus[visible_menu_count++] = menu;
    }
    
    // Draw the menu
    draw_menu(menu);
}

// Hide a menu
void hide_menu(Menu *menu) {
    if (!menu || !menu->visible) {
        return;
    }
    
    // Hide any visible submenus first
    for (int i = 0; i < menu->item_count; i++) {
        if (menu->items[i]->type == MENU_ITEM_SUBMENU && 
            menu->items[i]->submenu && 
            menu->items[i]->submenu->visible) {
            hide_menu(menu->items[i]->submenu);
        }
    }
    
    // Unmap the window
    XUnmapWindow(display, menu->window);
    menu->visible = 0;
    
    // Remove from visible menus list
    for (int i = 0; i < visible_menu_count; i++) {
        if (visible_menus[i] == menu) {
            // Remove this menu
            for (int j = i; j < visible_menu_count - 1; j++) {
                visible_menus[j] = visible_menus[j + 1];
            }
            visible_menu_count--;
            break;
        }
    }
}

// Hide all visible menus
static void hide_all_menus() {
    while (visible_menu_count > 0) {
        hide_menu(visible_menus[0]);
    }
}

// Draw a menu
static void draw_menu(Menu *menu) {
    if (!menu || !menu->visible) {
        return;
    }
    
    // Clear the window
    XClearWindow(display, menu->window);
    
    // Draw each menu item
    int y = 0;
    for (int i = 0; i < menu->item_count; i++) {
        draw_menu_item(menu, i);
    }
}

// Draw a menu item
static void draw_menu_item(Menu *menu, int index) {
    if (!menu || index < 0 || index >= menu->item_count) {
        return;
    }
    
    MenuItem *item = menu->items[index];
    GC gc = XCreateGC(display, menu->window, 0, NULL);
    
    // Calculate the item's position
    int y = 0;
    for (int i = 0; i < index; i++) {
        if (menu->items[i]->type == MENU_ITEM_SEPARATOR) {
            y += MENU_SEPARATOR_HEIGHT;
        } else {
            y += MENU_ITEM_HEIGHT;
        }
    }
    
    if (item->type == MENU_ITEM_SEPARATOR) {
        // Draw a separator line
        XSetForeground(display, gc, current_theme->border_color);
        XDrawLine(display, menu->window, gc,
                 MENU_ITEM_PADDING, y + MENU_SEPARATOR_HEIGHT / 2,
                 menu->width - MENU_ITEM_PADDING, y + MENU_SEPARATOR_HEIGHT / 2);
    } else {
        // Draw the background
        unsigned long bg_color;
        if (index == menu->selected_item) {
            bg_color = current_theme->menu_highlight_bg_color;
        } else {
            bg_color = current_theme->menu_bg_color;
        }
        
        XSetForeground(display, gc, bg_color);
        XFillRectangle(display, menu->window, gc,
                      0, y, menu->width, MENU_ITEM_HEIGHT);
        
        // Draw the text
        if (item->label) {
            unsigned long text_color;
            if (item->enabled) {
                text_color = current_theme->menu_fg_color;
            } else {
                text_color = current_theme->menu_disabled_fg_color;
            }
            
            XSetForeground(display, gc, text_color);
            XDrawString(display, menu->window, gc,
                       MENU_ITEM_PADDING, y + MENU_ITEM_HEIGHT - MENU_ITEM_PADDING,
                       item->label, strlen(item->label));
        }
        
        // Draw submenu indicator
        if (item->type == MENU_ITEM_SUBMENU) {
            XSetForeground(display, gc, current_theme->menu_fg_color);
            
            // Draw a triangle
            int arrow_x = menu->width - MENU_ITEM_PADDING - 8;
            int arrow_y = y + MENU_ITEM_HEIGHT / 2;
            XPoint points[3] = {
                {arrow_x, arrow_y - 4},
                {arrow_x, arrow_y + 4},
                {arrow_x + 8, arrow_y}
            };
            XFillPolygon(display, menu->window, gc, points, 3, Convex, CoordModeOrigin);
        }
    }
    
    XFreeGC(display, gc);
}

// Position a submenu relative to its parent
static void position_submenu(Menu *parent, Menu *submenu) {
    if (!parent || !submenu || !parent->visible) {
        return;
    }
    
    // Position to the right of the parent menu
    int x = parent->x + parent->width;
    
    // Align with the selected item
    int y = parent->y;
    for (int i = 0; i < parent->selected_item; i++) {
        if (parent->items[i]->type == MENU_ITEM_SEPARATOR) {
            y += MENU_SEPARATOR_HEIGHT;
        } else {
            y += MENU_ITEM_HEIGHT;
        }
    }
    
    // Show the submenu
    show_menu(submenu, x, y);
}

// Handle a click on a menu
static void handle_menu_click(Menu *menu, int x, int y) {
    if (!menu) {
        return;
    }
    
    // Find which item was clicked
    int item_y = 0;
    for (int i = 0; i < menu->item_count; i++) {
        int item_height = (menu->items[i]->type == MENU_ITEM_SEPARATOR) ? 
                         MENU_SEPARATOR_HEIGHT : MENU_ITEM_HEIGHT;
        
        if (y >= item_y && y < item_y + item_height) {
            // This item was clicked
            MenuItem *item = menu->items[i];
            
            if (item->type == MENU_ITEM_NORMAL && item->enabled) {
                // Execute the menu item
                execute_menu_item(item);
                hide_all_menus();
            } else if (item->type == MENU_ITEM_SUBMENU && item->enabled) {
                // Show the submenu
                menu->selected_item = i;
                draw_menu_item(menu, i);
                position_submenu(menu, item->submenu);
            }
            
            break;
        }
        
        item_y += item_height;
    }
}

// Handle mouse movement over a menu
static void handle_menu_motion(Menu *menu, int x, int y) {
    if (!menu) {
        return;
    }
    
    // Find which item the mouse is over
    int item_y = 0;
    int old_selected = menu->selected_item;
    menu->selected_item = -1;
    
    for (int i = 0; i < menu->item_count; i++) {
        int item_height = (menu->items[i]->type == MENU_ITEM_SEPARATOR) ? 
                         MENU_SEPARATOR_HEIGHT : MENU_ITEM_HEIGHT;
        
        if (y >= item_y && y < item_y + item_height) {
            // Mouse is over this item
            if (menu->items[i]->type != MENU_ITEM_SEPARATOR && menu->items[i]->enabled) {
                menu->selected_item = i;
            }
            break;
        }
        
        item_y += item_height;
    }
    
    // Redraw if selection changed
    if (old_selected != menu->selected_item) {
        if (old_selected != -1) {
            draw_menu_item(menu, old_selected);
        }
        
        if (menu->selected_item != -1) {
            draw_menu_item(menu, menu->selected_item);
            
            // If this is a submenu, show it
            if (menu->items[menu->selected_item]->type == MENU_ITEM_SUBMENU) {
                position_submenu(menu, menu->items[menu->selected_item]->submenu);
            }
        }
        
        // Hide submenus that are no longer selected
        for (int i = 0; i < menu->item_count; i++) {
            if (i != menu->selected_item && 
                menu->items[i]->type == MENU_ITEM_SUBMENU &&
                menu->items[i]->submenu->visible) {
                hide_menu(menu->items[i]->submenu);
            }
        }
    }
}

// Execute a menu item
static void execute_menu_item(MenuItem *item) {
    if (!item || !item->enabled) {
        return;
    }
    
    // Call the callback if provided
    if (item->callback) {
        item->callback(item);
    }
    
    // Execute the command if provided
    if (item->command) {
        // Execute via system - in a real WM would use fork/exec
        // For better implementation use fork+execl as in panel.c
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "%s &", item->command);
        system(cmd);
    }
}

// Handle menu events
int handle_menu_event(XEvent *event) {
    // Find which menu this event is for
    Menu *target_menu = NULL;
    for (int i = 0; i < visible_menu_count; i++) {
        if (event->xany.window == visible_menus[i]->window) {
            target_menu = visible_menus[i];
            break;
        }
    }
    
    if (!target_menu) {
        // Not a menu event
        return 0;
    }
    
    // Handle the event
    switch (event->type) {
        case Expose:
            draw_menu(target_menu);
            return 1;
            
        case ButtonPress:
            handle_menu_click(target_menu, event->xbutton.x, event->xbutton.y);
            return 1;
            
        case MotionNotify:
            handle_menu_motion(target_menu, event->xmotion.x, event->xmotion.y);
            return 1;
            
        case LeaveNotify:
            // Clear selection
            if (target_menu->selected_item != -1) {
                int old_selected = target_menu->selected_item;
                target_menu->selected_item = -1;
                draw_menu_item(target_menu, old_selected);
            }
            return 1;
            
        case ButtonRelease:
            // Generally handled by ButtonPress
            return 1;
    }
    
    return 0;
}

// Example callback for window operations
static void window_menu_callback(MenuItem *item) {
    if (!item || !item->command) {
        return;
    }
    
    // Get the window ID from the command string
    Window win = 0;
    sscanf(item->command, "window:%lu %s", &win, (char *)item->command);
    
    if (win == 0) {
        return;
    }
    
    // Get the command (after "window:ID ")
    char *cmd = strchr(item->command, ' ');
    if (!cmd) {
        return;
    }
    cmd++;
    
    // Execute the command
    if (strcmp(cmd, "close") == 0) {
        close_window(win);
    } else if (strcmp(cmd, "maximize") == 0) {
        maximize_window(win);
    } else if (strcmp(cmd, "minimize") == 0) {
        minimize_window(win);
    } else if (strcmp(cmd, "restore") == 0) {
        restore_window(win);
    }
}

// Create the root (desktop) menu
Menu *create_root_menu() {
    Menu *menu = create_menu();
    if (!menu) {
        return NULL;
    }
    
    // Add standard items
    add_menu_item(menu, "Terminal", "xterm", NULL);
    add_menu_item(menu, "File Manager", "pcmanfm", NULL);
    add_menu_item(menu, "Web Browser", "firefox", NULL);
    add_menu_separator(menu);
    
    // Create a submenu
    Menu *submenu = create_menu();
    add_menu_item(submenu, "Text Editor", "gedit", NULL);
    add_menu_item(submenu, "Calculator", "xcalc", NULL);
    add_submenu_item(menu, "Accessories", submenu);
    
    add_menu_separator(menu);
    add_menu_item(menu, "Logout", "exit", NULL);
    
    return menu;
}

// Create a window context menu for a specific window
Menu *create_window_menu(Window win) {
    Menu *menu = create_menu();
    if (!menu) {
        return NULL;
    }
    
    // Create the commands with window ID embedded
    char cmd_buffer[64];
    
    snprintf(cmd_buffer, sizeof(cmd_buffer), "window:%lu maximize", win);
    add_menu_item(menu, "Maximize", strdup(cmd_buffer), window_menu_callback);
    
    snprintf(cmd_buffer, sizeof(cmd_buffer), "window:%lu minimize", win);
    add_menu_item(menu, "Minimize", strdup(cmd_buffer), window_menu_callback);
    
    snprintf(cmd_buffer, sizeof(cmd_buffer), "window:%lu restore", win);
    add_menu_item(menu, "Restore", strdup(cmd_buffer), window_menu_callback);
    
    add_menu_separator(menu);
    
    snprintf(cmd_buffer, sizeof(cmd_buffer), "window:%lu close", win);
    add_menu_item(menu, "Close", strdup(cmd_buffer), window_menu_callback);
    
    return menu;
}

// Free a menu and all its resources
void free_menu(Menu *menu) {
    if (!menu) {
        return;
    }
    
    // First hide the menu if visible
    if (menu->visible) {
        hide_menu(menu);
    }
    
    // Free all menu items
    for (int i = 0; i < menu->item_count; i++) {
        MenuItem *item = menu->items[i];
        
        // Free the label and command
        if (item->label) {
            free(item->label);
        }
        
        if (item->command) {
            free(item->command);
        }
        
        // Free the item itself
        free(item);
    }
    
    // Free the items array
    free(menu->items);
    
    // Destroy the window if it exists
    if (menu->window != None) {
        XDestroyWindow(display, menu->window);
    }
    
    // Free the menu structure
    free(menu);
}
