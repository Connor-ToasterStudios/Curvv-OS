/**
 * AMOS Desktop OS - Window Management System Implementation
 * 
 * This file implements the window management interface for the AMOS Desktop OS.
 * It provides the core functionality for creating, manipulating, and rendering windows.
 */

#include "window.h"
#include <stdlib.h>
#include <string.h>

// Constants for window rendering
#define TITLE_BAR_HEIGHT 30
#define BORDER_WIDTH 1
#define BUTTON_SIZE 15
#define BUTTON_MARGIN 8

// Initialize the window system
bool amos_window_system_init(amos_window_system_t* system) {
    if (!system) {
        return false;
    }
    
    // Zero-initialize the structure
    memset(system, 0, sizeof(amos_window_system_t));
    
    // Set default theme colors
    system->title_bar_color = amos_color_rgb(9, 132, 227);  // Blue
    system->title_bar_active_color = amos_color_rgb(45, 152, 247);  // Brighter blue
    system->border_color = amos_color_rgb(178, 190, 195);  // Gray
    system->border_active_color = amos_color_rgb(9, 132, 227);  // Blue
    system->text_color = amos_color_rgb(255, 255, 255);  // White
    system->button_color = amos_color_rgb(116, 185, 255);  // Light blue
    system->button_hover_color = amos_color_rgb(144, 205, 255);  // Lighter blue
    
    return true;
}

// Clean up and release window system resources
void amos_window_system_cleanup(amos_window_system_t* system) {
    if (!system) {
        return;
    }
    
    // Destroy all windows
    for (int i = 0; i < system->window_count; i++) {
        amos_window_destroy(system, system->windows[i]);
    }
    
    // Reset state
    system->window_count = 0;
    system->active_window = NULL;
    system->drag_window = NULL;
    system->resize_window = NULL;
}

// Create a new window
amos_window_t* amos_window_create(
    amos_window_system_t* system,
    const char* title,
    int x, int y,
    int width, int height,
    amos_window_style_t style,
    uint32_t flags
) {
    if (!system || !title || width <= 0 || height <= 0) {
        return NULL;
    }
    
    // Check if we have room for another window
    if (system->window_count >= AMOS_MAX_WINDOWS) {
        return NULL;
    }
    
    // Allocate memory for the window
    amos_window_t* window = (amos_window_t*)malloc(sizeof(amos_window_t));
    if (!window) {
        return NULL;
    }
    
    // Initialize window properties
    window->id = system->window_count;
    strncpy(window->title, title, AMOS_MAX_TITLE_LENGTH - 1);
    window->title[AMOS_MAX_TITLE_LENGTH - 1] = '\0';  // Ensure null termination
    
    window->rect.x = x;
    window->rect.y = y;
    window->rect.width = width;
    window->rect.height = height;
    
    window->saved_rect = window->rect;  // Save initial size and position
    window->flags = flags;
    window->style = style;
    window->bg_color = amos_color_rgb(223, 230, 233);  // Light gray
    window->active = false;
    
    window->draw_callback = NULL;
    window->event_callback = NULL;
    window->user_data = NULL;
    
    // Create framebuffer for window content
    int content_width = width;
    int content_height = height - TITLE_BAR_HEIGHT;
    if (content_width <= 0 || content_height <= 0) {
        free(window);
        return NULL;
    }
    
    window->framebuffer = (amos_framebuffer_t*)malloc(sizeof(amos_framebuffer_t));
    if (!window->framebuffer) {
        free(window);
        return NULL;
    }
    
    if (!amos_fb_init(window->framebuffer, content_width, content_height, 4)) {
        free(window->framebuffer);
        free(window);
        return NULL;
    }
    
    // Clear window content
    amos_fb_clear(window->framebuffer, window->bg_color);
    
    // Link pointers
    window->next = NULL;
    window->prev = NULL;
    
    // Add window to system
    system->windows[system->window_count] = window;
    system->window_count++;
    
    // Make this the active window
    amos_window_activate(system, window);
    
    return window;
}

// Destroy a window and release its resources
void amos_window_destroy(amos_window_system_t* system, amos_window_t* window) {
    if (!system || !window) {
        return;
    }
    
    // Find the window in the system
    int index = -1;
    for (int i = 0; i < system->window_count; i++) {
        if (system->windows[i] == window) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        // Window not found
        return;
    }
    
    // Remove from system arrays and shift remaining windows
    for (int i = index; i < system->window_count - 1; i++) {
        system->windows[i] = system->windows[i + 1];
    }
    
    system->window_count--;
    
    // Update active window if necessary
    if (system->active_window == window) {
        system->active_window = (system->window_count > 0) ? 
                                system->windows[system->window_count - 1] : NULL;
    }
    
    // Update drag window if necessary
    if (system->drag_window == window) {
        system->drag_window = NULL;
    }
    
    // Update resize window if necessary
    if (system->resize_window == window) {
        system->resize_window = NULL;
    }
    
    // Release window resources
    if (window->framebuffer) {
        amos_fb_cleanup(window->framebuffer);
        free(window->framebuffer);
    }
    
    free(window);
}

// Set the window's draw callback function
void amos_window_set_draw_callback(amos_window_t* window, amos_window_draw_fn callback) {
    if (window) {
        window->draw_callback = callback;
    }
}

// Set the window's event callback function
void amos_window_set_event_callback(amos_window_t* window, amos_window_event_fn callback) {
    if (window) {
        window->event_callback = callback;
    }
}

// Set the window's user data pointer
void amos_window_set_user_data(amos_window_t* window, void* user_data) {
    if (window) {
        window->user_data = user_data;
    }
}

// Set the window's background color
void amos_window_set_bg_color(amos_window_t* window, amos_color_t color) {
    if (window) {
        window->bg_color = color;
    }
}

// Set the window's title
void amos_window_set_title(amos_window_t* window, const char* title) {
    if (window && title) {
        strncpy(window->title, title, AMOS_MAX_TITLE_LENGTH - 1);
        window->title[AMOS_MAX_TITLE_LENGTH - 1] = '\0';  // Ensure null termination
    }
}

// Move the window to a new position
void amos_window_move(amos_window_t* window, int x, int y) {
    if (window) {
        window->rect.x = x;
        window->rect.y = y;
    }
}

// Resize the window
void amos_window_resize(amos_window_t* window, int width, int height) {
    if (!window || width <= 0 || height <= 0) {
        return;
    }
    
    // Update window rectangle
    window->rect.width = width;
    window->rect.height = height;
    
    // Resize framebuffer
    int content_width = width;
    int content_height = height - TITLE_BAR_HEIGHT;
    if (content_width <= 0 || content_height <= 0) {
        return;
    }
    
    // Clean up old framebuffer
    amos_fb_cleanup(window->framebuffer);
    
    // Initialize new framebuffer
    amos_fb_init(window->framebuffer, content_width, content_height, 4);
    
    // Clear new framebuffer
    amos_fb_clear(window->framebuffer, window->bg_color);
}

// Show the window
void amos_window_show(amos_window_t* window) {
    if (window) {
        window->flags &= ~AMOS_WINDOW_FLAG_HIDDEN;
    }
}

// Hide the window
void amos_window_hide(amos_window_t* window) {
    if (window) {
        window->flags |= AMOS_WINDOW_FLAG_HIDDEN;
    }
}

// Maximize the window
void amos_window_maximize(amos_window_t* window) {
    if (!window) {
        return;
    }
    
    if (!(window->flags & AMOS_WINDOW_FLAG_MAXIMIZED)) {
        // Save current size and position
        window->saved_rect = window->rect;
        
        // Set maximized flag
        window->flags |= AMOS_WINDOW_FLAG_MAXIMIZED;
        
        // For a real implementation, the new size would be the desktop size
        // For this prototype, just use some large values
        window->rect.x = 0;
        window->rect.y = TITLE_BAR_HEIGHT;  // Leave room for taskbar
        window->rect.width = 1024;  // Example desktop width
        window->rect.height = 768 - TITLE_BAR_HEIGHT;  // Example desktop height minus taskbar
        
        // Resize the framebuffer
        amos_window_resize(window, window->rect.width, window->rect.height);
    }
}

// Minimize the window
void amos_window_minimize(amos_window_t* window) {
    if (window) {
        window->flags |= AMOS_WINDOW_FLAG_MINIMIZED;
    }
}

// Restore the window to its previous size and position
void amos_window_restore(amos_window_t* window) {
    if (!window) {
        return;
    }
    
    if (window->flags & AMOS_WINDOW_FLAG_MAXIMIZED) {
        // Clear maximized flag
        window->flags &= ~AMOS_WINDOW_FLAG_MAXIMIZED;
        
        // Restore previous size and position
        window->rect = window->saved_rect;
        
        // Resize the framebuffer
        amos_window_resize(window, window->rect.width, window->rect.height);
    }
    
    if (window->flags & AMOS_WINDOW_FLAG_MINIMIZED) {
        // Clear minimized flag
        window->flags &= ~AMOS_WINDOW_FLAG_MINIMIZED;
    }
}

// Set the window as active (bring to front, focus)
void amos_window_activate(amos_window_system_t* system, amos_window_t* window) {
    if (!system || !window) {
        return;
    }
    
    // Deactivate current active window
    if (system->active_window) {
        system->active_window->active = false;
    }
    
    // Set as active window
    window->active = true;
    system->active_window = window;
    
    // Bring to front (reorder in windows array)
    for (int i = 0; i < system->window_count; i++) {
        if (system->windows[i] == window) {
            // Move to end of array (top of z-order)
            for (int j = i; j < system->window_count - 1; j++) {
                system->windows[j] = system->windows[j + 1];
            }
            system->windows[system->window_count - 1] = window;
            break;
        }
    }
}

// Draw all windows in the window system to a framebuffer
void amos_window_system_draw(amos_window_system_t* system, amos_framebuffer_t* target_fb) {
    if (!system || !target_fb) {
        return;
    }
    
    // Draw windows from bottom to top
    for (int i = 0; i < system->window_count; i++) {
        amos_window_t* window = system->windows[i];
        
        // Skip hidden or minimized windows
        if ((window->flags & AMOS_WINDOW_FLAG_HIDDEN) || 
            (window->flags & AMOS_WINDOW_FLAG_MINIMIZED)) {
            continue;
        }
        
        // Draw window border
        amos_rect_t border_rect = window->rect;
        amos_color_t border_color = window->active ? 
                                   system->border_active_color : 
                                   system->border_color;
        amos_fb_draw_rect(target_fb, &border_rect, border_color);
        
        // Draw title bar
        amos_rect_t title_bar_rect;
        amos_window_get_titlebar_rect(window, &title_bar_rect);
        
        amos_color_t title_bar_color = window->active ? 
                                      system->title_bar_active_color : 
                                      system->title_bar_color;
        amos_fb_fill_rect(target_fb, &title_bar_rect, title_bar_color);
        
        // Draw window title
        // Simplified text rendering here - would need proper font rendering
        
        // Draw window buttons (close, maximize, minimize)
        amos_rect_t close_btn_rect, max_btn_rect, min_btn_rect;
        amos_window_get_close_button_rect(window, &close_btn_rect);
        amos_window_get_maximize_button_rect(window, &max_btn_rect);
        amos_window_get_minimize_button_rect(window, &min_btn_rect);
        
        amos_fb_fill_rect(target_fb, &close_btn_rect, amos_color_rgb(255, 0, 0));  // Red close button
        amos_fb_fill_rect(target_fb, &max_btn_rect, amos_color_rgb(253, 203, 110));  // Yellow maximize button
        amos_fb_fill_rect(target_fb, &min_btn_rect, amos_color_rgb(0, 184, 148));  // Green minimize button
        
        // Draw window content
        if (window->framebuffer && window->framebuffer->initialized) {
            // Get client area rect
            amos_rect_t client_rect;
            amos_window_get_client_rect(window, &client_rect);
            
            // Simple content blit (no alpha blending for simplicity)
            for (int y = 0; y < window->framebuffer->height; y++) {
                int target_y = client_rect.y + y;
                if (target_y >= 0 && target_y < target_fb->height) {
                    for (int x = 0; x < window->framebuffer->width; x++) {
                        int target_x = client_rect.x + x;
                        if (target_x >= 0 && target_x < target_fb->width) {
                            amos_color_t pixel = amos_fb_get_pixel(window->framebuffer, x, y);
                            amos_fb_set_pixel(target_fb, target_x, target_y, pixel);
                        }
                    }
                }
            }
        }
        
        // Call custom draw callback if set
        if (window->draw_callback) {
            window->draw_callback(window, target_fb);
        }
    }
}

// Handle mouse move events
bool amos_window_system_handle_mouse_move(amos_window_system_t* system, int x, int y) {
    if (!system) {
        return false;
    }
    
    // Handle window dragging
    if (system->drag_window) {
        int new_x = x - system->drag_offset_x;
        int new_y = y - system->drag_offset_y;
        amos_window_move(system->drag_window, new_x, new_y);
        return true;
    }
    
    // Handle window resizing
    if (system->resize_window) {
        int new_width = x - system->resize_window->rect.x;
        int new_height = y - system->resize_window->rect.y;
        
        // Ensure minimum size
        new_width = (new_width < 100) ? 100 : new_width;
        new_height = (new_height < 100) ? 100 : new_height;
        
        amos_window_resize(system->resize_window, new_width, new_height);
        return true;
    }
    
    // Update hover states, cursor, etc.
    return false;
}

// Handle mouse button press events
bool amos_window_system_handle_mouse_down(amos_window_system_t* system, int x, int y, int button) {
    if (!system) {
        return false;
    }
    
    // Check from top to bottom (reverse order of drawing)
    for (int i = system->window_count - 1; i >= 0; i--) {
        amos_window_t* window = system->windows[i];
        
        // Skip hidden or minimized windows
        if ((window->flags & AMOS_WINDOW_FLAG_HIDDEN) || 
            (window->flags & AMOS_WINDOW_FLAG_MINIMIZED)) {
            continue;
        }
        
        // Check if mouse is inside window
        if (x >= window->rect.x && x < window->rect.x + window->rect.width &&
            y >= window->rect.y && y < window->rect.y + window->rect.height) {
            
            // Activate window if not already active
            if (!window->active) {
                amos_window_activate(system, window);
            }
            
            // Check title bar (for dragging)
            amos_rect_t title_bar_rect;
            amos_window_get_titlebar_rect(window, &title_bar_rect);
            
            if (x >= title_bar_rect.x && x < title_bar_rect.x + title_bar_rect.width &&
                y >= title_bar_rect.y && y < title_bar_rect.y + title_bar_rect.height) {
                
                // Check window buttons
                amos_rect_t close_btn_rect, max_btn_rect, min_btn_rect;
                amos_window_get_close_button_rect(window, &close_btn_rect);
                amos_window_get_maximize_button_rect(window, &max_btn_rect);
                amos_window_get_minimize_button_rect(window, &min_btn_rect);
                
                if (x >= close_btn_rect.x && x < close_btn_rect.x + close_btn_rect.width &&
                    y >= close_btn_rect.y && y < close_btn_rect.y + close_btn_rect.height) {
                    // Close button clicked
                    amos_window_destroy(system, window);
                    return true;
                } else if (x >= max_btn_rect.x && x < max_btn_rect.x + max_btn_rect.width &&
                           y >= max_btn_rect.y && y < max_btn_rect.y + max_btn_rect.height) {
                    // Maximize button clicked
                    if (window->flags & AMOS_WINDOW_FLAG_MAXIMIZED) {
                        amos_window_restore(window);
                    } else {
                        amos_window_maximize(window);
                    }
                    return true;
                } else if (x >= min_btn_rect.x && x < min_btn_rect.x + min_btn_rect.width &&
                           y >= min_btn_rect.y && y < min_btn_rect.y + min_btn_rect.height) {
                    // Minimize button clicked
                    amos_window_minimize(window);
                    return true;
                } else {
                    // Start dragging window
                    system->drag_window = window;
                    system->drag_offset_x = x - window->rect.x;
                    system->drag_offset_y = y - window->rect.y;
                    return true;
                }
            }
            
            // Check resize handle (bottom-right corner)
            int resize_handle_size = 16;
            if (x >= window->rect.x + window->rect.width - resize_handle_size &&
                y >= window->rect.y + window->rect.height - resize_handle_size) {
                // Start resizing window
                system->resize_window = window;
                return true;
            }
            
            // Handle client area events
            amos_rect_t client_rect;
            amos_window_get_client_rect(window, &client_rect);
            
            if (x >= client_rect.x && x < client_rect.x + client_rect.width &&
                y >= client_rect.y && y < client_rect.y + client_rect.height) {
                
                // Convert to client coordinates
                int client_x = x - client_rect.x;
                int client_y = y - client_rect.y;
                
                // Call custom event handler if set
                if (window->event_callback) {
                    // In a real implementation, we would create a proper event structure
                    void* event_data = NULL;  // Placeholder
                    if (window->event_callback(window, event_data)) {
                        return true;
                    }
                }
            }
            
            return true;  // Event handled by this window
        }
    }
    
    return false;  // No window handled the event
}

// Handle mouse button release events
bool amos_window_system_handle_mouse_up(amos_window_system_t* system, int x, int y, int button) {
    if (!system) {
        return false;
    }
    
    bool handled = false;
    
    // End any dragging or resizing
    if (system->drag_window) {
        system->drag_window = NULL;
        handled = true;
    }
    
    if (system->resize_window) {
        system->resize_window = NULL;
        handled = true;
    }
    
    return handled;
}

// Calculate the top title bar area of a window
void amos_window_get_titlebar_rect(const amos_window_t* window, amos_rect_t* rect) {
    if (!window || !rect) {
        return;
    }
    
    rect->x = window->rect.x;
    rect->y = window->rect.y;
    rect->width = window->rect.width;
    rect->height = TITLE_BAR_HEIGHT;
}

// Calculate client area (content area) of a window
void amos_window_get_client_rect(const amos_window_t* window, amos_rect_t* rect) {
    if (!window || !rect) {
        return;
    }
    
    rect->x = window->rect.x;
    rect->y = window->rect.y + TITLE_BAR_HEIGHT;
    rect->width = window->rect.width;
    rect->height = window->rect.height - TITLE_BAR_HEIGHT;
}

// Calculate close button rectangle
void amos_window_get_close_button_rect(const amos_window_t* window, amos_rect_t* rect) {
    if (!window || !rect) {
        return;
    }
    
    rect->x = window->rect.x + window->rect.width - BUTTON_SIZE - BUTTON_MARGIN;
    rect->y = window->rect.y + BUTTON_MARGIN;
    rect->width = BUTTON_SIZE;
    rect->height = BUTTON_SIZE;
}

// Calculate maximize button rectangle
void amos_window_get_maximize_button_rect(const amos_window_t* window, amos_rect_t* rect) {
    if (!window || !rect) {
        return;
    }
    
    rect->x = window->rect.x + window->rect.width - (BUTTON_SIZE * 2) - (BUTTON_MARGIN * 2);
    rect->y = window->rect.y + BUTTON_MARGIN;
    rect->width = BUTTON_SIZE;
    rect->height = BUTTON_SIZE;
}

// Calculate minimize button rectangle
void amos_window_get_minimize_button_rect(const amos_window_t* window, amos_rect_t* rect) {
    if (!window || !rect) {
        return;
    }
    
    rect->x = window->rect.x + window->rect.width - (BUTTON_SIZE * 3) - (BUTTON_MARGIN * 3);
    rect->y = window->rect.y + BUTTON_MARGIN;
    rect->width = BUTTON_SIZE;
    rect->height = BUTTON_SIZE;
}