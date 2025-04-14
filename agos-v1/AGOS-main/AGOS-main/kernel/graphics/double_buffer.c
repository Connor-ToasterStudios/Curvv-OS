
#include "double_buffer.h"
#include <string.h>

static framebuffer_t* fb;
static uint32_t* back_buffer;

void dbuf_init(framebuffer_t* framebuffer) {
    fb = framebuffer;
    back_buffer = (uint32_t*)malloc(fb->width * fb->height * sizeof(uint32_t));
}

void dbuf_draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < fb->width && y >= 0 && y < fb->height) {
        back_buffer[y * fb->width + x] = color;
    }
}

void dbuf_clear(uint32_t color) {
    for (int i = 0; i < fb->width * fb->height; i++) {
        back_buffer[i] = color;
    }
}

void dbuf_swap(void) {
    memcpy(fb->buffer, back_buffer, fb->width * fb->height * sizeof(uint32_t));
}
