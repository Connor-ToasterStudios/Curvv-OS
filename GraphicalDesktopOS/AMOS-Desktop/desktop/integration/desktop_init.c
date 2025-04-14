/*
 * desktop_init.c - AMOS Desktop Environment Initialization
 *
 * This file provides the integration layer between the AMOS kernel
 * and the desktop environment. It initializes the framebuffer,
 * sets up input handling, and launches the window manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "desktop_init.h"
#include "../wm/window_manager.h"
#include "../ui/ui_toolkit.h"
#include "../state/state_manager.h"

/* Framebuffer device */
static char* fb_device = "/dev/fb0";

/* Global variables for framebuffer access */
static int fb_fd = -1;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static char* fb_mem = NULL;
static size_t fb_size = 0;

/* Desktop state variables */
static bool desktop_running = false;
static int desktop_width = 0;
static int desktop_height = 0;
static int desktop_depth = 0;

/*
 * Initialize the framebuffer for desktop rendering
 * Returns 0 on success, -1 on failure
 */
static int init_framebuffer(void) {
    fb_fd = open(fb_device, O_RDWR);
    if (fb_fd == -1) {
        perror("Error opening framebuffer device");
        return -1;
    }

    /* Get variable screen information */
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable screen info");
        close(fb_fd);
        return -1;
    }

    /* Get fixed screen information */
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed screen info");
        close(fb_fd);
        return -1;
    }

    /* Store screen dimensions */
    desktop_width = vinfo.xres;
    desktop_height = vinfo.yres;
    desktop_depth = vinfo.bits_per_pixel;

    /* Calculate framebuffer size */
    fb_size = finfo.line_length * vinfo.yres;

    /* Map framebuffer to memory */
    fb_mem = (char*)mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mem == MAP_FAILED) {
        perror("Error mapping framebuffer to memory");
        close(fb_fd);
        return -1;
    }

    printf("Framebuffer initialized: %dx%d, %d bpp\n", 
           desktop_width, desktop_height, desktop_depth);
    return 0;
}

/*
 * Free framebuffer resources
 */
static void cleanup_framebuffer(void) {
    if (fb_mem != NULL && fb_mem != MAP_FAILED) {
        munmap(fb_mem, fb_size);
        fb_mem = NULL;
    }

    if (fb_fd != -1) {
        close(fb_fd);
        fb_fd = -1;
    }
}

/*
 * Initialize input devices (keyboard, mouse)
 * Returns 0 on success, -1 on failure
 */
static int init_input_devices(void) {
    /* Input device initialization would go here */
    /* This includes keyboard and mouse handling */
    
    /* For simplicity, we're assuming success for now */
    printf("Input devices initialized\n");
    return 0;
}

/*
 * Initialize the AMOS desktop environment
 * This is the main entry point called by the kernel
 * Returns 0 on success, -1 on failure
 */
int desktop_init(void) {
    printf("Initializing AMOS Desktop Environment\n");

    /* Initialize framebuffer */
    if (init_framebuffer() != 0) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return -1;
    }

    /* Initialize input devices */
    if (init_input_devices() != 0) {
        fprintf(stderr, "Failed to initialize input devices\n");
        cleanup_framebuffer();
        return -1;
    }

    /* Initialize state manager */
    if (state_manager_init() != 0) {
        fprintf(stderr, "Failed to initialize state manager\n");
        cleanup_framebuffer();
        return -1;
    }
    
    /* Initialize UI toolkit */
    if (ui_toolkit_init(fb_mem, desktop_width, desktop_height, desktop_depth) != 0) {
        fprintf(stderr, "Failed to initialize UI toolkit\n");
        state_manager_cleanup();
        cleanup_framebuffer();
        return -1;
    }

    /* Initialize window manager */
    if (window_manager_init(fb_mem, desktop_width, desktop_height, desktop_depth) != 0) {
        fprintf(stderr, "Failed to initialize window manager\n");
        ui_toolkit_cleanup();
        state_manager_cleanup();
        cleanup_framebuffer();
        return -1;
    }

    /* Mark desktop as running */
    desktop_running = true;
    
    printf("AMOS Desktop Environment initialization complete\n");
    return 0;
}

/*
 * Start the desktop environment main loop
 * This function doesn't return until the desktop is shut down
 */
int desktop_run(void) {
    if (!desktop_running) {
        fprintf(stderr, "Cannot run desktop: not initialized\n");
        return -1;
    }

    printf("Starting AMOS Desktop Environment\n");

    /* Start the window manager main loop */
    window_manager_run();

    /* Clean up resources when window manager exits */
    window_manager_cleanup();
    ui_toolkit_cleanup();
    state_manager_cleanup();
    cleanup_framebuffer();
    
    desktop_running = false;
    
    printf("AMOS Desktop Environment shutdown complete\n");
    return 0;
}

/*
 * Shutdown the desktop environment
 * This can be called asynchronously to trigger desktop shutdown
 */
void desktop_shutdown(void) {
    if (desktop_running) {
        /* Signal the window manager to exit its main loop */
        window_manager_exit();
    }
}

/* 
 * Get current framebuffer information
 */
void desktop_get_fb_info(int* width, int* height, int* depth, char** buffer) {
    if (width != NULL) *width = desktop_width;
    if (height != NULL) *height = desktop_height;
    if (depth != NULL) *depth = desktop_depth;
    if (buffer != NULL) *buffer = fb_mem;
}