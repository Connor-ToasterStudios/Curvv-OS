/**
 * AMOS Desktop OS - Desktop Environment Integration Header
 * 
 * This file defines the interface for integrating the desktop environment
 * with the AMOS kernel.
 */

#ifndef AMOS_DESKTOP_INIT_H
#define AMOS_DESKTOP_INIT_H

#include <stdbool.h>

// Forward declarations
typedef struct amos_framebuffer_t amos_framebuffer_t;
typedef struct amos_window_system_t amos_window_system_t;
typedef struct amos_renderer3d_t amos_renderer3d_t;
typedef struct amos_window_t amos_window_t;

// Taskbar height
#define AMOS_TASKBAR_HEIGHT 35

/**
 * Desktop environment configuration structure
 */
typedef struct {
    int screen_width;            // Screen width in pixels
    int screen_height;           // Screen height in pixels
    unsigned int background_color;  // Desktop background color
    bool enable_3d;              // Whether to enable 3D rendering
    bool enable_browser;         // Whether to enable web browser functionality
    const char* theme_name;      // Desktop theme name
    const char* font_name;       // Font to use
    int font_size;               // Font size
} amos_desktop_config_t;

/**
 * Desktop environment state structure
 */
typedef struct {
    amos_desktop_config_t config;      // Desktop configuration
    amos_framebuffer_t* fb;           // System framebuffer
    amos_window_system_t* window_system;  // Window management system
    amos_renderer3d_t* renderer;      // 3D renderer (optional)
    amos_window_t* controller;        // Desktop controller window
    bool running;                     // Whether the desktop is running
} amos_desktop_state_t;

/**
 * Initialize the desktop environment
 * 
 * @param config Desktop configuration
 * @return true if initialization was successful, false otherwise
 */
bool amos_desktop_init(const amos_desktop_config_t* config);

/**
 * Clean up the desktop environment
 */
void amos_desktop_cleanup();

/**
 * Run the desktop environment main loop
 */
void amos_desktop_run();

/**
 * Process input events from the kernel
 */
void amos_desktop_process_events();

/**
 * Update desktop state
 */
void amos_desktop_update();

/**
 * Render the desktop environment
 */
void amos_desktop_render();

/**
 * Create default desktop applications
 */
void amos_desktop_create_default_apps();

/**
 * Initialize desktop taskbar
 * 
 * @return true if initialization was successful, false otherwise
 */
bool amos_desktop_init_taskbar();

/**
 * Clean up desktop taskbar
 */
void amos_desktop_cleanup_taskbar();

/**
 * Draw the desktop taskbar
 * 
 * @param fb Target framebuffer
 */
void amos_desktop_draw_taskbar(amos_framebuffer_t* fb);

/**
 * Initialize desktop icons
 * 
 * @return true if initialization was successful, false otherwise
 */
bool amos_desktop_init_icons();

/**
 * Clean up desktop icons
 */
void amos_desktop_cleanup_icons();

/**
 * Draw desktop icons
 * 
 * @param fb Target framebuffer
 */
void amos_desktop_draw_icons(amos_framebuffer_t* fb);

/**
 * Flush framebuffer to screen
 */
void amos_desktop_flush_framebuffer();

/**
 * Get mouse state from kernel
 * 
 * @param x Pointer to store X coordinate
 * @param y Pointer to store Y coordinate
 * @param buttons Pointer to store button state
 */
void amos_desktop_get_mouse_state(int* x, int* y, int* buttons);

/**
 * Sleep for specified milliseconds
 * 
 * @param ms Milliseconds to sleep
 */
void amos_desktop_sleep(int ms);

/**
 * Get desktop environment state
 * 
 * @return Pointer to desktop state structure
 */
amos_desktop_state_t* amos_desktop_get_state();

/**
 * Create desktop controller application
 */
void amos_desktop_create_controller();

/**
 * Handle desktop controller commands
 * 
 * @param command Command string
 * @return true if command was handled, false otherwise
 */
bool amos_desktop_controller_handle_command(const char* command);

/**
 * Launch an application by name
 * 
 * @param app_name Application name
 * @return true if application was launched, false otherwise
 */
bool amos_desktop_launch_application(const char* app_name);

#endif /* AMOS_DESKTOP_INIT_H */