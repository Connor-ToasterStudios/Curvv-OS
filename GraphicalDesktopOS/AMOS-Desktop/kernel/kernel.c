/*
 * kernel.c - AMOS Kernel with Desktop Integration
 *
 * This file provides the main kernel entry point and desktop integration
 * for the AMOS operating system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../desktop/integration/desktop_init.h"
#include "drivers/framebuffer.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "mm/memory.h"
#include "fs/filesystem.h"
#include "process/process.h"

/* Kernel state */
static bool kernel_initialized = false;
static bool desktop_enabled = true;

/*
 * Initialize kernel subsystems
 * Returns 0 on success, non-zero on failure
 */
static int kernel_init_subsystems(void) {
    int status = 0;
    
    printf("Initializing kernel subsystems:\n");
    
    /* Initialize memory management */
    printf("- Memory management\n");
    status = mm_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize memory management\n");
        return status;
    }
    
    /* Initialize interrupt handling */
    printf("- Interrupt handling\n");
    status = interrupt_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize interrupt handling\n");
        mm_cleanup();
        return status;
    }
    
    /* Initialize device drivers */
    printf("- Device drivers\n");
    status = drivers_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize device drivers\n");
        interrupt_cleanup();
        mm_cleanup();
        return status;
    }
    
    /* Initialize file system */
    printf("- File system\n");
    status = fs_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize file system\n");
        drivers_cleanup();
        interrupt_cleanup();
        mm_cleanup();
        return status;
    }
    
    /* Initialize process management */
    printf("- Process management\n");
    status = process_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize process management\n");
        fs_cleanup();
        drivers_cleanup();
        interrupt_cleanup();
        mm_cleanup();
        return status;
    }
    
    /* Initialize graphics subsystem */
    printf("- Graphics subsystem\n");
    status = framebuffer_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize graphics subsystem\n");
        process_cleanup();
        fs_cleanup();
        drivers_cleanup();
        interrupt_cleanup();
        mm_cleanup();
        return status;
    }
    
    kernel_initialized = true;
    printf("Kernel subsystems initialized successfully\n");
    return 0;
}

/*
 * Clean up kernel subsystems
 */
static void kernel_cleanup_subsystems(void) {
    if (!kernel_initialized) {
        return;
    }
    
    printf("Cleaning up kernel subsystems\n");
    
    /* Clean up in reverse order of initialization */
    framebuffer_cleanup();
    process_cleanup();
    fs_cleanup();
    drivers_cleanup();
    interrupt_cleanup();
    mm_cleanup();
    
    kernel_initialized = false;
}

/*
 * Start desktop environment
 * Returns 0 on success, non-zero on failure
 */
static int kernel_start_desktop(void) {
    int status;
    
    if (!desktop_enabled) {
        printf("Desktop environment disabled\n");
        return 0;
    }
    
    printf("Starting desktop environment integration:\n");
    
    /* Initialize desktop environment */
    printf("- Configuring desktop environment\n");
    status = desktop_init();
    if (status != 0) {
        fprintf(stderr, "Failed to initialize desktop environment\n");
        return status;
    }
    
    /* Start desktop main loop */
    printf("- Starting desktop environment\n");
    status = desktop_run();
    if (status != 0) {
        fprintf(stderr, "Desktop environment exited with status %d\n", status);
    }
    
    return status;
}

/*
 * Kernel main entry point
 */
int kernel_main(void) {
    int status;
    
    printf("AMOS Kernel starting\n");
    
    /* Initialize kernel subsystems */
    status = kernel_init_subsystems();
    if (status != 0) {
        fprintf(stderr, "Kernel initialization failed\n");
        return status;
    }
    
    /* Start desktop environment if enabled */
    if (desktop_enabled) {
        status = kernel_start_desktop();
        if (status != 0) {
            fprintf(stderr, "Desktop environment failed\n");
        }
    }
    
    /* Clean up kernel subsystems */
    kernel_cleanup_subsystems();
    
    printf("AMOS Kernel shutting down\n");
    return status;
}

/*
 * Kernel panic function
 */
void kernel_panic(const char* message) {
    fprintf(stderr, "KERNEL PANIC: %s\n", message);
    
    /* In a real kernel, we would halt the CPU here */
    /* For now, we just exit */
    exit(1);
}

/*
 * Function stubs for kernel subsystems that we don't fully implement here
 */
int mm_init(void) { return 0; }
void mm_cleanup(void) {}

int interrupt_init(void) { return 0; }
void interrupt_cleanup(void) {}

int drivers_init(void) { return 0; }
void drivers_cleanup(void) {}

int fs_init(void) { return 0; }
void fs_cleanup(void) {}

int process_init(void) { return 0; }
void process_cleanup(void) {}

int framebuffer_init(void) { return 0; }
void framebuffer_cleanup(void) {}