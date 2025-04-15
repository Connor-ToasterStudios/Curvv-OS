#include "terminal.h"
#include "../../../kernel/utils.h"
#include <stdlib.h>
#include <string.h>

// Default terminal dimensions
#define DEFAULT_ROWS 24
#define DEFAULT_COLS 80
#define MAX_COMMAND_LENGTH 1024
#define HISTORY_SIZE 100

// Terminal color definitions
#define TERM_BG_COLOR 0x000000  // Black
#define TERM_FG_COLOR 0xAAAAAA  // Light gray
#define CURSOR_COLOR  0xFFFFFF  // White

// Internal function declarations
static void terminal_draw_cursor(TerminalApp* term);
static void terminal_scroll(TerminalApp* term, int lines);
static void terminal_add_to_history(TerminalApp* term, const char* command);
static void terminal_print(TerminalApp* term, const char* text);

// Initialize the terminal application
TerminalApp* terminal_init(int x, int y, int width, int height) {
    TerminalApp* term = (TerminalApp*)malloc(sizeof(TerminalApp));
    if (!term) return NULL;
    
    // Calculate rows and columns based on dimensions
    term->rows = height / 16;  // Assuming 16px character height
    term->cols = width / 8;    // Assuming 8px character width
    
    // Create window
    term->window = create_window("Terminal", x, y, width, height);
    
    // Allocate buffer
    int buffer_size = term->rows * term->cols;
    term->buffer = (char*)malloc(buffer_size);
    if (!term->buffer) {
        free(term);
        return NULL;
    }
    memset(term->buffer, ' ', buffer_size);
    
    // Initialize cursor position
    term->cursor_x = 0;
    term->cursor_y = 0;
    term->scroll_offset = 0;
    
    // Allocate command history
    term->history_size = HISTORY_SIZE;
    term->history = (char**)malloc(sizeof(char*) * HISTORY_SIZE);
    if (!term->history) {
        free(term->buffer);
        free(term);
        return NULL;
    }
    
    for (int i = 0; i < HISTORY_SIZE; i++) {
        term->history[i] = NULL;
    }
    term->history_index = 0;
    
    // Register with state manager
    term->state_id = register_state("terminal");
    
    // Load previous state if available
    terminal_load_state(term);
    
    // Print welcome message
    terminal_print(term, "AMOS Desktop OS Terminal\n");
    terminal_print(term, "Type 'help' for a list of commands\n");
    terminal_print(term, "\n$ ");
    
    return term;
}

// Process keyboard input
void terminal_handle_key(TerminalApp* term, int key) {
    // Process key based on value
    switch (key) {
        case '\n':  // Enter key
            // Get the current command
            char command[MAX_COMMAND_LENGTH];
            int line_start = term->cols * term->cursor_y;
            int cmd_start = line_start + 2;  // Skip the "$ "
            
            int cmd_len = term->cursor_x - 2;
            if (cmd_len > 0 && cmd_len < MAX_COMMAND_LENGTH) {
                strncpy(command, &term->buffer[cmd_start], cmd_len);
                command[cmd_len] = '\0';
                
                // Add command to history
                terminal_add_to_history(term, command);
                
                // Execute the command
                terminal_print(term, "\n");
                terminal_execute_command(term, command);
                terminal_print(term, "\n$ ");
            } else {
                terminal_print(term, "\n$ ");
            }
            break;
            
        case '\b':  // Backspace
            if (term->cursor_x > 2) {  // Don't erase the prompt
                term->cursor_x--;
                // Shift characters left
                int pos = term->cursor_y * term->cols + term->cursor_x;
                memmove(&term->buffer[pos], &term->buffer[pos + 1], 
                        term->cols - term->cursor_x - 1);
                term->buffer[term->cursor_y * term->cols + term->cols - 1] = ' ';
            }
            break;
            
        default:
            if (key >= 32 && key <= 126) {  // Printable ASCII
                // Insert character at cursor
                int pos = term->cursor_y * term->cols + term->cursor_x;
                
                // Shift characters right
                memmove(&term->buffer[pos + 1], &term->buffer[pos], 
                        term->cols - term->cursor_x - 1);
                
                // Insert the new character
                term->buffer[pos] = key;
                term->cursor_x++;
                
                // Handle wrapping
                if (term->cursor_x >= term->cols) {
                    term->cursor_x = 0;
                    term->cursor_y++;
                    if (term->cursor_y >= term->rows) {
                        terminal_scroll(term, 1);
                        term->cursor_y = term->rows - 1;
                    }
                }
            }
            break;
    }
    
    // Redraw the terminal
    terminal_draw(term);
}

// Draw the terminal contents
void terminal_draw(TerminalApp* term) {
    // Clear the window
    fill_window(term->window, TERM_BG_COLOR);
    
    // Draw each character in the buffer
    for (int y = 0; y < term->rows; y++) {
        for (int x = 0; x < term->cols; x++) {
            char c = term->buffer[y * term->cols + x];
            if (c >= 32 && c <= 126) {  // Printable ASCII
                // Draw character at position
                draw_char(term->window, x * 8, y * 16, c, TERM_FG_COLOR);
            }
        }
    }
    
    // Draw cursor
    terminal_draw_cursor(term);
    
    // Update window
    update_window(term->window);
}

// Execute a command
void terminal_execute_command(TerminalApp* term, const char* command) {
    if (strcmp(command, "help") == 0) {
        terminal_print(term, "Available commands:\n");
        terminal_print(term, "  help     - Display this help message\n");
        terminal_print(term, "  clear    - Clear the terminal screen\n");
        terminal_print(term, "  echo     - Display text\n");
        terminal_print(term, "  version  - Display AMOS version\n");
        terminal_print(term, "  exit     - Close the terminal\n");
    }
    else if (strcmp(command, "clear") == 0) {
        // Clear the buffer
        memset(term->buffer, ' ', term->rows * term->cols);
        term->cursor_x = 0;
        term->cursor_y = 0;
    }
    else if (strncmp(command, "echo ", 5) == 0) {
        // Echo the text after "echo "
        terminal_print(term, command + 5);
        terminal_print(term, "\n");
    }
    else if (strcmp(command, "version") == 0) {
        terminal_print(term, "AMOS Desktop OS v1.0\n");
        terminal_print(term, "Copyright (c) 2025 AMOS Team\n");
    }
    else if (strcmp(command, "exit") == 0) {
        // Save state before exiting
        terminal_save_state(term);
        // Close the window
        close_window(term->window);
    }
    else if (strlen(command) > 0) {
        terminal_print(term, "Unknown command: ");
        terminal_print(term, command);
        terminal_print(term, "\n");
    }
}

// Resize terminal window
void terminal_resize(TerminalApp* term, int width, int height) {
    // Save old dimensions
    int old_rows = term->rows;
    int old_cols = term->cols;
    
    // Calculate new dimensions
    term->rows = height / 16;
    term->cols = width / 8;
    
    // Allocate new buffer
    char* new_buffer = (char*)malloc(term->rows * term->cols);
    if (!new_buffer) return;
    
    // Initialize new buffer
    memset(new_buffer, ' ', term->rows * term->cols);
    
    // Copy old content
    int copy_rows = (old_rows < term->rows) ? old_rows : term->rows;
    int copy_cols = (old_cols < term->cols) ? old_cols : term->cols;
    
    for (int y = 0; y < copy_rows; y++) {
        for (int x = 0; x < copy_cols; x++) {
            new_buffer[y * term->cols + x] = term->buffer[y * old_cols + x];
        }
    }
    
    // Free old buffer
    free(term->buffer);
    term->buffer = new_buffer;
    
    // Adjust cursor if needed
    if (term->cursor_x >= term->cols) {
        term->cursor_x = term->cols - 1;
    }
    if (term->cursor_y >= term->rows) {
        term->cursor_y = term->rows - 1;
    }
    
    // Resize window
    resize_window(term->window, width, height);
    
    // Redraw
    terminal_draw(term);
}

// Save terminal state
void terminal_save_state(TerminalApp* term) {
    // Create state data
    StateData state_data;
    
    // Save window position and size
    state_data.add_int(term->state_id, "window_x", term->window->x);
    state_data.add_int(term->state_id, "window_y", term->window->y);
    state_data.add_int(term->state_id, "window_width", term->window->width);
    state_data.add_int(term->state_id, "window_height", term->window->height);
    
    // Save buffer content (just the visible part)
    state_data.add_string(term->state_id, "buffer", term->buffer);
    
    // Save cursor position
    state_data.add_int(term->state_id, "cursor_x", term->cursor_x);
    state_data.add_int(term->state_id, "cursor_y", term->cursor_y);
    
    // Save state
    save_state_data(term->state_id, &state_data);
}

// Load terminal state
void terminal_load_state(TerminalApp* term) {
    // Get state data
    StateData state_data;
    if (load_state_data(term->state_id, &state_data)) {
        // Get window position and size
        int x = state_data.get_int(term->state_id, "window_x", term->window->x);
        int y = state_data.get_int(term->state_id, "window_y", term->window->y);
        int width = state_data.get_int(term->state_id, "window_width", term->window->width);
        int height = state_data.get_int(term->state_id, "window_height", term->window->height);
        
        // Restore window
        move_window(term->window, x, y);
        resize_window(term->window, width, height);
        
        // Restore buffer content
        char* buffer = state_data.get_string(term->state_id, "buffer", "");
        if (strlen(buffer) > 0) {
            strncpy(term->buffer, buffer, term->rows * term->cols);
        }
        
        // Restore cursor position
        term->cursor_x = state_data.get_int(term->state_id, "cursor_x", 0);
        term->cursor_y = state_data.get_int(term->state_id, "cursor_y", 0);
    }
}

// Cleanup terminal resources
void terminal_cleanup(TerminalApp* term) {
    if (!term) return;
    
    // Save state before cleanup
    terminal_save_state(term);
    
    // Free buffer
    if (term->buffer) {
        free(term->buffer);
    }
    
    // Free history
    if (term->history) {
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (term->history[i]) {
                free(term->history[i]);
            }
        }
        free(term->history);
    }
    
    // Cleanup window
    if (term->window) {
        close_window(term->window);
    }
    
    // Free terminal structure
    free(term);
}

// Internal function to draw the cursor
static void terminal_draw_cursor(TerminalApp* term) {
    // Draw cursor as a solid block
    int cursor_x = term->cursor_x * 8;
    int cursor_y = term->cursor_y * 16;
    
    // Draw cursor rectangle
    fill_rect(term->window, cursor_x, cursor_y, 8, 16, CURSOR_COLOR);
    
    // Draw character at cursor (inverted color)
    char c = term->buffer[term->cursor_y * term->cols + term->cursor_x];
    if (c >= 32 && c <= 126) {
        draw_char(term->window, cursor_x, cursor_y, c, TERM_BG_COLOR);
    }
}

// Internal function to scroll the terminal
static void terminal_scroll(TerminalApp* term, int lines) {
    if (lines <= 0) return;
    
    // Move content up
    for (int y = 0; y < term->rows - lines; y++) {
        for (int x = 0; x < term->cols; x++) {
            term->buffer[y * term->cols + x] = 
                term->buffer[(y + lines) * term->cols + x];
        }
    }
    
    // Clear the new lines
    for (int y = term->rows - lines; y < term->rows; y++) {
        for (int x = 0; x < term->cols; x++) {
            term->buffer[y * term->cols + x] = ' ';
        }
    }
    
    term->scroll_offset += lines;
}

// Internal function to add command to history
static void terminal_add_to_history(TerminalApp* term, const char* command) {
    // Don't add empty commands
    if (strlen(command) == 0) return;
    
    // Don't add if same as last command
    if (term->history[0] && strcmp(term->history[0], command) == 0) return;
    
    // Free the last entry if full
    if (term->history[term->history_size - 1]) {
        free(term->history[term->history_size - 1]);
    }
    
    // Shift history down
    for (int i = term->history_size - 1; i > 0; i--) {
        term->history[i] = term->history[i - 1];
    }
    
    // Add new command at the beginning
    term->history[0] = strdup(command);
    term->history_index = 0;
}

// Internal function to print text to terminal
static void terminal_print(TerminalApp* term, const char* text) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            // Move to the beginning of the next line
            term->cursor_x = 0;
            term->cursor_y++;
            
            // Scroll if needed
            if (term->cursor_y >= term->rows) {
                terminal_scroll(term, 1);
                term->cursor_y = term->rows - 1;
            }
        } else {
            // Put character at cursor position
            term->buffer[term->cursor_y * term->cols + term->cursor_x] = text[i];
            term->cursor_x++;
            
            // Handle wrapping
            if (term->cursor_x >= term->cols) {
                term->cursor_x = 0;
                term->cursor_y++;
                if (term->cursor_y >= term->rows) {
                    terminal_scroll(term, 1);
                    term->cursor_y = term->rows - 1;
                }
            }
        }
    }
}