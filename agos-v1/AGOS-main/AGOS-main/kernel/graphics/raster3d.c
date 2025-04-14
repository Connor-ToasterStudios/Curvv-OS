
#include "raster3d.h"
#include "graphics.h"
#include <math.h>

static Triangle cube_faces[] = {
    {{ {-1,-1,-1}, {1,-1,-1}, {1,1,-1} }, 0xFF0000},
    {{ {-1,-1,-1}, {1,1,-1}, {-1,1,-1} }, 0xFF0000},
    {{ {1,-1,-1}, {1,-1,1}, {1,1,1} }, 0x00FF00},
    {{ {1,-1,-1}, {1,1,1}, {1,1,-1} }, 0x00FF00},
    {{ {1,-1,1}, {-1,-1,1}, {-1,1,1} }, 0x0000FF},
    {{ {1,-1,1}, {-1,1,1}, {1,1,1} }, 0x0000FF},
    {{ {-1,-1,1}, {-1,-1,-1}, {-1,1,-1} }, 0xFFFF00},
    {{ {-1,-1,1}, {-1,1,-1}, {-1,1,1} }, 0xFFFF00},
    {{ {-1,1,-1}, {1,1,-1}, {1,1,1} }, 0xFF00FF},
    {{ {-1,1,-1}, {1,1,1}, {-1,1,1} }, 0xFF00FF},
    {{ {-1,-1,1}, {1,-1,1}, {1,-1,-1} }, 0x00FFFF},
    {{ {-1,-1,1}, {1,-1,-1}, {-1,-1,-1} }, 0x00FFFF}
};

static Vec3 rotate_y(Vec3 v, float angle) {
    float c = cosf(angle), s = sinf(angle);
    return (Vec3){v.x * c + v.z * s, v.y, -v.x * s + v.z * c};
}

static void project(Vec3 v, int* x, int* y) {
    float fov = 256, z = v.z + 4;
    *x = (int)((v.x * fov) / z + 512);
    *y = (int)((v.y * fov) / z + 384);
}

static void draw_triangle(Vec3 a, Vec3 b, Vec3 c, uint32_t color) {
    int x1, y1, x2, y2, x3, y3;
    project(a, &x1, &y1);
    project(b, &x2, &y2);
    project(c, &x3, &y3);

    graphics_draw_line(x1, y1, x2, y2, color);
    graphics_draw_line(x2, y2, x3, y3, color);
    graphics_draw_line(x3, y3, x1, y1, color);
}

void draw_raster_cube() {
    static float angle = 0;
    for (int i = 0; i < sizeof(cube_faces)/sizeof(Triangle); i++) {
        Triangle t = cube_faces[i];
        Vec3 a = rotate_y(t.v[0], angle);
        Vec3 b = rotate_y(t.v[1], angle);
        Vec3 c = rotate_y(t.v[2], angle);
        draw_triangle(a, b, c, t.color);
    }
    angle += 0.01f;
}
