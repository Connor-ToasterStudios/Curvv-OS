#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
    uint32_t* pixels;
} Bitmap;

Bitmap load_bitmap(uint8_t* data);
void draw_bitmap(Bitmap bmp, int x, int y);

#endif