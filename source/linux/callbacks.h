#include <gtk/gtk.h>


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
on_nsfstop_clicked                     (GtkButton       *button,
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
on_favorcombo_changed                  (GtkComboBox     *combobox,
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

gboolean
on_volumescroll_configure_event        (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_ratecombo_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data);

gboolean
on_ratecombo_configure_event           (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_mainwindow_destroy                  (GtkObject       *object,
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
