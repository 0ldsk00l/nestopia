#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"

GtkWidget* create_mainwindow (void) {

	GtkWidget *window;
	GtkWidget *box;

	GtkWidget *menubar;
	//GtkWidget *sep;
  
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *open;
	GtkWidget *quit;
	
	GtkWidget *emulatormenu;
	GtkWidget *emulator;
	GtkWidget *run;
	GtkWidget *pause;
	GtkWidget *savestate;
	GtkWidget *loadstate;
	GtkWidget *cheats;
	
	GtkWidget *configurationmenu;
	GtkWidget *configuration;
	GtkWidget *videoconfig;
	GtkWidget *audioconfig;
	GtkWidget *inputconfig;
	GtkWidget *miscconfig;

	GtkWidget *helpmenu;
	GtkWidget *help;
	GtkWidget *about;
	
	GtkWidget *drawingarea;
	
	GtkWidget *statusbar;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
	gtk_window_set_title(GTK_WINDOW(window), "Nestopia Undead");

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), box);

	menubar = gtk_menu_bar_new();
	//sep = gtk_separator_menu_item_new();
	filemenu = gtk_menu_new();
	emulatormenu = gtk_menu_new();
	configurationmenu = gtk_menu_new();
	helpmenu = gtk_menu_new();

	file = gtk_menu_item_new_with_label("File");
	open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	
	emulator = gtk_menu_item_new_with_label("Emulator");
	run = gtk_menu_item_new_with_label("Run");
	pause = gtk_menu_item_new_with_label("Pause");
	savestate = gtk_menu_item_new_with_label("Save State");
	loadstate = gtk_menu_item_new_with_label("Load State");
	cheats = gtk_menu_item_new_with_label("Cheats");
	
	configuration = gtk_menu_item_new_with_label("Configuration");
	videoconfig = gtk_menu_item_new_with_label("Video");
	audioconfig = gtk_menu_item_new_with_label("Audio");
	inputconfig = gtk_menu_item_new_with_label("Input");
	miscconfig = gtk_menu_item_new_with_label("Misc");
  
	help = gtk_menu_item_new_with_label("Help");
	about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(emulator), emulatormenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), run);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), pause);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), savestate);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), loadstate);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cheats);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(configuration), configurationmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(configurationmenu), videoconfig);
	gtk_menu_shell_append(GTK_MENU_SHELL(configurationmenu), audioconfig);
	gtk_menu_shell_append(GTK_MENU_SHELL(configurationmenu), inputconfig);
	gtk_menu_shell_append(GTK_MENU_SHELL(configurationmenu), miscconfig);
  
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
  
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), emulator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), configuration);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	
	drawingarea = gtk_drawing_area_new();
	gtk_widget_set_size_request (drawingarea, 200, 200);
	
	statusbar = gtk_statusbar_new();
	
	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), drawingarea, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), statusbar, FALSE, FALSE, 0);

	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(on_mainwindow_destroy), NULL);

	g_signal_connect(G_OBJECT(open), "activate",
		G_CALLBACK(on_open_clicked), NULL);
		
	g_signal_connect(G_OBJECT(quit), "activate",
		G_CALLBACK(on_mainwindow_destroy), NULL);

	g_signal_connect(G_OBJECT(run), "activate",
		G_CALLBACK(on_playbutton_clicked), NULL);
		
	g_signal_connect(G_OBJECT(savestate), "activate",
		G_CALLBACK(state_save), NULL);

	g_signal_connect(G_OBJECT(loadstate), "activate",
		G_CALLBACK(state_load), NULL);

	g_signal_connect(G_OBJECT(cheats), "activate",
		G_CALLBACK(on_cheatbutton_pressed), NULL);

	g_signal_connect(G_OBJECT(videoconfig), "activate",
		G_CALLBACK(create_videoconfig), NULL);
	
	g_signal_connect(G_OBJECT(audioconfig), "activate",
		G_CALLBACK(create_audioconfig), NULL);
		
	g_signal_connect(G_OBJECT(inputconfig), "activate",
		G_CALLBACK(create_inputconfig), NULL);
		
	g_signal_connect(G_OBJECT(miscconfig), "activate",
		G_CALLBACK(create_miscconfig), NULL);
        
	g_signal_connect(G_OBJECT(about), "activate",
		G_CALLBACK(create_about), NULL);

	gtk_widget_show_all(window);

	return window;
}

GtkWidget* create_about (void) {

	char svgpath[1024];
	sprintf(svgpath, "%s/icons/nestopia.svg", DATADIR);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(svgpath, 256, 256, NULL);
	
	GtkWidget *aboutdialog = gtk_about_dialog_new();
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutdialog), pixbuf);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), "Nestopia Undead");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), "1.43");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), "An accurate Nintendo Entertainment System Emulator");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), "http://0ldsk00l.ca/");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutdialog), "(c) 2012, R. Danbrook\n(c) 2007-2008, R. Belmont\n(c) 2003-2008, Martin Freij\n\nIcon based on art from Trollekop");
	g_object_unref(pixbuf), pixbuf = NULL;
	gtk_dialog_run(GTK_DIALOG(aboutdialog));
	gtk_widget_destroy(aboutdialog);
	
	return aboutdialog;
}
