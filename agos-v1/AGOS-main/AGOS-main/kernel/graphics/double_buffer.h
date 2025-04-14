
#ifndef DOUBLE_BUFFER_H
#define DOUBLE_BUFFER_H

#include <stdint.h>
#include "framebuffer.h"

void dbuf_init(framebuffer_t* fb);
void dbuf_draw_pixel(int x, int y, uint32_t color);
void dbuf_clear(uint32_t color);
void dbuf_swap(void);

#endif
