#ifndef _GTKUI_H_
#define _GTKUI_H_

#include <gtk/gtk.h>

void gtkui_init(int argc, char *argv[]);
void gtkui_create();
GtkWidget *gtkui_about();

int area_start(GtkWidget *widget, void *data);
unsigned int translate_gdk_sdl(int gdk_keyval);
int convert_keypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data);

void gtkui_swapbuffers();

#endif
