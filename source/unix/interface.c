#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"

GtkWidget *window;
GtkWidget *drawingarea;

GdkColor bg = {0, 0, 0, 0};

GtkWidget* create_mainwindow (int xres, int yres) {

	GtkWidget *box;

	GtkWidget *menubar;
  
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *open;
	GtkWidget *sep1;
	GtkWidget *quit;
	
	GtkWidget *emulatormenu;
	GtkWidget *emulator;
	GtkWidget *cont;
	GtkWidget *pause;
	GtkWidget *reset;
	GtkWidget *sep2;
	GtkWidget *fullscreen;
	GtkWidget *sep3;
	GtkWidget *loadstate;
	GtkWidget *savestate;
	GtkWidget *sep4;
	GtkWidget *flipdisk;
	GtkWidget *sep5;
	GtkWidget *movieload;
	GtkWidget *movierecord;
	GtkWidget *moviestop;
	GtkWidget *sep6;
	GtkWidget *cheats;
	GtkWidget *sep7;
	GtkWidget *configuration;

	GtkWidget *helpmenu;
	GtkWidget *help;
	GtkWidget *about;
	
	GtkWidget *statusbar;
	
	GtkAccelGroup *accelgroup = gtk_accel_group_new();
	
	GtkSettings *gtksettings;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_window_set_title(GTK_WINDOW(window), "Nestopia");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), box);

	menubar = gtk_menu_bar_new();

	filemenu = gtk_menu_new();
	emulatormenu = gtk_menu_new();
	helpmenu = gtk_menu_new();
	
	gtk_window_add_accel_group(GTK_WINDOW(window), accelgroup);

	file = gtk_menu_item_new_with_mnemonic("_File");
	open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	sep1 = gtk_separator_menu_item_new();
	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	
	emulator = gtk_menu_item_new_with_mnemonic("_Emulator");
	cont = gtk_image_menu_item_new_with_mnemonic("C_ontinue");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cont), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU));
	pause = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PAUSE, NULL);
	reset = gtk_image_menu_item_new_with_mnemonic("_Reset");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(reset), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
	sep2 = gtk_separator_menu_item_new();
	fullscreen = gtk_image_menu_item_new_from_stock(GTK_STOCK_FULLSCREEN, NULL);
	sep3 = gtk_separator_menu_item_new();
	loadstate = gtk_image_menu_item_new_with_mnemonic("_Load State...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(loadstate), gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU));
	savestate = gtk_image_menu_item_new_with_mnemonic("_Save State...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(savestate), gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU));
	sep4 = gtk_separator_menu_item_new();
	flipdisk = gtk_image_menu_item_new_with_mnemonic("Flip FDS _Disk");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(flipdisk), gtk_image_new_from_stock(GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	sep5 = gtk_separator_menu_item_new();
	movieload = gtk_image_menu_item_new_with_label("Load Movie...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(movieload), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
	movierecord = gtk_image_menu_item_new_with_label("Record Movie...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(movierecord), gtk_image_new_from_stock(GTK_STOCK_MEDIA_RECORD, GTK_ICON_SIZE_MENU));
	moviestop = gtk_image_menu_item_new_with_label("Stop Movie");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(moviestop), gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU));
	sep6 = gtk_separator_menu_item_new();
	cheats = gtk_image_menu_item_new_with_mnemonic("C_heats...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cheats), gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU));
	sep7 = gtk_separator_menu_item_new();
	configuration = gtk_image_menu_item_new_with_mnemonic("_Configuration...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(configuration), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));

	help = gtk_menu_item_new_with_mnemonic("_Help");
	about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep1);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), loadstate);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), savestate);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep2);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), movieload);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), movierecord);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), moviestop);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep3);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(emulator), emulatormenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cont);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), pause);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), reset);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep4);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), fullscreen);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep5);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), flipdisk);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep6);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cheats);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep7);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), configuration);
  
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
  
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), emulator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	
	drawingarea = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawingarea, xres, yres);
	
	statusbar = gtk_statusbar_new();
	
	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), drawingarea, TRUE, TRUE, 0);
	//gtk_box_pack_start(GTK_BOX(box), statusbar, FALSE, FALSE, 0);
	
	GtkTargetEntry target_entry[1];

	target_entry[0].target = (gchar *)"text/uri-list";
	target_entry[0].flags = 0;
	target_entry[0].info = 0;
	
	gtk_drag_dest_set(drawingarea, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP), 
		target_entry, sizeof(target_entry) / sizeof(GtkTargetEntry), (GdkDragAction)(GDK_ACTION_MOVE | GDK_ACTION_COPY));

	g_signal_connect(G_OBJECT(drawingarea), "drag-data-received",
		G_CALLBACK(drag_data_received), NULL);

	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(on_mainwindow_destroy), NULL);

	g_signal_connect(G_OBJECT(open), "activate",
		G_CALLBACK(on_open_clicked), NULL);

	g_signal_connect(G_OBJECT(quit), "activate",
		G_CALLBACK(on_mainwindow_destroy), NULL);

	g_signal_connect(G_OBJECT(cont), "activate",
		G_CALLBACK(on_playbutton_clicked), NULL);

	g_signal_connect(G_OBJECT(pause), "activate",
		G_CALLBACK(pause_clicked), NULL);
		
	g_signal_connect(G_OBJECT(reset), "activate",
		G_CALLBACK(reset_clicked), NULL);
		
	g_signal_connect(G_OBJECT(fullscreen), "activate",
		G_CALLBACK(fullscreen_clicked), NULL);

	g_signal_connect(G_OBJECT(loadstate), "activate",
		G_CALLBACK(state_load), NULL);

	g_signal_connect(G_OBJECT(savestate), "activate",
		G_CALLBACK(state_save), NULL);
		
	g_signal_connect(G_OBJECT(flipdisk), "activate",
		G_CALLBACK(flipdisk_clicked), NULL);
		
	g_signal_connect(G_OBJECT(movieload), "activate",
		G_CALLBACK(movie_load), NULL);
		
	g_signal_connect(G_OBJECT(movierecord), "activate",
		G_CALLBACK(movie_record), NULL);
		
	g_signal_connect(G_OBJECT(moviestop), "activate",
		G_CALLBACK(movie_stop), NULL);

	g_signal_connect(G_OBJECT(cheats), "activate",
		G_CALLBACK(on_cheatbutton_pressed), NULL);

	g_signal_connect(G_OBJECT(configuration), "activate",
		G_CALLBACK(create_config), NULL);

	g_signal_connect(G_OBJECT(about), "activate",
		G_CALLBACK(create_about), NULL);
		
	gtksettings = gtk_settings_get_default();
	g_object_set(G_OBJECT(gtksettings), "gtk-application-prefer-dark-theme", TRUE, NULL);

	gtk_key_snooper_install(convertKeypress, NULL);

	gtk_widget_show_all(window);

	char SDL_windowhack[32];
	sprintf(SDL_windowhack, "SDL_WINDOWID=%ld", GDK_WINDOW_XID(gtk_widget_get_window(drawingarea)));
	set_window_id(SDL_windowhack);
	
	gtk_widget_modify_bg(drawingarea, GTK_STATE_NORMAL, &bg);

	return window;
}

void redraw_drawingarea(int xres, int yres) {
	gtk_widget_set_size_request(drawingarea, xres, yres);
	gtk_widget_modify_bg(drawingarea, GTK_STATE_NORMAL, &bg);
}
