#ifndef _GTKUI_CALLBACKS_H_
#define _GTKUI_CALLBACKS_H_

void gtkui_cb_reset(GtkWidget *reset, int hard);

void gtkui_cb_destroy_config();
void gtkui_cb_video_filter(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_scale(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_palette(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_decoder(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_ntscmode(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_xbrrounding(GtkComboBox *combobox, gpointer userdata);

void gtkui_cb_video_refresh();

unsigned int gtkui_cb_translate_gdk_sdl(int gdk_keyval);
int gtkui_cb_convert_key(GtkWidget *grab, GdkEventKey *event, gpointer user_data);

#endif
