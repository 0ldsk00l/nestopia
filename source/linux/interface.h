void pause_clicked();
void videoconfig_clicked();
void audioconfig_clicked();
void inputconfig_clicked();
void miscconfig_clicked();
void about_clicked();
void state_load();
void state_save();
void movie_load();
void movie_record();
void movie_stop();
void redraw_request();
void redraw_drawingarea(int xres, int yres);
void set_window_id(char* sdlwindowid);

GtkWidget* create_mainwindow(int xres, int yres);
GtkWidget* create_config();
GtkWidget* create_inputconfig();
GtkWidget* create_nsfplayer();
GtkWidget* create_about();
GtkWidget* create_archselect();
GtkWidget* create_cheatwindow();

gint convertKeypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data);
