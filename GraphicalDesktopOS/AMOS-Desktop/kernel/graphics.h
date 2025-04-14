#ifndef GRAPHICS_H
#define GRAPHICS_H

int graphics_init(void* framebuffer, int width, int height, int depth);
void graphics_cleanup();
void* get_framebuffer();
int get_screen_width();
int get_screen_height();
int get_color_depth();

#endif /* GRAPHICS_H */
