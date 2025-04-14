
#ifndef ENGINE3D_H
#define ENGINE3D_H

#include <stdint.h>

typedef struct { float x, y, z; } Vec3;
typedef struct { int x, y; float z; uint32_t color; } Vertex;

void draw_triangle(Vertex a, Vertex b, Vertex c, int ox, int oy);
void init_3d_engine(void);

#endif
