/**
 * AMOS Desktop OS - Window Management System
 * 
 * This file defines the window management interface for the AMOS Desktop OS.
 * It provides the core functionality for creating, manipulating, and rendering windows.
 */

#ifndef AMOS_WINDOW_H
#define AMOS_WINDOW_H

#include "framebuffer.h"
#include <stdbool.h>

// Maximum number of windows that can be managed simultaneously
#define AMOS_MAX_WINDOWS 64

// Maximum length of a window title
#define AMOS_MAX_TITLE_LENGTH 64

// Window flags
#define AMOS_WINDOW_FLAG_MOVABLE    0x0001
#define AMOS_WINDOW_FLAG_RESIZABLE  0x0002
#define AMOS_WINDOW_FLAG_MINIMIZED  0x0004
#define AMOS_WINDOW_FLAG_MAXIMIZED  0x0008
#define AMOS_WINDOW_FLAG_HIDDEN     0x0010
#define AMOS_WINDOW_FLAG_MODAL      0x0020
#define AMOS_WINDOW_FLAG_PINNED     0x0040
#define AMOS_WINDOW_FLAG_BORDERLESS 0x0080
#define AMOS_WINDOW_FLAG_TOPMOST    0x0100
#define AMOS_WINDOW_FLAG_TABBABLE   0x0200
#define AMOS_WINDOW_FLAG_TABBED     0x0400
#define AMOS_WINDOW_FLAG_ACTIVE_TAB 0x0800

// Window styles
typedef enum {
    AMOS_WINDOW_STYLE_NORMAL,
    AMOS_WINDOW_STYLE_DIALOG,
    AMOS_WINDOW_STYLE_POPUP,
    AMOS_WINDOW_STYLE_TOOLBAR,
    AMOS_WINDOW_STYLE_MENU
} amos_window_style_t;

// Forward declaration
typedef struct amos_window_t amos_window_t;

// Window draw callback function
typedef void (*amos_window_draw_fn)(amos_window_t* window, amos_framebuffer_t* fb);

// Window event callback function
typedef bool (*amos_window_event_fn)(amos_window_t* window, void* event);

// Window structure
struct amos_window_t {
    int id;                             // Unique window ID
    char title[AMOS_MAX_TITLE_LENGTH];  // Window title
    amos_rect_t rect;                   // Window rectangle (position and size)
    amos_rect_t saved_rect;             // Saved rectangle (for restore after maximize/minimize)
    uint32_t flags;                     // Window flags
    amos_window_style_t style;          // Window style
    amos_color_t bg_color;              // Background color
    bool active;                        // Whether this window is active/focused
    
    // Content buffer - each window has its own framebuffer
    amos_framebuffer_t* framebuffer;
    
    // Callback functions
    amos_window_draw_fn draw_callback;
    amos_window_event_fn event_callback;
    
    // User data pointer - can be used by applications
    void* user_data;
    
    // Linked list - general window chain
    amos_window_t* next;
    amos_window_t* prev;
    
    // Tab management
    amos_window_t* parent_window;       // Parent window (if this is a tab)
    amos_window_t* tab_group;           // First window in tab group (if this is a parent)
    amos_window_t* next_tab;            // Next tab in group
    amos_window_t* prev_tab;            // Previous tab in group
    int tab_count;                      // Number of tabs in group (if parent)
    int tab_index;                      // Index of this tab in parent
    amos_color_t tab_color;             // Tab color
};

// Window system structure
typedef struct {
    amos_window_t* windows[AMOS_MAX_WINDOWS];  // Window array
    int window_count;                          // Current number of windows
    amos_window_t* active_window;              // Currently active/focused window
    
    amos_window_t* drag_window;                // Window being dragged
    int drag_offset_x;
    int drag_offset_y;
    
    amos_window_t* resize_window;              // Window being resized
    
    // Theme colors
    amos_color_t title_bar_color;       // Title bar color
    amos_color_t title_bar_active_color;  // Active title bar color
    amos_color_t border_color;          // Window border color
    amos_color_t border_active_color;   // Active window border color
    amos_color_t text_color;            // Text color
    amos_color_t button_color;          // Window buttons color
    amos_color_t button_hover_color;    // Button hover color
} amos_window_system_t;

/**
 * Initialize the window system
 * 
 * @param system Pointer to window system structure
 * @return true if initialization successful, false otherwise
 */
bool amos_window_system_init(amos_window_system_t* system);

/**
 * Clean up and release window system resources
 * 
 * @param system Pointer to window system structure
 */
void amos_window_system_cleanup(amos_window_system_t* system);

/**
 * Create a new window
 * 
 * @param system Pointer to window system structure
 * @param title Window title
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 * @param style Window style
 * @param flags Window flags
 * @return Pointer to the created window, or NULL on failure
 */
amos_window_t* amos_window_create(
    amos_window_system_t* system,
    const char* title,
    int x, int y,
    int width, int height,
    amos_window_style_t style,
    uint32_t flags
);

/**
 * Destroy a window and release its resources
 * 
 * @param system Pointer to window system structure
 * @param window Pointer to window
 */
void amos_window_destroy(amos_window_system_t* system, amos_window_t* window);

/**
 * Set the window's draw callback function
 * 
 * @param window Pointer to window
 * @param callback Callback function to set
 */
void amos_window_set_draw_callback(amos_window_t* window, amos_window_draw_fn callback);

/**
 * Set the window's event callback function
 * 
 * @param window Pointer to window
 * @param callback Callback function to set
 */
void amos_window_set_event_callback(amos_window_t* window, amos_window_event_fn callback);

/**
 * Set the window's user data pointer
 * 
 * @param window Pointer to window
 * @param user_data User data pointer
 */
void amos_window_set_user_data(amos_window_t* window, void* user_data);

/**
 * Set the window's background color
 * 
 * @param window Pointer to window
 * @param color Background color
 */
void amos_window_set_bg_color(amos_window_t* window, amos_color_t color);

/**
 * Set the window's title
 * 
 * @param window Pointer to window
 * @param title New title
 */
void amos_window_set_title(amos_window_t* window, const char* title);

/**
 * Move the window to a new position
 * 
 * @param window Pointer to window
 * @param x New X position
 * @param y New Y position
 */
void amos_window_move(amos_window_t* window, int x, int y);

/**
 * Resize the window
 * 
 * @param window Pointer to window
 * @param width New width
 * @param height New height
 */
void amos_window_resize(amos_window_t* window, int width, int height);

/**
 * Show the window
 * 
 * @param window Pointer to window
 */
void amos_window_show(amos_window_t* window);

/**
 * Hide the window
 * 
 * @param window Pointer to window
 */
void amos_window_hide(amos_window_t* window);

/**
 * Maximize the window
 * 
 * @param window Pointer to window
 */
void amos_window_maximize(amos_window_t* window);

/**
 * Minimize the window
 * 
 * @param window Pointer to window
 */
void amos_window_minimize(amos_window_t* window);

/**
 * Restore the window to its previous size and position
 * 
 * @param window Pointer to window
 */
void amos_window_restore(amos_window_t* window);

/**
 * Set the window as active (bring to front, focus)
 * 
 * @param system Pointer to window system structure
 * @param window Pointer to window
 */
void amos_window_activate(amos_window_system_t* system, amos_window_t* window);

/**
 * Draw all windows in the window system to a framebuffer
 * 
 * @param system Pointer to window system structure
 * @param target_fb Framebuffer to draw to
 */
void amos_window_system_draw(amos_window_system_t* system, amos_framebuffer_t* target_fb);

/**
 * Handle mouse move events
 * 
 * @param system Pointer to window system structure
 * @param x Mouse x coordinate
 * @param y Mouse y coordinate
 * @return true if event was handled, false otherwise
 */
bool amos_window_system_handle_mouse_move(amos_window_system_t* system, int x, int y);

/**
 * Handle mouse button press events
 * 
 * @param system Pointer to window system structure
 * @param x Mouse x coordinate
 * @param y Mouse y coordinate
 * @param button Button pressed (1 = left, 2 = middle, 3 = right)
 * @return true if event was handled, false otherwise
 */
bool amos_window_system_handle_mouse_down(amos_window_system_t* system, int x, int y, int button);

/**
 * Handle mouse button release events
 * 
 * @param system Pointer to window system structure
 * @param x Mouse x coordinate
 * @param y Mouse y coordinate
 * @param button Button released (1 = left, 2 = middle, 3 = right)
 * @return true if event was handled, false otherwise
 */
bool amos_window_system_handle_mouse_up(amos_window_system_t* system, int x, int y, int button);

/**
 * Calculate the top title bar area of a window
 * 
 * @param window Pointer to window
 * @param rect Pointer to store rectangle
 */
void amos_window_get_titlebar_rect(const amos_window_t* window, amos_rect_t* rect);

/**
 * Calculate client area (content area) of a window
 * 
 * @param window Pointer to window
 * @param rect Pointer to store rectangle
 */
void amos_window_get_client_rect(const amos_window_t* window, amos_rect_t* rect);

/**
 * Calculate close button rectangle
 * 
 * @param window Pointer to window
 * @param rect Pointer to store rectangle
 */
void amos_window_get_close_button_rect(const amos_window_t* window, amos_rect_t* rect);

/**
 * Calculate maximize button rectangle
 * 
 * @param window Pointer to window
 * @param rect Pointer to store rectangle
 */
void amos_window_get_maximize_button_rect(const amos_window_t* window, amos_rect_t* rect);

/**
 * Calculate minimize button rectangle
 * 
 * @param window Pointer to window
 * @param rect Pointer to store rectangle
 */
void amos_window_get_minimize_button_rect(const amos_window_t* window, amos_rect_t* rect);

/**
 * Add a window as a tab to a parent window
 * 
 * @param system Pointer to window system structure
 * @param parent_window Pointer to parent window
 * @param tab_window Pointer to window to add as a tab
 * @return true if successful, false otherwise
 */
bool amos_window_add_tab(amos_window_system_t* system, amos_window_t* parent_window, amos_window_t* tab_window);

/**
 * Remove a tab from its parent window
 * 
 * @param system Pointer to window system structure
 * @param tab_window Pointer to tab window to remove
 * @return true if successful, false otherwise
 */
bool amos_window_remove_tab(amos_window_system_t* system, amos_window_t* tab_window);

/**
 * Switch to a different tab in a tabbed window
 * 
 * @param system Pointer to window system structure
 * @param parent_window Pointer to parent window
 * @param tab_index Index of tab to switch to
 * @return true if successful, false otherwise
 */
bool amos_window_switch_tab(amos_window_system_t* system, amos_window_t* parent_window, int tab_index);

/**
 * Get the active tab in a tabbed window
 * 
 * @param parent_window Pointer to parent window
 * @return Pointer to active tab window, or NULL if not a tabbed window
 */
amos_window_t* amos_window_get_active_tab(const amos_window_t* parent_window);

/**
 * Check if a window is a tab
 * 
 * @param window Pointer to window
 * @return true if window is a tab, false otherwise
 */
bool amos_window_is_tab(const amos_window_t* window);

/**
 * Check if a window has tabs
 * 
 * @param window Pointer to window
 * @return true if window has tabs, false otherwise
 */
bool amos_window_has_tabs(const amos_window_t* window);

/**
 * Calculate tab area rectangle
 * 
 * @param window Pointer to window
 * @param rect Pointer to store rectangle
 */
void amos_window_get_tab_area_rect(const amos_window_t* window, amos_rect_t* rect);

/**
 * Calculate specific tab rectangle
 * 
 * @param window Pointer to window
 * @param tab_index Tab index
 * @param rect Pointer to store rectangle
 * @return true if successful, false otherwise
 */
bool amos_window_get_tab_rect(const amos_window_t* window, int tab_index, amos_rect_t* rect);

/**
 * Draw tabs for a window
 * 
 * @param fb Target framebuffer
 * @param window Pointer to window
 */
void amos_window_draw_tabs(amos_framebuffer_t* fb, const amos_window_t* window);

#endif /* AMOS_WINDOW_H */