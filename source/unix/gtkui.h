#include <gtk/gtk.h>

void gtkui_init(int argc, char *argv[], int xres, int yres);
//void UIHelp_NSFLoaded(void);

void pause_clicked();
void redraw_drawingarea(int xres, int yres);
void drag_data_received(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data);
void set_window_id(char* sdlwindowid);

GtkWidget* create_mainwindow(int xres, int yres);
GtkWidget* create_config();
GtkWidget* create_inputconfig();
GtkWidget* create_nsfplayer();
GtkWidget* create_about();
GtkWidget* create_archselect();
GtkWidget* create_cheatwindow();

void create_messagewindow(char* message);

gint convertKeypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data);

/* This is old stuff that needs to be removed at some point */
gboolean
on_mainwindow_destroy_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_nsfspinbutton_change_value          (GtkSpinButton   *spinbutton,
                                        GtkScrollType    scroll,
                                        gpointer         user_data);

void
on_nsfspinbutton_value_changed         (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_open_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_playbutton_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_nsfplay_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_scalecombo_changed                  (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_rendercombo_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_systemcombo_changed                  (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_ntsccombo_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_videocombo_changed                  (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_check_fullscreen_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_unlimitsprcheck_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_controlcheck_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_configbutton_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_alsacheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_volumescroll_value_changed          (GtkRange        *range,
                                        gpointer         user_data);

void
on_volumescroll_configure_event        (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_ratecombo_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_ratecombo_configure_event           (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_mainwindow_destroy                  (GObject       *object,
                                        gpointer         user_data);
                                        
void
on_nsfplayer_destroy					(GObject       *object,
										gpointer         user_data);

void
on_scaleamtcombo_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_configcombo_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_archok_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_archcancel_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_cheatbutton_pressed                 (GtkButton       *button,
                                        gpointer         user_data);
                                        
void
on_aboutbutton_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_parok_pressed                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_genieok_pressed                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_chdelete_pressed                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_cheatok_pressed                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_spatchcombo_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_surrscroll_value_changed            (GtkRange        *range,
                                        gpointer         user_data);

void
on_stereocheck_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_surrcheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_excitecheck_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_chtggvalid_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_genieok_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_parok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_cheatok_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_cheatopen_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_cheatsave_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_parvalid_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_chdelete_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_sndapicombo_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data);
