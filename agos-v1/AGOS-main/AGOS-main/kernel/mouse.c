
#include "mouse.h"
#include "interrupts/irqhandlers.h"

static int mouse_x = 0, mouse_y = 0;
static int buttons = 0;

void init_mouse(void) {
    // Mouse initialization code here
    outb(0x64, 0xA8);
    outb(0x64, 0x20);
    uint8_t status = inb(0x60) | 2;
    outb(0x64, 0x60);
    outb(0x60, status);
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    inb(0x60);
}

void mouse_update(void) {
    // Mouse update code here
    uint8_t data = inb(0x60);
    buttons = data & 0x07;
    int dx = ((data & 0x10) ? 0xFF00 : 0) | inb(0x60);
    int dy = ((data & 0x20) ? 0xFF00 : 0) | inb(0x60);
    
    mouse_x = (mouse_x + dx < 0) ? 0 : ((mouse_x + dx > 799) ? 799 : mouse_x + dx);
    mouse_y = (mouse_y - dy < 0) ? 0 : ((mouse_y - dy > 599) ? 599 : mouse_y - dy);
}

int mouse_get_x(void) { return mouse_x; }
int mouse_get_y(void) { return mouse_y; }
int mouse_get_left(void) { return buttons & 1; }
int mouse_get_right(void) { return (buttons >> 1) & 1; }
