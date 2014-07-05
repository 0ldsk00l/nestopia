#ifndef _GTKUI_CHEATS_H_
#define _GTKUI_CHEATS_H_

GtkWidget *gtkui_cheats();
void gtkui_cheats_check(GtkWidget *widget, gchar *element, gpointer userdata);
void gtkui_cheats_toggle(GtkWidget *widget, gpointer userdata);
void gtkui_cheats_fill_tree(char *filename);
void gtkui_cheats_save();
void gtkui_cheats_gg_add(GtkWidget *widget, gpointer userdata);
void gtkui_cheats_par_add(GtkWidget *widget, gpointer userdata);
void gtkui_cheats_remove(GtkWidget *widget, gpointer userdata);
void gtkui_cheats_ok();
void gtkui_cheats_clear();
gboolean gtkui_cheats_scan_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata);
gboolean gtkui_cheats_write_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata);

#endif
