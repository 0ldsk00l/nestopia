#ifndef _GTKUI_H_
#define _GTKUI_H_

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

void gtkui_init(int argc, char *argv[]);
void gtkui_create();
void gtkui_resize();
void gtkui_set_title(const char *title);
GtkWidget *gtkui_about();
void gtkui_image_paths();
void gtkui_message(const char* message);
void gtkui_cursor_set_crosshair();
void gtkui_cursor_set_default();

#endif
