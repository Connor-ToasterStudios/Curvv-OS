#include "interrupts/idt.h"
#include "utils.h"
#include "interrupts/timer.h"
#include "amosstartascii.h"
#include "graphics.h"
#include "events.h"
#include "multitask.h"
#include "desktop.h"
#include "app_manager.h"

int main() {
    print(amosLogoLow);
    initIdt();
    init_graphics();
    init_keyboard();
    init_mouse();
    multitask_init();
    app_manager_init();
    desktop_init();
    
    while (1) {
        desktop_draw();
        draw_raster_cube();
        handle_events();
        mouse_update();
    }
}