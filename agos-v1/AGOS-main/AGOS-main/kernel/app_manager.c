
#include "app_manager.h"
#include "graphics/graphics.h"
#include "window.h"

#define MAX_APPS 16

typedef struct {
    char name[32];
    void (*start)(void);
    bool running;
} App;

static App apps[MAX_APPS];
static int app_count = 0;

void app_manager_init(void) {
    app_count = 0;
}

void app_manager_register(const char* name, void (*start)(void)) {
    if (app_count < MAX_APPS) {
        strncpy(apps[app_count].name, name, 31);
        apps[app_count].start = start;
        apps[app_count].running = false;
        app_count++;
    }
}

void app_manager_draw(void) {
    for (int i = 0; i < app_count; i++) {
        if (apps[i].running) {
            // Draw app windows here
        }
    }
}
