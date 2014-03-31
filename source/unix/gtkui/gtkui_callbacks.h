#ifndef _GTKUI_CALLBACKS_H_
#define _GTKUI_CALLBACKS_H_

void gtkui_cb_reset(GtkWidget *reset, int hard);
unsigned int gtkui_cb_translate_gdk_sdl(int gdk_keyval);
int gtkui_cb_convert_key(GtkWidget *grab, GdkEventKey *event, gpointer user_data);

#endif
