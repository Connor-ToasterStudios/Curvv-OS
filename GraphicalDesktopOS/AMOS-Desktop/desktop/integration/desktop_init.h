/*
 * desktop_init.h - AMOS Desktop Environment Initialization Interface
 *
 * This header defines the public interface for initializing and
 * controlling the AMOS desktop environment.
 */

#ifndef AMOS_DESKTOP_INIT_H
#define AMOS_DESKTOP_INIT_H

#include <linux/fb.h>  /* For framebuffer structures */

/*
 * Initialize the AMOS desktop environment
 * This is the main entry point called by the kernel
 * Returns 0 on success, -1 on failure
 */
int desktop_init(void);

/*
 * Start the desktop environment main loop
 * This function doesn't return until the desktop is shut down
 * Returns 0 on success, -1 on failure
 */
int desktop_run(void);

/*
 * Shutdown the desktop environment
 * This can be called asynchronously to trigger desktop shutdown
 */
void desktop_shutdown(void);

/* 
 * Get current framebuffer information
 * Provides access to framebuffer dimensions and memory
 */
void desktop_get_fb_info(int* width, int* height, int* depth, char** buffer);

#endif /* AMOS_DESKTOP_INIT_H */