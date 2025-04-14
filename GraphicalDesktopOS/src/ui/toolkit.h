/*
 * toolkit.h - Lightweight UI toolkit for the window manager
 * 
 * This toolkit provides the basic UI elements and drawing primitives
 * needed by the window manager and applications.
 */

#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <X11/Xlib.h>
#include "themes.h"

// Toolkit initialization
void init_toolkit(Display *dpy, Window root_win, int scr);

// Process pending toolkit updates (animations, etc)
void process_toolkit_updates();

// Initialize the widget system
void init_widgets();

// Button widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *label;
    int state;  // 0=normal, 1=hover, 2=pressed
    void (*click_handler)(void *user_data);
    void *user_data;
} Button;

// Label widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *text;
    int alignment;  // 0=left, 1=center, 2=right
} Label;

// Text input widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *text;
    int cursor_pos;
    int selection_start;
    int selection_end;
    void (*change_handler)(void *user_data);
    void *user_data;
} TextInput;

// Checkbox widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *label;
    int checked;
    void (*change_handler)(int checked, void *user_data);
    void *user_data;
} Checkbox;

// Progress bar widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    int min_value;
    int max_value;
    int current_value;
} ProgressBar;

// Dropdown/combobox widget
typedef struct {
    Window window;
    Window list_window;
    int x, y;
    int width, height;
    char **items;
    int item_count;
    int selected_index;
    int is_open;
    void (*selection_handler)(int index, void *user_data);
    void *user_data;
} Dropdown;

// Slider widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    int min_value;
    int max_value;
    int current_value;
    int orientation;  // 0=horizontal, 1=vertical
    void (*change_handler)(int value, void *user_data);
    void *user_data;
} Slider;

// Tab control
typedef struct {
    Window window;
    Window *tab_windows;
    int x, y;
    int width, height;
    char **tab_labels;
    int tab_count;
    int active_tab;
    void (*tab_change_handler)(int index, void *user_data);
    void *user_data;
} TabControl;

// ListView widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char **items;
    int item_count;
    int selected_index;
    int top_index;
    int visible_items;
    void (*selection_handler)(int index, void *user_data);
    void *user_data;
} ListView;

// Dialog widget
typedef struct {
    Window window;
    int x, y;
    int width, height;
    char *title;
    int modal;
    void (*close_handler)(int result, void *user_data);
    void *user_data;
} Dialog;

// Widget creation functions
Button *create_button(Window parent, int x, int y, int width, int height, 
                     const char *label, void (*click_handler)(void *), void *user_data);

Label *create_label(Window parent, int x, int y, int width, int height, 
                   const char *text, int alignment);

TextInput *create_text_input(Window parent, int x, int y, int width, int height, 
                            const char *initial_text, 
                            void (*change_handler)(void *), void *user_data);

Checkbox *create_checkbox(Window parent, int x, int y, int width, int height, 
                         const char *label, int initial_state, 
                         void (*change_handler)(int, void *), void *user_data);

ProgressBar *create_progress_bar(Window parent, int x, int y, int width, int height, 
                               int min_value, int max_value, int initial_value);

Dropdown *create_dropdown(Window parent, int x, int y, int width, int height, 
                         char **items, int item_count, int initial_selection, 
                         void (*selection_handler)(int, void *), void *user_data);

Slider *create_slider(Window parent, int x, int y, int width, int height, 
                     int min_value, int max_value, int initial_value, int orientation, 
                     void (*change_handler)(int, void *), void *user_data);

TabControl *create_tab_control(Window parent, int x, int y, int width, int height, 
                              char **tab_labels, int tab_count, 
                              void (*tab_change_handler)(int, void *), void *user_data);

ListView *create_list_view(Window parent, int x, int y, int width, int height, 
                          char **items, int item_count, 
                          void (*selection_handler)(int, void *), void *user_data);

Dialog *create_dialog(Window parent, int x, int y, int width, int height, 
                     const char *title, int modal, 
                     void (*close_handler)(int, void *), void *user_data);

// Widget methods
void button_set_label(Button *button, const char *label);
void button_set_state(Button *button, int state);
void button_destroy(Button *button);

void label_set_text(Label *label, const char *text);
void label_set_alignment(Label *label, int alignment);
void label_destroy(Label *label);

void text_input_set_text(TextInput *input, const char *text);
const char *text_input_get_text(TextInput *input);
void text_input_select_all(TextInput *input);
void text_input_destroy(TextInput *input);

void checkbox_set_checked(Checkbox *checkbox, int checked);
int checkbox_is_checked(Checkbox *checkbox);
void checkbox_destroy(Checkbox *checkbox);

void progress_bar_set_value(ProgressBar *bar, int value);
int progress_bar_get_value(ProgressBar *bar);
void progress_bar_destroy(ProgressBar *bar);

void dropdown_set_selected(Dropdown *dropdown, int index);
int dropdown_get_selected(Dropdown *dropdown);
void dropdown_add_item(Dropdown *dropdown, const char *item);
void dropdown_remove_item(Dropdown *dropdown, int index);
void dropdown_destroy(Dropdown *dropdown);

void slider_set_value(Slider *slider, int value);
int slider_get_value(Slider *slider);
void slider_destroy(Slider *slider);

void tab_control_set_active(TabControl *tabs, int index);
int tab_control_get_active(TabControl *tabs);
Window tab_control_get_container(TabControl *tabs, int index);
void tab_control_destroy(TabControl *tabs);

void list_view_set_selected(ListView *list, int index);
int list_view_get_selected(ListView *list);
void list_view_add_item(ListView *list, const char *item);
void list_view_remove_item(ListView *list, int index);
void list_view_destroy(ListView *list);

int dialog_show_modal(Dialog *dialog);
void dialog_close(Dialog *dialog, int result);
void dialog_destroy(Dialog *dialog);

// Message dialogs
int show_message_dialog(Window parent, const char *title, const char *message, int buttons);
int show_error_dialog(Window parent, const char *title, const char *message);
int show_confirm_dialog(Window parent, const char *title, const char *message);
char *show_input_dialog(Window parent, const char *title, const char *prompt, const char *default_text);

// Drawing primitives
void draw_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height);
void draw_filled_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height);
void draw_rounded_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height, int radius);
void draw_filled_rounded_rectangle(Display *dpy, Window win, GC gc, int x, int y, int width, int height, int radius);
void draw_line(Display *dpy, Window win, GC gc, int x1, int y1, int x2, int y2);
void draw_text(Display *dpy, Window win, GC gc, int x, int y, const char *text, int alignment);
void draw_image(Display *dpy, Window win, GC gc, int x, int y, const char *xpm_data[]);

// Color utilities
unsigned long get_color(Display *dpy, const char *color_name);
GC create_gc(Display *dpy, Window win, unsigned long foreground, unsigned long background);
void free_gc(Display *dpy, GC gc);

// Event handling for widgets
int handle_widget_event(XEvent *event);

#endif /* TOOLKIT_H */
