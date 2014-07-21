#ifndef _GTKUI_CONFIG_H_
#define _GTKUI_CONFIG_H_

#define MARGIN_TB 5
#define MARGIN_LR 10

GtkWidget *gtkui_config();
void gtkui_config_ok();
void gtkui_config_input();
void gtkui_config_input_refresh();
void gtkui_config_input_fields(int type, int pnum);
void gtkui_config_input_focus(int counter);

#endif
