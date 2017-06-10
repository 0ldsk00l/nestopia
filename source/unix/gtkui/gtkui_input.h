#ifndef _GTKUI_INPUT_H_
#define _GTKUI_INPUT_H_

typedef struct {
	guint u;
	guint d;
	guint l;
	guint r;
	guint select;
	guint start;
	guint a;
	guint b;
	guint ta;
	guint tb;
} gpad_t;

typedef struct {
	guint qsave1;
	guint qsave2;
	guint qload1;
	guint qload2;
	guint screenshot;
	guint fdsflip;
	guint fdsswitch;
	guint insertcoin1;
	guint insertcoin2;
	guint reset;
	guint ffspeed;
	guint rwstart;
	guint rwstop;
	guint fullscreen;
	guint filter;
	guint scalefactor;
} gkeys_t;

void gtkui_input_set_default();
void gtkui_input_config_read();
void gtkui_input_config_write();
void gtkui_input_config_item(int pnum, int bnum);
void gtkui_input_null();
int gtkui_input_process_key(GtkWidget *widget, GdkEventKey *event, gpointer userdata);
int gtkui_input_process_mouse(GtkWidget *widget, GdkEventButton *event, gpointer userdata);

#endif
