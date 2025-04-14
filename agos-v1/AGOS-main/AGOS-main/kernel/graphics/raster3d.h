
#ifndef RASTER3D_H
#define RASTER3D_H

#include <stdint.h>

typedef struct { float x, y, z; } Vec3;
typedef struct { Vec3 v[3]; uint32_t color; } Triangle;

void draw_raster_cube(void);
void init_3d_renderer(void);
void draw_3d_scene(int ox, int oy);

#endif
