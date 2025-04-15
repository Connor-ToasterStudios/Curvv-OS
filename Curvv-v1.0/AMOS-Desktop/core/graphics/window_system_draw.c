/**
 * AMOS Desktop OS - Window System Draw Implementation
 * 
 * This file implements the window drawing function with tab support.
 */

#include "window.h"

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
        
        // Skip tabbed windows (they're drawn by their parent)
        if (window->flags & AMOS_WINDOW_FLAG_TABBED) {
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
        
        // Draw tab area if this window has tabs
        if (window->flags & AMOS_WINDOW_FLAG_TABBABLE && window->tab_count > 0) {
            amos_window_draw_tabs(target_fb, window);
        }
        
        // Get the framebuffer to draw
        amos_framebuffer_t* content_fb = window->framebuffer;
        amos_rect_t client_rect;
        amos_window_get_client_rect(window, &client_rect);
        
        // If this window has tabs, draw the active tab's content
        if (window->flags & AMOS_WINDOW_FLAG_TABBABLE && window->tab_count > 0) {
            amos_window_t* active_tab = amos_window_get_active_tab(window);
            if (active_tab && active_tab->framebuffer && active_tab->framebuffer->initialized) {
                content_fb = active_tab->framebuffer;
                
                // Adjust client rectangle for tab area
                client_rect.y += 25;  // Height of tab area
                client_rect.height -= 25;
            }
        }
        
        // Draw window content
        if (content_fb && content_fb->initialized) {
            // Simple content blit (no alpha blending for simplicity)
            for (int y = 0; y < content_fb->height; y++) {
                int target_y = client_rect.y + y;
                if (target_y >= 0 && target_y < target_fb->height) {
                    for (int x = 0; x < content_fb->width; x++) {
                        int target_x = client_rect.x + x;
                        if (target_x >= 0 && target_x < target_fb->width) {
                            amos_color_t pixel = amos_fb_get_pixel(content_fb, x, y);
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