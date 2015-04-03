#ifndef _GTKUI_CALLBACKS_H_
#define _GTKUI_CALLBACKS_H_

void gtkui_cb_reset(GtkWidget *reset, int hard);
void gtkui_cb_nothing();
void gtkui_cb_video_refresh();

void gtkui_cb_video_filter(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_scale(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_ntscmode(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_xbrrounding(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_xbrpixblend(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_video_linear_filter(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_video_tv_aspect(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_video_unmask_overscan(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_video_stretch_aspect(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_video_unlimited_sprites(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_video_palette(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_decoder(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_video_brightness(GtkRange *range, gpointer userdata);
void gtkui_cb_video_saturation(GtkRange *range, gpointer userdata);
void gtkui_cb_video_contrast(GtkRange *range, gpointer userdata);
void gtkui_cb_video_hue(GtkRange *range, gpointer userdata);

void gtkui_cb_audio_api(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_audio_samplerate(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_audio_stereo(GtkToggleButton *togglebutton, gpointer userdata);

void gtkui_cb_input_turbopulse(GtkRange *range, gpointer userdata);

void gtkui_cb_misc_default_system(GtkComboBox *combobox, gpointer userdata);
void gtkui_cb_timing_altspeed(GtkRange *range, gpointer userdata);
void gtkui_cb_timing_vsync(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_timing_limiter(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_misc_soft_patching(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_misc_genie_distortion(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_misc_disable_gui(GtkToggleButton *togglebutton, gpointer userdata);
void gtkui_cb_misc_config_pause(GtkToggleButton *togglebutton, gpointer userdata);

unsigned int gtkui_cb_translate_gdk_sdl(int gdk_keyval);
int gtkui_cb_convert_key(GtkWidget *grab, GdkEventKey *event, gpointer userdata);
int gtkui_cb_convert_mouse(GtkDrawingArea *area, GdkEventButton *event, gpointer userdata);

void gtkui_drag_data(GtkWidget *widget, GdkDragContext *dragcontext, gint x, gint y, GtkSelectionData *seldata, guint info, guint time, gpointer data);

#endif
