/*
 * ui_toolkit.h - AMOS Desktop UI Toolkit Interface
 *
 * This header defines the public interface for the UI toolkit
 * component of the AMOS desktop environment. It provides widgets
 * and drawing primitives for applications and window manager.
 */

#ifndef AMOS_UI_TOOLKIT_H
#define AMOS_UI_TOOLKIT_H

#include <stdbool.h>

/* Color structure */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} ui_color_t;

/* Rectangle structure */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} ui_rect_t;

/* Text alignment options */
typedef enum {
    UI_ALIGN_LEFT,
    UI_ALIGN_CENTER,
    UI_ALIGN_RIGHT
} ui_text_align_t;

/* Widget types */
typedef enum {
    UI_WIDGET_NONE,
    UI_WIDGET_BUTTON,
    UI_WIDGET_LABEL,
    UI_WIDGET_TEXTBOX,
    UI_WIDGET_CHECKBOX,
    UI_WIDGET_RADIO,
    UI_WIDGET_SLIDER,
    UI_WIDGET_PROGRESS,
    UI_WIDGET_LIST,
    UI_WIDGET_DROPDOWN,
    UI_WIDGET_SCROLLBAR,
    UI_WIDGET_PANEL,
    UI_WIDGET_TABCONTROL,
    UI_WIDGET_MENUITEM
} ui_widget_type_t;

/* Forward declarations */
struct ui_widget;
struct ui_context;

/* UI event types */
typedef enum {
    UI_EVENT_NONE,
    UI_EVENT_CLICK,
    UI_EVENT_DOUBLECLICK,
    UI_EVENT_PRESS,
    UI_EVENT_RELEASE,
    UI_EVENT_ENTER,
    UI_EVENT_LEAVE,
    UI_EVENT_CHANGE,
    UI_EVENT_KEYPRESS,
    UI_EVENT_KEYRELEASE,
    UI_EVENT_FOCUS,
    UI_EVENT_BLUR
} ui_event_type_t;

/* UI event structure */
typedef struct {
    ui_event_type_t type;
    struct ui_widget* widget;
    int x;
    int y;
    unsigned int key;
    unsigned int state;
} ui_event_t;

/* Widget event handler function type */
typedef bool (*ui_event_handler_t)(struct ui_widget* widget, ui_event_t* event, void* user_data);

/*
 * Initialize the UI toolkit
 * fb_mem: Pointer to framebuffer memory
 * width: Screen width
 * height: Screen height
 * depth: Color depth in bits per pixel
 * Returns 0 on success, -1 on failure
 */
int ui_toolkit_init(char* fb_mem, int width, int height, int depth);

/*
 * Clean up UI toolkit resources
 */
void ui_toolkit_cleanup(void);

/*
 * Create a new UI context for drawing
 * buffer: Pointer to drawing buffer
 * width: Buffer width
 * height: Buffer height
 * depth: Color depth in bits per pixel
 * Returns UI context handle on success, NULL on failure
 */
struct ui_context* ui_context_create(char* buffer, int width, int height, int depth);

/*
 * Destroy a UI context
 */
void ui_context_destroy(struct ui_context* ctx);

/*
 * Clear UI context with specified color
 */
void ui_clear(struct ui_context* ctx, ui_color_t color);

/*
 * Draw a pixel
 */
void ui_draw_pixel(struct ui_context* ctx, int x, int y, ui_color_t color);

/*
 * Draw a line
 */
void ui_draw_line(struct ui_context* ctx, int x1, int y1, int x2, int y2, ui_color_t color);

/*
 * Draw a rectangle
 */
void ui_draw_rect(struct ui_context* ctx, ui_rect_t rect, ui_color_t color, bool filled);

/*
 * Draw a rounded rectangle
 */
void ui_draw_rounded_rect(struct ui_context* ctx, ui_rect_t rect, int radius, ui_color_t color, bool filled);

/*
 * Draw a circle
 */
void ui_draw_circle(struct ui_context* ctx, int x, int y, int radius, ui_color_t color, bool filled);

/*
 * Draw text
 */
void ui_draw_text(struct ui_context* ctx, int x, int y, const char* text, ui_color_t color, ui_text_align_t align);

/*
 * Draw an image
 * format: Image format (e.g., "RGB", "RGBA")
 * data: Image data
 */
void ui_draw_image(struct ui_context* ctx, int x, int y, int width, int height, const char* format, const void* data);

/*
 * Create a button widget
 */
struct ui_widget* ui_button_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text);

/*
 * Create a label widget
 */
struct ui_widget* ui_label_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text);

/*
 * Create a textbox widget
 */
struct ui_widget* ui_textbox_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text);

/*
 * Create a checkbox widget
 */
struct ui_widget* ui_checkbox_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text, bool checked);

/*
 * Create a radio button widget
 */
struct ui_widget* ui_radio_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text, bool selected);

/*
 * Create a slider widget
 */
struct ui_widget* ui_slider_create(struct ui_context* ctx, int x, int y, int width, int height, int min_value, int max_value, int value);

/*
 * Create a progress bar widget
 */
struct ui_widget* ui_progress_create(struct ui_context* ctx, int x, int y, int width, int height, int min_value, int max_value, int value);

/*
 * Create a list widget
 */
struct ui_widget* ui_list_create(struct ui_context* ctx, int x, int y, int width, int height);

/*
 * Create a dropdown widget
 */
struct ui_widget* ui_dropdown_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text);

/*
 * Create a scrollbar widget
 */
struct ui_widget* ui_scrollbar_create(struct ui_context* ctx, int x, int y, int width, int height, int min_value, int max_value, int value);

/*
 * Create a panel widget
 */
struct ui_widget* ui_panel_create(struct ui_context* ctx, int x, int y, int width, int height);

/*
 * Create a tab control widget
 */
struct ui_widget* ui_tabcontrol_create(struct ui_context* ctx, int x, int y, int width, int height);

/*
 * Create a menu item widget
 */
struct ui_widget* ui_menuitem_create(struct ui_context* ctx, int x, int y, int width, int height, const char* text);

/*
 * Destroy a widget
 */
void ui_widget_destroy(struct ui_widget* widget);

/*
 * Set widget position
 */
void ui_widget_set_position(struct ui_widget* widget, int x, int y);

/*
 * Set widget size
 */
void ui_widget_set_size(struct ui_widget* widget, int width, int height);

/*
 * Set widget text
 */
void ui_widget_set_text(struct ui_widget* widget, const char* text);

/*
 * Set widget enabled state
 */
void ui_widget_set_enabled(struct ui_widget* widget, bool enabled);

/*
 * Set widget visible state
 */
void ui_widget_set_visible(struct ui_widget* widget, bool visible);

/*
 * Set widget event handler
 */
void ui_widget_set_event_handler(struct ui_widget* widget, ui_event_handler_t handler, void* user_data);

/*
 * Set widget value (for widgets that have a value)
 */
void ui_widget_set_value(struct ui_widget* widget, int value);

/*
 * Get widget value (for widgets that have a value)
 */
int ui_widget_get_value(struct ui_widget* widget);

/*
 * Draw a widget
 */
void ui_widget_draw(struct ui_widget* widget);

/*
 * Process mouse input for a widget
 */
bool ui_widget_process_mouse(struct ui_widget* widget, int x, int y, int buttons);

/*
 * Process keyboard input for a widget
 */
bool ui_widget_process_key(struct ui_widget* widget, unsigned int key, bool pressed);

/*
 * Check if a point is inside a widget
 */
bool ui_widget_contains_point(struct ui_widget* widget, int x, int y);

/*
 * Get widget rectangle
 */
ui_rect_t ui_widget_get_rect(struct ui_widget* widget);

/*
 * Get widget type
 */
ui_widget_type_t ui_widget_get_type(struct ui_widget* widget);

/*
 * Create theme-specified colors
 */
ui_color_t ui_color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

/*
 * Create theme-specified colors from RGB value
 */
ui_color_t ui_color_from_rgb(unsigned int rgb);

/*
 * Set global theme colors
 */
void ui_set_theme_colors(ui_color_t background, ui_color_t foreground, ui_color_t accent, ui_color_t highlight);

/*
 * Common predefined colors
 */
ui_color_t ui_color_black(void);
ui_color_t ui_color_white(void);
ui_color_t ui_color_red(void);
ui_color_t ui_color_green(void);
ui_color_t ui_color_blue(void);
ui_color_t ui_color_yellow(void);
ui_color_t ui_color_cyan(void);
ui_color_t ui_color_magenta(void);
ui_color_t ui_color_transparent(void);

#endif /* AMOS_UI_TOOLKIT_H */