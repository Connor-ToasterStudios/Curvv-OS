#include "bitmap.h"
#include "graphics/graphics.h"
#include <stdlib.h>

Bitmap load_bitmap(uint8_t* data) {
    Bitmap bmp;
    bmp.width = *(uint32_t*)(data + 18);
    bmp.height = *(uint32_t*)(data + 22);
    int size = bmp.width * bmp.height;
    bmp.pixels = malloc(size * sizeof(uint32_t));
    
    uint8_t* pixel_data = data + *(uint32_t*)(data + 10);
    for (int i = 0; i < size; i++) {
        int b = pixel_data[i * 3];
        int g = pixel_data[i * 3 + 1];
        int r = pixel_data[i * 3 + 2];
        bmp.pixels[i] = (r << 16) | (g << 8) | b;
    }
    
    return bmp;
}

void draw_bitmap(Bitmap bmp, int x, int y) {
    for (int py = 0; py < bmp.height; py++) {
        for (int px = 0; px < bmp.width; px++) {
            uint32_t color = bmp.pixels[py * bmp.width + px];
            graphics_draw_pixel(x + px, y + py, color);
        }
    }
}