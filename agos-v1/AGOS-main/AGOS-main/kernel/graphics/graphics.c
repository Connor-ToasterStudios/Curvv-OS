
#include "graphics.h"
#include "double_buffer.h"
#include "font.h"

void init_graphics(void) {
    dbuf_init((framebuffer_t*)0xFD000000);
    dbuf_clear(0x000000);
}

void graphics_draw_pixel(int x, int y, uint32_t color) {
    dbuf_draw_pixel(x, y, color);
}

void graphics_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            graphics_draw_pixel(x + i, y + j, color);
        }
    }
}

void graphics_swap_buffers(void) {
    dbuf_swap();
}
