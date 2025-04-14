/**
 * AMOS Desktop OS - Framebuffer Implementation
 * 
 * Native C implementation of the framebuffer rendering system.
 */

#include "framebuffer.h"
#include <stdlib.h>
#include <string.h>

bool amos_fb_init(amos_framebuffer_t* fb, int width, int height, int bpp) {
    if (!fb || width <= 0 || height <= 0 || (bpp != 3 && bpp != 4)) {
        return false;
    }
    
    // Calculate required buffer size (including pitch alignment)
    fb->width = width;
    fb->height = height;
    fb->bytes_per_pixel = bpp;
    fb->pitch = width * bpp;
    
    // Align pitch to 4-byte boundary for better performance
    fb->pitch = (fb->pitch + 3) & ~3;
    
    // Allocate the buffer
    size_t buffer_size = fb->pitch * fb->height;
    fb->buffer = (uint8_t*)malloc(buffer_size);
    if (!fb->buffer) {
        return false;
    }
    
    // Clear the buffer to black
    memset(fb->buffer, 0, buffer_size);
    
    fb->initialized = true;
    return true;
}

void amos_fb_cleanup(amos_framebuffer_t* fb) {
    if (fb && fb->initialized && fb->buffer) {
        free(fb->buffer);
        fb->buffer = NULL;
        fb->initialized = false;
    }
}

void amos_fb_clear(amos_framebuffer_t* fb, amos_color_t color) {
    if (!fb || !fb->initialized || !fb->buffer) {
        return;
    }
    
    // For 32-bit (4 bytes per pixel), we can optimize by filling 32 bits at a time
    if (fb->bytes_per_pixel == 4) {
        // Cast buffer to 32-bit for faster filling
        uint32_t* buffer32 = (uint32_t*)fb->buffer;
        size_t pixel_count = fb->pitch * fb->height / 4;
        
        for (size_t i = 0; i < pixel_count; i++) {
            buffer32[i] = color;
        }
    } else {
        // For other formats, fill byte by byte
        uint8_t r, g, b, a;
        amos_color_get_rgba(color, &r, &g, &b, &a);
        
        for (int y = 0; y < fb->height; y++) {
            uint8_t* row = fb->buffer + y * fb->pitch;
            for (int x = 0; x < fb->width; x++) {
                uint8_t* pixel = row + x * fb->bytes_per_pixel;
                pixel[0] = r;  // R
                pixel[1] = g;  // G
                pixel[2] = b;  // B
                if (fb->bytes_per_pixel == 4) {
                    pixel[3] = a;  // A
                }
            }
        }
    }
}

void amos_fb_set_pixel(amos_framebuffer_t* fb, int x, int y, amos_color_t color) {
    // Bounds checking
    if (!fb || !fb->initialized || !fb->buffer || 
        x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        return;
    }
    
    uint8_t* pixel = fb->buffer + y * fb->pitch + x * fb->bytes_per_pixel;
    
    if (fb->bytes_per_pixel == 4) {
        // Direct 32-bit assignment
        *((uint32_t*)pixel) = color;
    } else {
        // Extract components and assign individually
        uint8_t r, g, b, a;
        amos_color_get_rgba(color, &r, &g, &b, &a);
        
        pixel[0] = r;  // R
        pixel[1] = g;  // G
        pixel[2] = b;  // B
    }
}

amos_color_t amos_fb_get_pixel(const amos_framebuffer_t* fb, int x, int y) {
    // Bounds checking
    if (!fb || !fb->initialized || !fb->buffer || 
        x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        return 0;
    }
    
    const uint8_t* pixel = fb->buffer + y * fb->pitch + x * fb->bytes_per_pixel;
    
    if (fb->bytes_per_pixel == 4) {
        // Direct 32-bit read
        return *((uint32_t*)pixel);
    } else {
        // Extract components and combine
        uint8_t r = pixel[0];
        uint8_t g = pixel[1];
        uint8_t b = pixel[2];
        return amos_color_rgb(r, g, b);
    }
}

void amos_fb_draw_hline(amos_framebuffer_t* fb, int x1, int y, int x2, amos_color_t color) {
    // Ensure x1 <= x2
    if (x1 > x2) {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }
    
    // Clip to framebuffer bounds
    if (y < 0 || y >= fb->height || x2 < 0 || x1 >= fb->width) {
        return;
    }
    
    x1 = (x1 < 0) ? 0 : x1;
    x2 = (x2 >= fb->width) ? fb->width - 1 : x2;
    
    // Draw the line
    for (int x = x1; x <= x2; x++) {
        amos_fb_set_pixel(fb, x, y, color);
    }
}

void amos_fb_draw_vline(amos_framebuffer_t* fb, int x, int y1, int y2, amos_color_t color) {
    // Ensure y1 <= y2
    if (y1 > y2) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    // Clip to framebuffer bounds
    if (x < 0 || x >= fb->width || y2 < 0 || y1 >= fb->height) {
        return;
    }
    
    y1 = (y1 < 0) ? 0 : y1;
    y2 = (y2 >= fb->height) ? fb->height - 1 : y2;
    
    // Draw the line
    for (int y = y1; y <= y2; y++) {
        amos_fb_set_pixel(fb, x, y, color);
    }
}

void amos_fb_draw_line(amos_framebuffer_t* fb, int x1, int y1, int x2, int y2, amos_color_t color) {
    // Implementation of Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;
    
    while (1) {
        amos_fb_set_pixel(fb, x1, y1, color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void amos_fb_draw_rect(amos_framebuffer_t* fb, const amos_rect_t* rect, amos_color_t color) {
    if (!fb || !fb->initialized || !fb->buffer || !rect) {
        return;
    }
    
    int x1 = rect->x;
    int y1 = rect->y;
    int x2 = rect->x + rect->width - 1;
    int y2 = rect->y + rect->height - 1;
    
    // Draw four lines
    amos_fb_draw_hline(fb, x1, y1, x2, color);  // Top
    amos_fb_draw_hline(fb, x1, y2, x2, color);  // Bottom
    amos_fb_draw_vline(fb, x1, y1, y2, color);  // Left
    amos_fb_draw_vline(fb, x2, y1, y2, color);  // Right
}

void amos_fb_fill_rect(amos_framebuffer_t* fb, const amos_rect_t* rect, amos_color_t color) {
    if (!fb || !fb->initialized || !fb->buffer || !rect) {
        return;
    }
    
    // Clip rectangle to framebuffer bounds
    int x1 = rect->x;
    int y1 = rect->y;
    int x2 = rect->x + rect->width - 1;
    int y2 = rect->y + rect->height - 1;
    
    if (x1 >= fb->width || y1 >= fb->height || x2 < 0 || y2 < 0) {
        return;  // Rectangle is completely outside
    }
    
    x1 = (x1 < 0) ? 0 : x1;
    y1 = (y1 < 0) ? 0 : y1;
    x2 = (x2 >= fb->width) ? fb->width - 1 : x2;
    y2 = (y2 >= fb->height) ? fb->height - 1 : y2;
    
    // Fill the rectangle
    for (int y = y1; y <= y2; y++) {
        amos_fb_draw_hline(fb, x1, y, x2, color);
    }
}

void amos_fb_draw_circle(amos_framebuffer_t* fb, int x_center, int y_center, int radius, amos_color_t color) {
    if (!fb || !fb->initialized || !fb->buffer || radius <= 0) {
        return;
    }
    
    // Implementation of Bresenham's circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        amos_fb_set_pixel(fb, x_center + x, y_center + y, color);
        amos_fb_set_pixel(fb, x_center + y, y_center + x, color);
        amos_fb_set_pixel(fb, x_center - y, y_center + x, color);
        amos_fb_set_pixel(fb, x_center - x, y_center + y, color);
        amos_fb_set_pixel(fb, x_center - x, y_center - y, color);
        amos_fb_set_pixel(fb, x_center - y, y_center - x, color);
        amos_fb_set_pixel(fb, x_center + y, y_center - x, color);
        amos_fb_set_pixel(fb, x_center + x, y_center - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void amos_fb_fill_circle(amos_framebuffer_t* fb, int x_center, int y_center, int radius, amos_color_t color) {
    if (!fb || !fb->initialized || !fb->buffer || radius <= 0) {
        return;
    }
    
    // Implementation using horizontal scanlines for filling
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        // Draw horizontal lines between opposite octant points
        amos_fb_draw_hline(fb, x_center - x, y_center + y, x_center + x, color);
        amos_fb_draw_hline(fb, x_center - y, y_center + x, x_center + y, color);
        amos_fb_draw_hline(fb, x_center - x, y_center - y, x_center + x, color);
        amos_fb_draw_hline(fb, x_center - y, y_center - x, x_center + y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

amos_color_t amos_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
}

amos_color_t amos_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return amos_color_rgba(r, g, b, 255);
}

void amos_color_get_rgba(amos_color_t color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    if (r) *r = (color & 0xFF);
    if (g) *g = ((color >> 8) & 0xFF);
    if (b) *b = ((color >> 16) & 0xFF);
    if (a) *a = ((color >> 24) & 0xFF);
}

amos_color_t amos_color_blend(amos_color_t src, amos_color_t dst) {
    uint8_t src_r, src_g, src_b, src_a;
    uint8_t dst_r, dst_g, dst_b, dst_a;
    
    amos_color_get_rgba(src, &src_r, &src_g, &src_b, &src_a);
    amos_color_get_rgba(dst, &dst_r, &dst_g, &dst_b, &dst_a);
    
    // Alpha blending formula: out = src * srcAlpha + dst * (1 - srcAlpha)
    float src_alpha = src_a / 255.0f;
    float inv_src_alpha = 1.0f - src_alpha;
    
    uint8_t out_r = (uint8_t)(src_r * src_alpha + dst_r * inv_src_alpha);
    uint8_t out_g = (uint8_t)(src_g * src_alpha + dst_g * inv_src_alpha);
    uint8_t out_b = (uint8_t)(src_b * src_alpha + dst_b * inv_src_alpha);
    uint8_t out_a = (uint8_t)(src_a + dst_a * inv_src_alpha);
    
    return amos_color_rgba(out_r, out_g, out_b, out_a);
}