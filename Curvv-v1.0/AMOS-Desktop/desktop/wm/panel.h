/*
 * panel.h - Panel/taskbar implementation for the window manager
 */

#ifndef PANEL_H
#define PANEL_H

#include <X11/Xlib.h>
#include "../ui/themes.h"

// Panel positions
typedef enum {
    PANEL_POSITION_TOP,
    PANEL_POSITION_BOTTOM,
    PANEL_POSITION_LEFT,
    PANEL_POSITION_RIGHT
} PanelPosition;

// Panel item types
typedef enum {
    PANEL_ITEM_LAUNCHER,
    PANEL_ITEM_TASKBUTTON,
    PANEL_ITEM_CLOCK,
    PANEL_ITEM_SYSTEM_TRAY,
    PANEL_ITEM_WORKSPACE_SWITCHER,
    PANEL_ITEM_SEPARATOR,
    PANEL_ITEM_CUSTOM
} PanelItemType;

// Panel item structure
typedef struct {
    PanelItemType type;
    Window window;
    int x, y;
    int width, height;
    char *label;
    Window associated_window;  // For taskbuttons
    int workspace;             // For workspace switcher
    void (*click_handler)(int button, int x, int y);  // Click event handler
} PanelItem;

// Initialize the panel
void init_panel(Display *dpy, Window root_win, int scr, Theme *theme);

// Set the panel position
void set_panel_position(PanelPosition position);

// Add a launcher to the panel
void add_panel_launcher(const char *label, const char *command);

// Add a task button for a window
void add_panel_taskbutton(Window win, const char *label);

// Remove a task button when window is closed
void remove_panel_taskbutton(Window win);

// Update a task button label when window title changes
void update_panel_taskbutton(Window win, const char *label);

// Highlight the task button for the active window
void highlight_panel_taskbutton(Window win);

// Add clock to the panel
void add_panel_clock();

// Add system tray to the panel
void add_panel_system_tray();

// Add workspace switcher to the panel
void add_panel_workspace_switcher(int num_workspaces);

// Add separator to the panel
void add_panel_separator();

// Add custom item to the panel
void add_panel_custom(const char *label, void (*click_handler)(int button, int x, int y));

// Draw the panel
void draw_panel();

// Process panel events
int handle_panel_event(XEvent *event);

// Update the clock
void update_panel_clock();

// Show/hide the panel
void show_panel();
void hide_panel();

// Auto-hide the panel
void set_panel_autohide(int enable);

#endif /* PANEL_H */
