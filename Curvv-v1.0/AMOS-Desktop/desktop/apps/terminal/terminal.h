#ifndef TERMINAL_APP_H
#define TERMINAL_APP_H

#include "../../ui/toolkit.h"
#include "../../wm/window.h"
#include "../../state/state_manager.h"

// Terminal application structure
typedef struct {
    Window* window;         // Window for this terminal
    char* buffer;           // Text buffer
    int cursor_x;           // Cursor X position
    int cursor_y;           // Cursor Y position
    int rows;               // Number of rows
    int cols;               // Number of columns
    int scroll_offset;      // Scroll position
    int history_size;       // Command history size
    char** history;         // Command history
    int history_index;      // Current history index
    StateID state_id;       // State identifier
} TerminalApp;

// Initialize the terminal application
TerminalApp* terminal_init(int x, int y, int width, int height);

// Process keyboard input
void terminal_handle_key(TerminalApp* term, int key);

// Draw the terminal contents
void terminal_draw(TerminalApp* term);

// Execute a command
void terminal_execute_command(TerminalApp* term, const char* command);

// Resize terminal window
void terminal_resize(TerminalApp* term, int width, int height);

// Save terminal state
void terminal_save_state(TerminalApp* term);

// Load terminal state
void terminal_load_state(TerminalApp* term);

// Cleanup terminal resources
void terminal_cleanup(TerminalApp* term);

#endif /* TERMINAL_APP_H */