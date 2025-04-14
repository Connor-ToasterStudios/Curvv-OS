
#ifndef FONT_H
#define FONT_H

#include <stdint.h>

extern const uint8_t font_data[256][16];
void draw_char(char c, int x, int y, uint32_t color);
void draw_text(const char* text, int x, int y, uint32_t color);

#endif
