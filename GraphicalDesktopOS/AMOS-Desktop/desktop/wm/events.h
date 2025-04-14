/*
 * events.h - Event handling for the window manager
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <X11/Xlib.h>

// Set up event handlers
void setup_event_handlers();

// X error handler
int error_handler(Display *display, XErrorEvent *event);

// Handle events
void handle_window_destroy(Window w);
void handle_configure_request(XConfigureRequestEvent *e);
void handle_map_request(Window w);
void handle_button_press(XButtonEvent *e);
void handle_button_release(XButtonEvent *e);
void handle_motion_notify(XMotionEvent *e);
void handle_key_press(XKeyEvent *e);
void handle_enter_window(Window w);
void handle_client_message(XClientMessageEvent *e);

#endif /* EVENTS_H */
