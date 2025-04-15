/*
 * state_manager.h - State management for window persistence
 */

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <X11/Xlib.h>

// Window state structure
typedef struct {
    Window window;
    int x, y;
    int width, height;
    int is_minimized;
    int is_maximized;
    int workspace;
    int tab_group;
    int tab_index;
    char *title;
    void *application_state;
    size_t app_state_size;
} WindowState;

// Initialize the state manager
void init_state_manager();

// Add a window to state management
void add_window_state(Window win);

// Remove a window from state management
void remove_window_state(Window win);

// Save the state of a window
void save_window_state(Window win);

// Get the state of a window
int get_window_state(Window win, WindowState *state);

// Save all window states
void save_all_window_states();

// Load window states
void load_window_states();

// Store application-specific state
void store_application_state(Window win, void *data, size_t size);

// Retrieve application-specific state
void *retrieve_application_state(Window win, size_t *size);

#endif /* STATE_MANAGER_H */
