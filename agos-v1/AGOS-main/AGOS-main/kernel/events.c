#include "mouse.h"
#include "events.h"

static int last_left = 0;
static int click = 0;

void events_init() {}

void events_poll() {
    int curr = mouse_get_left();
    click = (curr == 1 && last_left == 0);
    last_left = curr;
}

int is_mouse_over(int x, int y, int w, int h) {
    int mx = mouse_get_x(), my = mouse_get_y();
    return mx >= x && mx < (x + w) && my >= y && my < (y + h);
}

int mouse_clicked() {
    return click;
}
