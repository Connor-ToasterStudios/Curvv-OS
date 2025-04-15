/*
 * themes.h - Theme management for the window manager
 */

#ifndef THEMES_H
#define THEMES_H

#include <X11/Xlib.h>

// Theme structure
typedef struct {
    char *name;
    
    // Window colors
    unsigned long bg_color;
    unsigned long fg_color;
    unsigned long border_color;
    
    // Title bar colors
    unsigned long title_bg_color;
    unsigned long title_fg_color;
    unsigned long title_active_bg_color;
    unsigned long title_active_fg_color;
    
    // Button colors
    unsigned long button_bg_color;
    unsigned long button_fg_color;
    unsigned long button_hover_bg_color;
    unsigned long button_active_bg_color;
    
    // Panel colors
    unsigned long panel_bg_color;
    unsigned long panel_fg_color;
    
    // Menu colors
    unsigned long menu_bg_color;
    unsigned long menu_fg_color;
    unsigned long menu_highlight_bg_color;
    unsigned long menu_disabled_fg_color;
    
    // Taskbar colors
    unsigned long taskbar_button_bg;
    unsigned long taskbar_button_fg;
    unsigned long taskbar_button_active_bg;
    
    // Desktop colors
    unsigned long desktop_bg_color;
    unsigned long desktop_icon_color;
    unsigned long desktop_icon_label_bg;
    unsigned long desktop_icon_label_fg;
    
    // Workspace switcher colors
    unsigned long workspace_active_color;
    
    // Font configuration
    char *font_name;
    int font_size;
    
    // Border widths
    int window_border_width;
    int button_border_width;
    
    // Dimensions
    int titlebar_height;
    int panel_height;
    int menu_item_height;
    int button_corner_radius;
} Theme;

// Initialize theme system
void init_themes();

// Load a theme by name
int load_theme(Theme *theme, const char *theme_name);

// Get the current theme
Theme *get_current_theme();

// Set the current theme
void set_current_theme(const char *theme_name);

// Create a default theme
void create_default_theme(Theme *theme);

// Create a dark theme
void create_dark_theme(Theme *theme);

// Create a light theme
void create_light_theme(Theme *theme);

// Apply a theme to a window
void apply_theme_to_window(Window window, const Theme *theme);

// Free theme resources
void free_theme(Theme *theme);

#endif /* THEMES_H */
