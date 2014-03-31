#ifndef _GTKUI_H_
#define _GTKUI_H_

#include <gtk/gtk.h>

void gtkui_init(int argc, char *argv[]);
void gtkui_create();
void gtkui_toggle_fullscreen();
void gtkui_resize();
GtkWidget *gtkui_about();

int area_start(GtkWidget *widget, void *data);

void gtkui_swapbuffers();

#endif
