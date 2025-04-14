/**
 * AMOS Desktop OS - Framebuffer Interface
 * 
 * This file defines the framebuffer interface for the AMOS Desktop OS.
 * It provides low-level graphics rendering functions.
 */

#ifndef AMOS_FRAMEBUFFER_H
#define AMOS_FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>

// RGB color type (32-bit RGBA)
typedef uint32_t amos_color_t;

// Rectangle structure
typedef struct {
    int x;
    int y;
    int width;
    int height;
} amos_rect_t;

// Framebuffer structure
typedef struct {
    uint8_t* buffer;         // Pixel data buffer
    int width;               // Width in pixels
    int height;              // Height in pixels
    int bytes_per_pixel;     // Bytes per pixel (3 for RGB, 4 for RGBA)
    int pitch;               // Bytes per row (may include padding)
    bool initialized;        // Whether the framebuffer is initialized
} amos_framebuffer_t;

/**
 * Initialize a framebuffer
 * 
 * @param fb Pointer to framebuffer structure
 * @param width Width in pixels
 * @param height Height in pixels
 * @param bpp Bytes per pixel (3 for RGB, 4 for RGBA)
 * @return true if initialization was successful, false otherwise
 */
bool amos_fb_init(amos_framebuffer_t* fb, int width, int height, int bpp);

/**
 * Clean up a framebuffer and release resources
 * 
 * @param fb Pointer to framebuffer structure
 */
void amos_fb_cleanup(amos_framebuffer_t* fb);

/**
 * Clear framebuffer to a specific color
 * 
 * @param fb Pointer to framebuffer structure
 * @param color Color to clear with
 */
void amos_fb_clear(amos_framebuffer_t* fb, amos_color_t color);

/**
 * Set a pixel in the framebuffer
 * 
 * @param fb Pointer to framebuffer structure
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color value
 */
void amos_fb_set_pixel(amos_framebuffer_t* fb, int x, int y, amos_color_t color);

/**
 * Get a pixel from the framebuffer
 * 
 * @param fb Pointer to framebuffer structure
 * @param x X coordinate
 * @param y Y coordinate
 * @return Color value at the specified position
 */
amos_color_t amos_fb_get_pixel(const amos_framebuffer_t* fb, int x, int y);

/**
 * Draw a horizontal line
 * 
 * @param fb Pointer to framebuffer structure
 * @param x1 Starting X coordinate
 * @param y Y coordinate
 * @param x2 Ending X coordinate
 * @param color Line color
 */
void amos_fb_draw_hline(amos_framebuffer_t* fb, int x1, int y, int x2, amos_color_t color);

/**
 * Draw a vertical line
 * 
 * @param fb Pointer to framebuffer structure
 * @param x X coordinate
 * @param y1 Starting Y coordinate
 * @param y2 Ending Y coordinate
 * @param color Line color
 */
void amos_fb_draw_vline(amos_framebuffer_t* fb, int x, int y1, int y2, amos_color_t color);

/**
 * Draw a line between two points
 * 
 * @param fb Pointer to framebuffer structure
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 * @param color Line color
 */
void amos_fb_draw_line(amos_framebuffer_t* fb, int x1, int y1, int x2, int y2, amos_color_t color);

/**
 * Draw a rectangle outline
 * 
 * @param fb Pointer to framebuffer structure
 * @param rect Pointer to rectangle structure
 * @param color Rectangle outline color
 */
void amos_fb_draw_rect(amos_framebuffer_t* fb, const amos_rect_t* rect, amos_color_t color);

/**
 * Fill a rectangle with a color
 * 
 * @param fb Pointer to framebuffer structure
 * @param rect Pointer to rectangle structure
 * @param color Fill color
 */
void amos_fb_fill_rect(amos_framebuffer_t* fb, const amos_rect_t* rect, amos_color_t color);

/**
 * Draw a circle outline
 * 
 * @param fb Pointer to framebuffer structure
 * @param x_center X coordinate of center
 * @param y_center Y coordinate of center
 * @param radius Circle radius
 * @param color Circle outline color
 */
void amos_fb_draw_circle(amos_framebuffer_t* fb, int x_center, int y_center, int radius, amos_color_t color);

/**
 * Fill a circle with a color
 * 
 * @param fb Pointer to framebuffer structure
 * @param x_center X coordinate of center
 * @param y_center Y coordinate of center
 * @param radius Circle radius
 * @param color Fill color
 */
void amos_fb_fill_circle(amos_framebuffer_t* fb, int x_center, int y_center, int radius, amos_color_t color);

/**
 * Create an RGBA color value
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255)
 * @return Color value
 */
amos_color_t amos_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Create an RGB color value (with alpha=255)
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Color value
 */
amos_color_t amos_color_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * Extract RGBA components from a color value
 * 
 * @param color Color value
 * @param r Pointer to store red component
 * @param g Pointer to store green component
 * @param b Pointer to store blue component
 * @param a Pointer to store alpha component
 */
void amos_color_get_rgba(amos_color_t color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);

/**
 * Blend two colors using alpha blending
 * 
 * @param src Source color (to be blended over destination)
 * @param dst Destination color
 * @return Blended color
 */
amos_color_t amos_color_blend(amos_color_t src, amos_color_t dst);

#endif /* AMOS_FRAMEBUFFER_H */