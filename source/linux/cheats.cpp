/*
	NEStopia / Linux
	Port by R. Belmont
	
	cheats.cpp - Cheat Manager UI and cheat handling
*/

#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <cstdlib>
#include <cwchar>
#include <cerrno>
#include <cstring>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiCheats.hpp"
#include "../core/NstStream.hpp"
#include "../core/NstXml.hpp"	// not entirely kosher but Marty does it on Windows :)
#include "cheats.h"
#include "settings.h"

extern "C" {
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "callbacks.h"
}

#include "uihelp.h"

using namespace Nes::Api;
using namespace LinuxNst;

typedef Nes::Core::Xml Xml;

typedef struct 
{
	Cheats::Code code;
	char gg[9], par[9], raw[32], description[64];
	bool enabled;
	unsigned int crc;
} NstCheatEntry;


extern Emulator emulator;
extern GtkWidget *mainwindow;
extern char rootname[512];

static GtkWidget *cheatwin, *cheattree, *ggedit, *paredit, *ggokbut, *parokbut;
static GtkWidget *chopen, *chsave, *chdelete;
static GtkTreeStore *treestore;
static GtkCellRenderer *renderer, *togglerend;
static GtkTreeViewColumn *column[5];
static GtkTreeSelection *selection;
static std::vector<NstCheatEntry> cheatlist;	// our private list of cheats
static std::vector<GtkTreeIter> treeiters;
static bool sIsOpen;

static int find_current_selection(void)
{
	int i;

	for (i = 0; i < treeiters.size(); i++)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &treeiters[i]))
		{
			return i;
		}
	}

	return -1;
}

static void enable_toggle(GtkCellRendererToggle *cell_renderer, gchar *path, gpointer user_data)
{
	int item;
	char *eptr;

	item = strtoul(path, &eptr, 0);

	cheatlist[item].enabled = !cheatlist[item].enabled;

	gtk_tree_store_set(treestore, &treeiters[item], 0, cheatlist[item].enabled, -1);
}

static void on_cheatwin_destroy(GtkObject *object, gpointer user_data)
{
	treeiters.clear();
	sIsOpen = false;
}

CheatMgr::CheatMgr()
{
	sIsOpen = false;
}

CheatMgr::~CheatMgr()
{
}

void CheatMgr::ShowManager()
{
	int i;
	unsigned short addr;
	unsigned char val, cmp;
	bool usecmp;
	Cheats cheats(emulator);
	std::vector<NstCheatEntry> templist;

	if (sIsOpen) 
	{
		return;
	}

	sIsOpen = true;

	cheatwin = create_cheatwind();
	cheattree = lookup_widget(cheatwin, "cheattree");
	ggedit = lookup_widget(cheatwin, "ggedit");
	paredit = lookup_widget(cheatwin, "paredit");
	ggokbut = lookup_widget(cheatwin, "genieok");
	parokbut = lookup_widget(cheatwin, "parok");
	chopen = lookup_widget(cheatwin, "cheatopen"); 
	chsave = lookup_widget(cheatwin, "cheatsave"); 
	chdelete = lookup_widget(cheatwin, "chdelete");  

	gtk_window_set_icon(GTK_WINDOW(cheatwin), UIHelp_GetNSTIcon());

	g_signal_connect((gpointer)cheatwin, "destroy", G_CALLBACK(on_cheatwin_destroy), NULL);

	// set various widgets' user data to our "this" pointer
	gtk_object_set_user_data(GTK_OBJECT(ggokbut), (gpointer)this);
	gtk_object_set_user_data(GTK_OBJECT(parokbut), (gpointer)this);
	gtk_object_set_user_data(GTK_OBJECT(chopen), (gpointer)this);
	gtk_object_set_user_data(GTK_OBJECT(chsave), (gpointer)this);
	gtk_object_set_user_data(GTK_OBJECT(chdelete), (gpointer)this);

	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW (cheattree), FALSE);

	// set up our tree store
	treestore = gtk_tree_store_new(5, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	// attach the store to the tree	
	gtk_tree_view_set_model(GTK_TREE_VIEW(cheattree), GTK_TREE_MODEL(treestore));

	// create a cell renderer using the stock text one
	renderer = gtk_cell_renderer_text_new();

	// get the stock toggle renderer
	togglerend = gtk_cell_renderer_toggle_new();

	// get us an event handler for it
	g_signal_connect(G_OBJECT(togglerend), "toggled", G_CALLBACK(enable_toggle), NULL);

	// create the display columns
	column[0] = gtk_tree_view_column_new_with_attributes("Enable", togglerend, "active", 0, NULL);
	column[1] = gtk_tree_view_column_new_with_attributes("Game Genie", renderer, "text",  1, NULL);
	column[2] = gtk_tree_view_column_new_with_attributes("PAR", renderer, "text",  2, NULL);
	column[3] = gtk_tree_view_column_new_with_attributes("Raw", renderer, "text",  3, NULL);
	column[4] = gtk_tree_view_column_new_with_attributes("Description", renderer, "text",  4, NULL);

	// add the display column and renderer to the tree view
	gtk_tree_view_append_column(GTK_TREE_VIEW(cheattree), column[0]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(cheattree), column[1]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(cheattree), column[2]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(cheattree), column[3]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(cheattree), column[4]);

	// get the selection object too
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cheattree));

	// add the existing cheats
	// since AddCode inserts into the master cheatlist we cheat ourselves and
	// make a copy of the cheatlist, clear the master, then insert from the copy
	templist = cheatlist;
	cheatlist.clear();
	for (i = 0; i < templist.size(); i++)
	{
		Cheats::Code codetoadd;

		codetoadd = templist[i].code;
		AddCode(codetoadd, true, templist[i].enabled, templist[i].description);
	}

	// show the window
	gtk_widget_show(cheatwin);
}

void CheatMgr::Enable(void)
{
	Cheats cheats(emulator);
	int i;

	// clear anything in the engine
	cheats.ClearCodes();	

	// now scan our list and add any enabled codes
	for (i = 0; i < cheatlist.size(); i++)
	{
		if (cheatlist[i].enabled) 
		{
			cheats.SetCode(cheatlist[i].code);
		}
	}
}

void CheatMgr::Unload(void)
{
	Cheats cheats(emulator);

	// clear any in-engine cheats
	cheats.ClearCodes();

	// clear our cheatlist
	cheatlist.clear();
}

void CheatMgr::AddCode(Cheats::Code &code, bool useenable, bool enable, char *description)
{
	Cheats cheats(emulator);
	GtkTreeIter tmpiter;
	unsigned short addr;
	unsigned char val, cmp;
	bool usecmp;
	int iteridx = treeiters.size();
	int clidx = cheatlist.size();
	NstCheatEntry newentry;

	newentry.code = code;
	newentry.crc = 0;
	newentry.description[0] = '\0';

	if (description != NULL)
	{
		strncpy(newentry.description, description, 63);
	}

	if (useenable) 
	{
		newentry.enabled = enable;
	}
	else
	{
		newentry.enabled = true;
	}

	if (cheats.GameGenieEncode(code, newentry.gg) != Nes::RESULT_OK)
	{
		strcpy(newentry.gg, "---");
	}

	// not all GG codes can be represented in PAR
	if (cheats.ProActionRockyEncode(code, newentry.par) != Nes::RESULT_OK)
	{
		strcpy(newentry.par, "---");
	}

	// add it to the list box
	sprintf(newentry.raw, "%04x - %02x - %02x", code.address, code.value, code.compare);

//	printf("AddCode [GG %s]: iteridx = %d, clidx = %d, useenable %c, enable %c\n", newentry.gg, iteridx, clidx, useenable ? 'T' : 'F', enable ? 'T' : 'F');

	cheatlist.push_back(newentry);
	treeiters.push_back(tmpiter);
	gtk_tree_store_insert(treestore, &treeiters[iteridx], NULL, 999999);
	gtk_tree_store_set(treestore, &treeiters[iteridx], 0, cheatlist[clidx].enabled, 1, cheatlist[clidx].gg, 2, cheatlist[clidx].par, 3, cheatlist[clidx].raw, 4, cheatlist[clidx].description, -1);

	// add it to the engine so it takes effect
//	cheats.SetCode(code);
}							    

void on_parok_clicked(GtkButton *button, gpointer user_data)
{
	Cheats cheats(emulator);
	Cheats::Code code;
	CheatMgr *cmgr = (CheatMgr *)user_data;

	if (cheats.ProActionRockyDecode(gtk_entry_get_text(GTK_ENTRY(paredit)), code) == Nes::RESULT_OK)
	{
		cmgr->AddCode(code);
		gtk_entry_set_text(GTK_ENTRY(ggedit), "");
		gtk_entry_set_text(GTK_ENTRY(paredit), "");
	}
}

void on_parvalid_clicked(GtkButton *button, gpointer user_data)
{
	Cheats cheats(emulator);
	Cheats::Code code;
	char gg[9];

	if (cheats.ProActionRockyDecode(gtk_entry_get_text(GTK_ENTRY(paredit)), code) == Nes::RESULT_OK)
	{
		cheats.GameGenieEncode(code, gg);
		gtk_entry_set_text(GTK_ENTRY(ggedit), gg);
	}
}

void on_genieok_clicked(GtkButton *button, gpointer user_data)
{
	Cheats cheats(emulator);
	Cheats::Code code;
	CheatMgr *cmgr = (CheatMgr *)user_data;

	if (cheats.GameGenieDecode(gtk_entry_get_text(GTK_ENTRY(ggedit)), code) == Nes::RESULT_OK)
	{
		cmgr->AddCode(code);
		gtk_entry_set_text(GTK_ENTRY(ggedit), "");
		gtk_entry_set_text(GTK_ENTRY(paredit), "");
	}
}

void on_chtggvalid_clicked(GtkButton *button, gpointer user_data)
{
	Cheats cheats(emulator);
	Cheats::Code code;
	char par[9];

	if (cheats.GameGenieDecode(gtk_entry_get_text(GTK_ENTRY(ggedit)), code) == Nes::RESULT_OK)
	{
		cheats.ProActionRockyEncode(code, par);
		gtk_entry_set_text(GTK_ENTRY(paredit), par);
	}
}

void on_chdelete_clicked(GtkButton *button, gpointer user_data)
{
	int cursel = find_current_selection();

	if (cursel != -1)
	{
		gtk_tree_store_remove(treestore, &treeiters[cursel]);
		memset(&treeiters[cursel], 0, sizeof(GtkTreeIter));
		cheatlist.erase(cheatlist.begin()+cursel);
		treeiters.erase(treeiters.begin()+cursel);
	}
}

void on_cheatopen_clicked(GtkButton *button, gpointer user_data)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	Xml xml;
	Cheats cheats(emulator);
	CheatMgr *cmgr = (CheatMgr *)user_data;
	char description[64];

	dialog = gtk_file_chooser_dialog_new ("Import cheats (.xml)",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "NEStopia XML cheats");
	gtk_file_filter_add_pattern(filter, "*.xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		std::ifstream stream( filename, std::ifstream::in|std::ifstream::binary );

		if (stream.is_open())
		{
			xml.Read( stream );
			
			if (xml.GetRoot().IsType( L"cheats" ))
			{
				for (Xml::Node node(xml.GetRoot().GetFirstChild()); node; node=node.GetNextSibling())
				{
					if (!node.IsType( L"cheat" ))
						continue;

					Cheats::Code code;

					code.useCompare = false;

					if (const Xml::Node address=node.GetChild( L"address" ))
					{
						uint v;

						if (0xFFFF < (v=address.GetUnsignedValue()))
							continue;

						code.address = v;

						if (const Xml::Node value=node.GetChild( L"value" ))
						{
							if (0xFF < (v=value.GetUnsignedValue()))
								continue;

							code.value = v;
						}

						if (const Xml::Node compare=node.GetChild( L"compare" ))
						{
							if (0xFF < (v=compare.GetUnsignedValue()))
								continue;

							code.compare = v;
							code.useCompare = true;
						}
					}
					else
					{
						char codestr[512];

						if (const Xml::Node genie=node.GetChild( L"genie" ))
						{
							wcstombs(codestr, genie.GetValue(), 511);
							if (cheats.GameGenieDecode(codestr, code) != Nes::RESULT_OK)
							{
								continue;
							}
						}
						else if (const Xml::Node rocky=node.GetChild( L"rocky" ))
						{
							wcstombs(codestr, rocky.GetValue(), 511);
							if (cheats.ProActionRockyDecode(codestr, code) != Nes::RESULT_OK)
							{
								continue;
							}
						}
						else
						{
							continue;
						}
					}

					// add the code, with a description if it's got one
					if (node.GetChild( L"description" )) 
					{
						wcstombs(description, node.GetChild( L"description" ).GetValue(), 63);
						cmgr->AddCode(code, true, node.GetAttribute( L"enabled" ).IsValue( L"0" ) ? false : true, description);
					}
					else
					{
						cmgr->AddCode(code, true, node.GetAttribute( L"enabled" ).IsValue( L"0" ) ? false : true);
					}

					// poke in the CRC too if there is one
					cheatlist[cheatlist.size()-1].crc = node.GetAttribute( L"game" ).GetUnsignedValue();
				}
			}
		}

		g_free (filename);
	}

	gtk_widget_destroy(dialog);
}

void on_cheatsave_clicked(GtkButton *button, gpointer user_data)
{
	GtkWidget *dialog;
	char defname[512];
	Cheats cheats(emulator);

	// nothing to export?
	if (cheats.NumCodes() == 0)
	{
		return;
	}

	defname[0] = '\0';
	strcpy(defname, rootname);
	strcat(defname, ".xml");

	dialog = gtk_file_chooser_dialog_new ("Export cheats (.xml)",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), defname);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		std::ofstream stream( filename, std::ifstream::out|std::ifstream::binary );

		if (stream.is_open())
		{
			int i;
			Xml xml;
			Xml::Node root(xml.GetRoot());

			root = xml.Create( L"cheats" );
			root.AddAttribute( L"version", L"1.0" );

			for (i = 0; i < cheatlist.size(); i++)
			{
				Cheats::Code code;
				char buffer[9];
				wchar_t wbuffer[64];

				code = cheatlist[i].code;

				Xml::Node node( root.AddChild( L"cheat" ) );
				node.AddAttribute( L"enabled", cheatlist[i].enabled ? L"1" : L"0" );

				if (cheatlist[i].crc)
				{
					sprintf(buffer, "%08X", cheatlist[i].crc);
					mbstowcs(wbuffer, buffer, 63);
					node.AddAttribute( L"game", wbuffer );
				}

				if (NES_SUCCEEDED(cheats.GameGenieEncode(code, buffer)))
				{
					mbstowcs(wbuffer, buffer, 63);
					node.AddChild( L"genie", wbuffer );
				}

				if (NES_SUCCEEDED(cheats.ProActionRockyEncode(code, buffer)))
				{
					mbstowcs(wbuffer, buffer, 63);
					node.AddChild( L"rocky", wbuffer );
				}

				sprintf(buffer, "0x%04X", code.address);
				mbstowcs(wbuffer, buffer, 63);
				node.AddChild( L"address", wbuffer );
				sprintf(buffer, "0x%02X", code.value);
				mbstowcs(wbuffer, buffer, 63);
				node.AddChild( L"value", wbuffer );

				if (code.useCompare)
				{
					sprintf(buffer, "0x%02X", code.compare);
					mbstowcs(wbuffer, buffer, 63);
					node.AddChild( L"compare", wbuffer );
				}

				if (strlen(cheatlist[i].description))
				{
					mbstowcs(wbuffer, cheatlist[i].description, 63);
					node.AddChild( L"description", wbuffer );
				}
			}

			xml.Write( root, stream );
		}

		g_free (filename);
	}

	gtk_widget_destroy(dialog);
}

void on_cheatok_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(cheatwin);

	treeiters.clear();

	sIsOpen = false;
}

