
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

void init_graphics(void);
void graphics_draw_pixel(int x, int y, uint32_t color);
void graphics_draw_rect(int x, int y, int w, int h, uint32_t color);
void graphics_draw_text(const char* text, int x, int y, uint32_t color);
void graphics_swap_buffers(void);

#endif
