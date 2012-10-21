void videoconfig_clicked();
void audioconfig_clicked();
void inputconfig_clicked();
void miscconfig_clicked();
void about_clicked();
void state_save();
void state_load();
void set_window_id(char* sdlwindowid);

GtkWidget* create_mainwindow(int xres, int yres);
GtkWidget* create_videoconfig();
GtkWidget* create_audioconfig();
GtkWidget* create_inputconfig();
GtkWidget* create_miscconfig();
GtkWidget* create_nsfplayer();
GtkWidget* create_about();
GtkWidget* create_archselect();
GtkWidget* create_cheatwindow();

gint convertKeypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data);
