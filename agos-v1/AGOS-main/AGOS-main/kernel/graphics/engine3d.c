#include "engine3d.h"
#include "graphics.h"
#include <math.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600

static float zbuffer[HEIGHT][WIDTH];

typedef struct { float x, y, z; } Vec3;
typedef struct { int x, y; float z; uint32_t color; } Vertex;

static Vec3 verts[] = {
    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
    {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1},
};

static int tris[][3] = {
    {0,1,2}, {0,2,3}, {4,5,6}, {4,6,7},
    {0,1,5}, {0,5,4}, {2,3,7}, {2,7,6},
    {1,2,6}, {1,6,5}, {3,0,4}, {3,4,7},
};

static Vec3 rotate(Vec3 v, float angle) {
    float c = cosf(angle), s = sinf(angle);
    float x = v.x * c + v.z * s;
    float z = -v.x * s + v.z * c;
    return (Vec3){x, v.y, z};
}

void draw_triangle(Vertex a, Vertex b, Vertex c, int ox, int oy) {
    int minX = fmax(0, fmin(a.x, fmin(b.x, c.x)));
    int maxX = fmin(WIDTH - 1, fmax(a.x, fmax(b.x, c.x)));
    int minY = fmax(0, fmin(a.y, fmin(b.y, c.y)));
    int maxY = fmin(HEIGHT - 1, fmax(a.y, fmax(b.y, c.y)));

    float area = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    if (area == 0) return;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float w0 = ((b.x - a.x)*(y - a.y) - (b.y - a.y)*(x - a.x)) / area;
            float w1 = ((c.x - b.x)*(y - b.y) - (c.y - b.y)*(x - b.x)) / area;
            float w2 = 1 - w0 - w1;
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float z = a.z * w0 + b.z * w1 + c.z * w2;
                if (z > zbuffer[y][x]) {
                    zbuffer[y][x] = z;
                    graphics_draw_pixel(x + ox, y + oy, a.color);
                }
            }
        }
    }
}

void init_3d_engine(void) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            zbuffer[y][x] = -999999.0f;
        }
    }
}

void draw_3d_engine(int ox, int oy) {
    init_3d_engine(); // Initialize zbuffer

    static float angle = 0;
    angle += 0.01f;

    for (int i = 0; i < sizeof(tris) / sizeof(tris[0]); i++) {
        Vec3 v1 = rotate(verts[tris[i][0]], angle);
        Vec3 v2 = rotate(verts[tris[i][1]], angle);
        Vec3 v3 = rotate(verts[tris[i][2]], angle);

        Vertex a = {(v1.x * 200 + WIDTH/2), (v1.y * 200 + HEIGHT/2), v1.z + 3, 0x00FF00}; //Adjusted scaling
        Vertex b = {(v2.x * 200 + WIDTH/2), (v2.y * 200 + HEIGHT/2), v2.z + 3, 0x00FF00}; //Adjusted scaling
        Vertex c = {(v3.x * 200 + WIDTH/2), (v3.y * 200 + HEIGHT/2), v3.z + 3, 0x00FF00}; //Adjusted scaling

        draw_triangle(a, b, c, ox, oy);
    }
}