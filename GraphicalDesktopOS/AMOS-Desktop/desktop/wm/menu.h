/*
 * menu.h - Menu system for the window manager
 */

#ifndef MENU_H
#define MENU_H

#include <X11/Xlib.h>
#include "../ui/themes.h"

// Menu item types
typedef enum {
    MENU_ITEM_NORMAL,   // Regular clickable menu item
    MENU_ITEM_SUBMENU,  // Item with a submenu
    MENU_ITEM_SEPARATOR // Horizontal separator line
} MenuItemType;

// Menu item structure
typedef struct MenuItem {
    MenuItemType type;
    char *label;
    char *command;
    struct Menu *submenu;
    int enabled;
    void (*callback)(struct MenuItem *item);
} MenuItem;

// Menu structure
typedef struct Menu {
    Window window;
    int x, y;
    int width, height;
    int visible;
    MenuItem **items;
    int item_count;
    int selected_item;
    struct Menu *parent;
} Menu;

// Initialize the menu system
void init_menu(Display *dpy, Window root_win, int scr, Theme *theme);

// Create a new menu
Menu *create_menu();

// Add an item to a menu
void add_menu_item(Menu *menu, const char *label, const char *command, void (*callback)(MenuItem *item));

// Add a submenu item
void add_submenu_item(Menu *menu, const char *label, Menu *submenu);

// Add a separator to a menu
void add_menu_separator(Menu *menu);

// Show a menu at a specific position
void show_menu(Menu *menu, int x, int y);

// Hide a menu
void hide_menu(Menu *menu);

// Handle menu events
int handle_menu_event(XEvent *event);

// Create the root (desktop) menu
Menu *create_root_menu();

// Create a window context menu for a specific window
Menu *create_window_menu(Window win);

// Free a menu and all its resources
void free_menu(Menu *menu);

#endif /* MENU_H */
