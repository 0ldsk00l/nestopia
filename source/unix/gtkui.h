#include <gtk/gtk.h>

void UIHelp_Init(int argc, char *argv[], int xres, int yres);
void UIHelp_NSFLoaded(void);
GdkPixbuf *UIHelp_GetNSTIcon(void);

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
