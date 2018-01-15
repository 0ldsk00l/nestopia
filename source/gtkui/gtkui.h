#ifndef _GTKUI_H_
#define _GTKUI_H_

#include <gtk/gtk.h>

void gtkui_init(int argc, char *argv[]);
void gtkui_emuloop_start();
void gtkui_emuloop_stop();
void gtkui_create();
void gtkui_resize();
void gtkui_set_title(const char *title);
GtkWidget *gtkui_about();
void gtkui_image_paths();
void gtkui_message(const char* message);
void gtkui_cursor_set_crosshair();
void gtkui_cursor_set_default();
void gtkui_toggle_fullscreen();
void gtkui_signals_init();
void gtkui_signals_deinit();

#endif
