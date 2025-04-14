#include "graphics.h"
#include "ui.h"
#include "events.h"
#include "bitmap.h"
#include "app_manager.h"

extern uint8_t bmp_icon_data[];
static Bitmap icon;

void desktop_init() {
    icon = load_bitmap(bmp_icon_data);
    app_manager_init();
}

void desktop_draw() {
    graphics_draw_rect(0, 0, 1024, 768, 0x1E1E1E);
    graphics_draw_text("MyOS Desktop", 20, 10, 0xFFFFFF);
    draw_bitmap(icon, 20, 40);
    app_manager_draw();
}
