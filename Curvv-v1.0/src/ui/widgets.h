/*
 * widgets.h - Widget definitions for the UI toolkit
 */

#ifndef WIDGETS_H
#define WIDGETS_H

#include <X11/Xlib.h>
#include "themes.h"

// Initialize widgets
void init_widgets();

// Desktop icon widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *label;
    char *icon_path;
    char *command;
} DesktopIcon;

// Create a desktop icon
DesktopIcon *create_desktop_icon(int x, int y, const char *label, 
                               const char *icon_path, const char *command);

// Destroy a desktop icon
void destroy_desktop_icon(DesktopIcon *icon);

// Draw desktop icon
void draw_desktop_icon(DesktopIcon *icon);

// Taskbar button widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *label;
    Window app_window;
    int is_active;
} TaskbarButton;

// Create a taskbar button
TaskbarButton *create_taskbar_button(int x, int y, int width, int height, 
                                   const char *label, Window app_window);

// Set taskbar button active state
void taskbar_button_set_active(TaskbarButton *button, int active);

// Destroy a taskbar button
void destroy_taskbar_button(TaskbarButton *button);

// Draw taskbar button
void draw_taskbar_button(TaskbarButton *button);

// Menu item widget
typedef struct MenuItem {
    int type;  // 0=normal, 1=separator, 2=submenu
    char *label;
    char *command;
    struct Menu *submenu;
    int enabled;
    void (*callback)(struct MenuItem *);
} MenuItem;

// Menu widget
typedef struct Menu {
    Window window;
    int x, y;
    int width, height;
    MenuItem **items;
    int item_count;
    int selected_item;
    struct Menu *parent;
} Menu;

// Create a menu
Menu *create_menu();

// Add a menu item
void add_menu_item(Menu *menu, const char *label, const char *command, 
                  void (*callback)(MenuItem *));

// Add a submenu
void add_submenu(Menu *menu, const char *label, Menu *submenu);

// Add a separator
void add_menu_separator(Menu *menu);

// Show a menu
void show_menu(Menu *menu, int x, int y);

// Hide a menu
void hide_menu(Menu *menu);

// Handle menu events
int handle_menu_event(XEvent *event);

// System tray widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    Window *icons;
    int icon_count;
} SystemTray;

// Create a system tray
SystemTray *create_system_tray(int x, int y, int width, int height);

// Add an icon to the system tray
void system_tray_add_icon(SystemTray *tray, Window icon);

// Remove an icon from the system tray
void system_tray_remove_icon(SystemTray *tray, Window icon);

// Destroy a system tray
void destroy_system_tray(SystemTray *tray);

// Volume control widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    int volume;
    int muted;
} VolumeControl;

// Create a volume control
VolumeControl *create_volume_control(int x, int y, int width, int height);

// Set volume
void volume_control_set_volume(VolumeControl *control, int volume);

// Toggle mute
void volume_control_toggle_mute(VolumeControl *control);

// Destroy a volume control
void destroy_volume_control(VolumeControl *control);

// Clock widget
typedef struct {
    Window window;
    int x, y;
    int width, int height;
    int format;  // 0=24h, 1=12h, 2=date+time
    int show_seconds;
} Clock;

// Create a clock
Clock *create_clock(int x, int y, int width, int height, int format, int show_seconds);

// Update clock display
void update_clock(Clock *clock);

// Destroy a clock
void destroy_clock(Clock *clock);

// Workspace switcher widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    int workspace_count;
    int current_workspace;
} WorkspaceSwitcher;

// Create a workspace switcher
WorkspaceSwitcher *create_workspace_switcher(int x, int y, int width, int height, int count);

// Set current workspace
void workspace_switcher_set_current(WorkspaceSwitcher *switcher, int workspace);

// Destroy a workspace switcher
void destroy_workspace_switcher(WorkspaceSwitcher *switcher);

#endif /* WIDGETS_H */
