/*
 * state_manager.c - State management implementation for window persistence
 *
 * This module provides functionality to store and retrieve window state,
 * as well as application-specific data, allowing for persistence between
 * sessions or when windows are redrawn.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <X11/Xlib.h>

#include "state_manager.h"
#include "../wm/window.h"

#define MAX_WINDOWS 128
#define STATE_DIR "/var/amos/state"
#define WINDOW_STATE_FILE STATE_DIR "/windows.state"
#define APP_STATE_DIR STATE_DIR "/app_state"

// In-memory representation of window states
static WindowState window_states[MAX_WINDOWS];
static int window_state_count = 0;

// Display connection - set during initialization
static Display *display = NULL;

// Forward declarations
static void ensure_state_dirs_exist();
static int find_window_state_index(Window win);
static void read_window_state_file();
static void write_window_state_file();

// Initialize the state manager
void init_state_manager() {
    printf("Initializing state manager...\n");
    
    // Get the display connection from the WM
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "State manager: cannot connect to X display\n");
        return;
    }
    
    // Make sure state directories exist
    ensure_state_dirs_exist();
    
    // Initialize window state array
    for (int i = 0; i < MAX_WINDOWS; i++) {
        window_states[i].window = None;
        window_states[i].title = NULL;
        window_states[i].application_state = NULL;
        window_states[i].app_state_size = 0;
    }
    
    // Load saved states
    read_window_state_file();
    
    printf("State manager initialized\n");
}

// Ensure state directories exist
static void ensure_state_dirs_exist() {
    struct stat st = {0};
    
    // Create main state directory
    if (stat(STATE_DIR, &st) == -1) {
        if (mkdir(STATE_DIR, 0755) == -1) {
            fprintf(stderr, "Failed to create state directory: %s\n", strerror(errno));
        }
    }
    
    // Create app state directory
    if (stat(APP_STATE_DIR, &st) == -1) {
        if (mkdir(APP_STATE_DIR, 0755) == -1) {
            fprintf(stderr, "Failed to create app state directory: %s\n", strerror(errno));
        }
    }
}

// Add a window to state management
void add_window_state(Window win) {
    if (window_state_count >= MAX_WINDOWS) {
        fprintf(stderr, "Maximum number of window states reached\n");
        return;
    }
    
    // Check if this window is already tracked
    for (int i = 0; i < window_state_count; i++) {
        if (window_states[i].window == win) {
            return;  // Already tracked
        }
    }
    
    // Add the window to our state tracking
    int idx = window_state_count++;
    WindowState *state = &window_states[idx];
    
    // Initialize with default values
    state->window = win;
    state->x = 0;
    state->y = 0;
    state->width = 300;
    state->height = 200;
    state->is_minimized = 0;
    state->is_maximized = 0;
    state->workspace = 0;
    state->tab_group = -1;
    state->tab_index = -1;
    state->title = NULL;
    state->application_state = NULL;
    state->app_state_size = 0;
    
    // Get the window's current geometry
    XWindowAttributes attr;
    if (XGetWindowAttributes(display, win, &attr)) {
        state->width = attr.width;
        state->height = attr.height;
        state->x = attr.x;
        state->y = attr.y;
    }
    
    // Get the window title
    XTextProperty text_prop;
    if (XGetWMName(display, win, &text_prop) && text_prop.value) {
        state->title = strdup((char *)text_prop.value);
        XFree(text_prop.value);
    } else {
        state->title = strdup("Untitled");
    }
    
    printf("Added window state for %lu: %s (%d,%d) %dx%d\n", 
           win, state->title, state->x, state->y, state->width, state->height);
}

// Remove a window from state management
void remove_window_state(Window win) {
    int idx = find_window_state_index(win);
    if (idx == -1) {
        return;  // Not found
    }
    
    // Free resources
    if (window_states[idx].title) {
        free(window_states[idx].title);
    }
    
    if (window_states[idx].application_state) {
        free(window_states[idx].application_state);
    }
    
    // Remove app state file if it exists
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%lu.state", APP_STATE_DIR, win);
    unlink(filename);
    
    // Remove the state by shifting everything down
    for (int i = idx; i < window_state_count - 1; i++) {
        window_states[i] = window_states[i + 1];
    }
    
    window_state_count--;
    
    printf("Removed window state for %lu\n", win);
}

// Find the index of a window in our state array
static int find_window_state_index(Window win) {
    for (int i = 0; i < window_state_count; i++) {
        if (window_states[i].window == win) {
            return i;
        }
    }
    return -1;
}

// Save the state of a window
void save_window_state(Window win) {
    int idx = find_window_state_index(win);
    if (idx == -1) {
        add_window_state(win);
        idx = find_window_state_index(win);
        if (idx == -1) return;  // Failed to add
    }
    
    // Get the window's current geometry
    XWindowAttributes attr;
    if (XGetWindowAttributes(display, win, &attr)) {
        window_states[idx].width = attr.width;
        window_states[idx].height = attr.height;
        window_states[idx].x = attr.x;
        window_states[idx].y = attr.y;
    }
    
    // Get the window title
    XTextProperty text_prop;
    if (XGetWMName(display, win, &text_prop) && text_prop.value) {
        if (window_states[idx].title) {
            free(window_states[idx].title);
        }
        window_states[idx].title = strdup((char *)text_prop.value);
        XFree(text_prop.value);
    }
    
    // Get state from the window structure (minimized, maximized, etc.)
    WMWindow *w = find_window(win);
    if (w) {
        window_states[idx].is_minimized = w->is_minimized;
        window_states[idx].is_fullscreen = w->is_fullscreen;
        window_states[idx].tab_group = w->group_id;
        window_states[idx].tab_index = w->tab_id;
    }
    
    // Write to disk
    write_window_state_file();
}

// Get the state of a window
int get_window_state(Window win, WindowState *state) {
    int idx = find_window_state_index(win);
    if (idx == -1) {
        return 0;  // Not found
    }
    
    // Copy the state
    *state = window_states[idx];
    
    return 1;  // Success
}

// Save all window states
void save_all_window_states() {
    // This will write all window states to disk
    write_window_state_file();
    
    printf("Saved %d window states\n", window_state_count);
}

// Read window states from file
static void read_window_state_file() {
    FILE *f = fopen(WINDOW_STATE_FILE, "r");
    if (!f) {
        fprintf(stderr, "Could not open window state file for reading: %s\n", strerror(errno));
        return;
    }
    
    // Read window count
    if (fscanf(f, "%d\n", &window_state_count) != 1) {
        fprintf(stderr, "Failed to read window count from state file\n");
        fclose(f);
        return;
    }
    
    // Make sure we don't overflow
    if (window_state_count > MAX_WINDOWS) {
        fprintf(stderr, "Warning: truncating window state count from %d to %d\n", 
                window_state_count, MAX_WINDOWS);
        window_state_count = MAX_WINDOWS;
    }
    
    // Read each window state
    for (int i = 0; i < window_state_count; i++) {
        WindowState *state = &window_states[i];
        
        // Format: window_id x y width height is_min is_max workspace tab_group tab_index title
        unsigned long win_id;
        if (fscanf(f, "%lu %d %d %d %d %d %d %d %d %d ", 
                &win_id, &state->x, &state->y, &state->width, &state->height,
                &state->is_minimized, &state->is_maximized,
                &state->workspace, &state->tab_group, &state->tab_index) != 10) {
            fprintf(stderr, "Failed to read window state %d\n", i);
            break;
        }
        
        // Convert the window ID to a real window
        state->window = win_id;
        
        // Read the title (rest of line)
        char title_buf[256];
        if (fgets(title_buf, sizeof(title_buf), f)) {
            // Remove newline
            int len = strlen(title_buf);
            if (len > 0 && title_buf[len-1] == '\n') {
                title_buf[len-1] = '\0';
            }
            
            state->title = strdup(title_buf);
        } else {
            state->title = strdup("Untitled");
        }
        
        state->application_state = NULL;
        state->app_state_size = 0;
    }
    
    fclose(f);
    printf("Loaded %d window states\n", window_state_count);
}

// Write window states to file
static void write_window_state_file() {
    FILE *f = fopen(WINDOW_STATE_FILE, "w");
    if (!f) {
        fprintf(stderr, "Could not open window state file for writing: %s\n", strerror(errno));
        return;
    }
    
    // Write window count
    fprintf(f, "%d\n", window_state_count);
    
    // Write each window state
    for (int i = 0; i < window_state_count; i++) {
        WindowState *state = &window_states[i];
        
        // Format: window_id x y width height is_min is_max workspace tab_group tab_index title
        fprintf(f, "%lu %d %d %d %d %d %d %d %d %d %s\n", 
               state->window, state->x, state->y, state->width, state->height,
               state->is_minimized, state->is_maximized,
               state->workspace, state->tab_group, state->tab_index,
               state->title ? state->title : "Untitled");
    }
    
    fclose(f);
}

// Load window states
void load_window_states() {
    read_window_state_file();
}

// Store application-specific state
void store_application_state(Window win, void *data, size_t size) {
    int idx = find_window_state_index(win);
    if (idx == -1) {
        add_window_state(win);
        idx = find_window_state_index(win);
        if (idx == -1) return;  // Failed to add
    }
    
    // Free old state if it exists
    if (window_states[idx].application_state) {
        free(window_states[idx].application_state);
        window_states[idx].application_state = NULL;
        window_states[idx].app_state_size = 0;
    }
    
    // Allocate and store new state
    if (data && size > 0) {
        window_states[idx].application_state = malloc(size);
        if (window_states[idx].application_state) {
            memcpy(window_states[idx].application_state, data, size);
            window_states[idx].app_state_size = size;
        }
    }
    
    // Write to app state file
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%lu.state", APP_STATE_DIR, win);
    
    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(&size, sizeof(size_t), 1, f);
        if (data && size > 0) {
            fwrite(data, 1, size, f);
        }
        fclose(f);
    } else {
        fprintf(stderr, "Failed to write app state file for window %lu: %s\n", 
                win, strerror(errno));
    }
}

// Retrieve application-specific state
void *retrieve_application_state(Window win, size_t *size) {
    int idx = find_window_state_index(win);
    if (idx == -1) {
        // Try to read from file
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%lu.state", APP_STATE_DIR, win);
        
        FILE *f = fopen(filename, "rb");
        if (!f) {
            if (size) *size = 0;
            return NULL;
        }
        
        // Read size
        size_t data_size;
        if (fread(&data_size, sizeof(size_t), 1, f) != 1) {
            fclose(f);
            if (size) *size = 0;
            return NULL;
        }
        
        // Allocate and read data
        void *data = NULL;
        if (data_size > 0) {
            data = malloc(data_size);
            if (data) {
                if (fread(data, 1, data_size, f) != data_size) {
                    free(data);
                    data = NULL;
                    data_size = 0;
                }
            }
        }
        
        fclose(f);
        
        if (size) *size = data_size;
        return data;
    }
    
    // Return in-memory state
    if (size) *size = window_states[idx].app_state_size;
    
    if (window_states[idx].application_state && window_states[idx].app_state_size > 0) {
        // Return a copy of the state
        void *copy = malloc(window_states[idx].app_state_size);
        if (copy) {
            memcpy(copy, window_states[idx].application_state, window_states[idx].app_state_size);
            return copy;
        }
    }
    
    return NULL;
}
