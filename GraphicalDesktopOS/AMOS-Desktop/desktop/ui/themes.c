/*
 * themes.c - Theme management implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "themes.h"

// Global display reference
extern Display *display;
extern int screen;

// The current theme
static Theme current_theme;
static int themes_initialized = 0;

// Initialize the theme system
void init_themes() {
    if (themes_initialized) return;
    
    // Initialize with default theme
    create_default_theme(&current_theme);
    
    themes_initialized = 1;
    printf("Theme system initialized\n");
}

// Load a theme by name
int load_theme(Theme *theme, const char *theme_name) {
    if (!theme || !theme_name) return 0;
    
    if (strcmp(theme_name, "default") == 0) {
        create_default_theme(theme);
        return 1;
    } else if (strcmp(theme_name, "dark") == 0) {
        create_dark_theme(theme);
        return 1;
    } else if (strcmp(theme_name, "light") == 0) {
        create_light_theme(theme);
        return 1;
    }
    
    // Theme not found, use default
    create_default_theme(theme);
    return 0;
}

// Get the current theme
Theme *get_current_theme() {
    if (!themes_initialized) {
        init_themes();
    }
    
    return &current_theme;
}

// Set the current theme
void set_current_theme(const char *theme_name) {
    if (!themes_initialized) {
        init_themes();
    }
    
    load_theme(&current_theme, theme_name);
}

// Helper function to get a color by name
static unsigned long get_color(const char *color_name) {
    XColor color;
    Colormap cmap = DefaultColormap(display, screen);
    
    if (!XParseColor(display, cmap, color_name, &color)) {
        fprintf(stderr, "Cannot parse color: %s\n", color_name);
        return BlackPixel(display, screen);
    }
    
    if (!XAllocColor(display, cmap, &color)) {
        fprintf(stderr, "Cannot allocate color: %s\n", color_name);
        return BlackPixel(display, screen);
    }
    
    return color.pixel;
}

// Create a default theme (medium gray with blue accents)
void create_default_theme(Theme *theme) {
    if (!theme) return;
    
    // Set theme name
    theme->name = strdup("default");
    
    // Window colors
    theme->bg_color = get_color("#d6d6d6");
    theme->fg_color = get_color("#000000");
    theme->border_color = get_color("#888888");
    
    // Title bar colors
    theme->title_bg_color = get_color("#cccccc");
    theme->title_fg_color = get_color("#000000");
    theme->title_active_bg_color = get_color("#5294e2");
    theme->title_active_fg_color = get_color("#ffffff");
    
    // Button colors
    theme->button_bg_color = get_color("#d6d6d6");
    theme->button_fg_color = get_color("#000000");
    theme->button_hover_bg_color = get_color("#e6e6e6");
    theme->button_active_bg_color = get_color("#5294e2");
    
    // Panel colors
    theme->panel_bg_color = get_color("#2f343f");
    theme->panel_fg_color = get_color("#ffffff");
    
    // Menu colors
    theme->menu_bg_color = get_color("#f5f5f5");
    theme->menu_fg_color = get_color("#000000");
    theme->menu_highlight_bg_color = get_color("#5294e2");
    theme->menu_disabled_fg_color = get_color("#888888");
    
    // Taskbar colors
    theme->taskbar_button_bg = get_color("#2f343f");
    theme->taskbar_button_fg = get_color("#ffffff");
    theme->taskbar_button_active_bg = get_color("#5294e2");
    
    // Desktop colors
    theme->desktop_bg_color = get_color("#2f343f");
    theme->desktop_icon_color = get_color("#5294e2");
    theme->desktop_icon_label_bg = get_color("#000000");
    theme->desktop_icon_label_fg = get_color("#ffffff");
    
    // Workspace switcher colors
    theme->workspace_active_color = get_color("#5294e2");
    
    // Font configuration
    theme->font_name = strdup("fixed");
    theme->font_size = 12;
    
    // Border widths
    theme->window_border_width = 1;
    theme->button_border_width = 1;
    
    // Dimensions
    theme->titlebar_height = 20;
    theme->panel_height = 30;
    theme->menu_item_height = 20;
    theme->button_corner_radius = 2;
}

// Create a dark theme
void create_dark_theme(Theme *theme) {
    if (!theme) return;
    
    // Set theme name
    theme->name = strdup("dark");
    
    // Window colors
    theme->bg_color = get_color("#2f343f");
    theme->fg_color = get_color("#ffffff");
    theme->border_color = get_color("#1a1a1a");
    
    // Title bar colors
    theme->title_bg_color = get_color("#2f343f");
    theme->title_fg_color = get_color("#d3dae3");
    theme->title_active_bg_color = get_color("#5294e2");
    theme->title_active_fg_color = get_color("#ffffff");
    
    // Button colors
    theme->button_bg_color = get_color("#383c4a");
    theme->button_fg_color = get_color("#d3dae3");
    theme->button_hover_bg_color = get_color("#404552");
    theme->button_active_bg_color = get_color("#5294e2");
    
    // Panel colors
    theme->panel_bg_color = get_color("#2f343f");
    theme->panel_fg_color = get_color("#d3dae3");
    
    // Menu colors
    theme->menu_bg_color = get_color("#383c4a");
    theme->menu_fg_color = get_color("#d3dae3");
    theme->menu_highlight_bg_color = get_color("#5294e2");
    theme->menu_disabled_fg_color = get_color("#7c818c");
    
    // Taskbar colors
    theme->taskbar_button_bg = get_color("#383c4a");
    theme->taskbar_button_fg = get_color("#d3dae3");
    theme->taskbar_button_active_bg = get_color("#5294e2");
    
    // Desktop colors
    theme->desktop_bg_color = get_color("#2f343f");
    theme->desktop_icon_color = get_color("#5294e2");
    theme->desktop_icon_label_bg = get_color("#2f343f");
    theme->desktop_icon_label_fg = get_color("#d3dae3");
    
    // Workspace switcher colors
    theme->workspace_active_color = get_color("#5294e2");
    
    // Font configuration
    theme->font_name = strdup("fixed");
    theme->font_size = 12;
    
    // Border widths
    theme->window_border_width = 1;
    theme->button_border_width = 1;
    
    // Dimensions
    theme->titlebar_height = 20;
    theme->panel_height = 30;
    theme->menu_item_height = 20;
    theme->button_corner_radius = 2;
}

// Create a light theme
void create_light_theme(Theme *theme) {
    if (!theme) return;
    
    // Set theme name
    theme->name = strdup("light");
    
    // Window colors
    theme->bg_color = get_color("#f5f5f5");
    theme->fg_color = get_color("#000000");
    theme->border_color = get_color("#cccccc");
    
    // Title bar colors
    theme->title_bg_color = get_color("#e6e6e6");
    theme->title_fg_color = get_color("#000000");
    theme->title_active_bg_color = get_color("#5294e2");
    theme->title_active_fg_color = get_color("#ffffff");
    
    // Button colors
    theme->button_bg_color = get_color("#e6e6e6");
    theme->button_fg_color = get_color("#000000");
    theme->button_hover_bg_color = get_color("#f0f0f0");
    theme->button_active_bg_color = get_color("#5294e2");
    
    // Panel colors
    theme->panel_bg_color = get_color("#e6e6e6");
    theme->panel_fg_color = get_color("#000000");
    
    // Menu colors
    theme->menu_bg_color = get_color("#f5f5f5");
    theme->menu_fg_color = get_color("#000000");
    theme->menu_highlight_bg_color = get_color("#5294e2");
    theme->menu_disabled_fg_color = get_color("#888888");
    
    // Taskbar colors
    theme->taskbar_button_bg = get_color("#e6e6e6");
    theme->taskbar_button_fg = get_color("#000000");
    theme->taskbar_button_active_bg = get_color("#5294e2");
    
    // Desktop colors
    theme->desktop_bg_color = get_color("#f5f5f5");
    theme->desktop_icon_color = get_color("#5294e2");
    theme->desktop_icon_label_bg = get_color("#f5f5f5");
    theme->desktop_icon_label_fg = get_color("#000000");
    
    // Workspace switcher colors
    theme->workspace_active_color = get_color("#5294e2");
    
    // Font configuration
    theme->font_name = strdup("fixed");
    theme->font_size = 12;
    
    // Border widths
    theme->window_border_width = 1;
    theme->button_border_width = 1;
    
    // Dimensions
    theme->titlebar_height = 20;
    theme->panel_height = 30;
    theme->menu_item_height = 20;
    theme->button_corner_radius = 2;
}

// Apply a theme to a window
void apply_theme_to_window(Window window, const Theme *theme) {
    if (!theme) return;
    
    // Set window background and border
    XSetWindowBackground(display, window, theme->bg_color);
    XSetWindowBorder(display, window, theme->border_color);
    XClearWindow(display, window);
}

// Free theme resources
void free_theme(Theme *theme) {
    if (!theme) return;
    
    // Free strings
    if (theme->name) free(theme->name);
    if (theme->font_name) free(theme->font_name);
}
