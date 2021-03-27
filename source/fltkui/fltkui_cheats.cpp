/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2021 R. Danbrook
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_draw.H>

#include "cheats.h"
#include "nstcommon.h"
#include "fltkui_cheats.h"

static Fl_Input *input_desc;
static Fl_Input *input_gg;
static Fl_Input *input_par;
static int rsel = 0;

extern Emulator emulator;
extern std::vector<NstCheat> chtlist;
extern nstpaths_t nstpaths;

class ChtTable : public Fl_Table_Row
{
protected:
	void draw_cell(TableContext context, int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);

public:
	ChtTable(int x, int y, int w, int h, const char *l=0) : Fl_Table_Row(x,y,w,h,l) { end(); }
	~ChtTable() { }
};

static ChtTable *ctable;

static void cb_ok(Fl_Widget *w, long) {
	w->parent()->hide();
}

void cb_table(Fl_Widget* w, long rn) {
	Fl_Table *table = (Fl_Table*)w;

	if (!table->rows()) { return; }

	rn = table->callback_row();
	rsel = rn;

	if (Fl::event_clicks() > 0) {
		chtlist[rn].enabled = !chtlist[rn].enabled;
		nst_cheats_refresh();
	}

	ctable->redraw();
}

void cb_toggle(Fl_Widget* w, long) {
	if (!chtlist.size()) { return; }
	chtlist[rsel].enabled = !chtlist[rsel].enabled;
	nst_cheats_refresh();
	ctable->redraw();
}

void cb_add(Fl_Widget* w, long) {
	NstCheat cht;
	bool addgg = false;
	bool addpar = false;
	cht.enabled = true;

	wchar_t wtmp[256];

	mbstowcs(wtmp, input_desc->value(), 256);
	cht.description = std::wstring(wtmp);

	if (strlen(input_gg->value())) {
		mbstowcs(wtmp, input_gg->value(), 256);
		cht.gg = std::wstring(wtmp);
		addgg = true;
	}

	if (strlen(input_par->value())) {
		mbstowcs(wtmp, input_par->value(), 256);
		cht.par = std::wstring(wtmp);
		addpar = true;
	}

	cht.address = cht.value = cht.compare = 0;

	if (addgg || addpar) {
		chtlist.push_back(cht);
		nst_cheats_refresh();
		ctable->rows(chtlist.size());
		input_desc->value("");
		input_gg->value("");
		input_par->value("");
		return;
	}
}

void cb_del(Fl_Widget* w, long) {
	if (chtlist.size()) {
		chtlist.erase(chtlist.begin() + rsel);
		nst_cheats_refresh();
		ctable->rows(chtlist.size());
	}
}

void cb_clr(Fl_Widget* w, long) {
	chtlist.clear();
	nst_cheats_refresh();
	ctable->rows(chtlist.size());
}

void cb_load(Fl_Widget* w, long) {
	Fl_Native_File_Chooser fc;
	fc.title("Select a Cheat List");
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fc.directory((const char*)nstpaths.cheatpath);
	fc.filter("Nestopia Cheats\t*.xml");

	// Show file chooser
	switch (fc.show()) {
		case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
		case 1: break; // Cancel
		default:
			if (fc.filename()) {
				chtlist.clear();
				nst_cheats_init(fc.filename());
				ctable->rows(chtlist.size());
			}
			break;
	}
}

// Handle drawing all cells in table
void ChtTable::draw_cell(TableContext context, int r, int c, int X, int Y, int W, int H) {
	static char s[128];

	switch (context) {
		case CONTEXT_COL_HEADER:
			switch (c) {
				case 0: snprintf(s, sizeof(s), ""); break;
				case 1: snprintf(s, sizeof(s), "Game Genie"); break;
				case 2: snprintf(s, sizeof(s), "PAR"); break;
				case 3: snprintf(s, sizeof(s), "Raw"); break;
				case 4: snprintf(s, sizeof(s), "Description"); break;
			}

			fl_push_clip(X, Y, W, H);

			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());
			fl_color(FL_BLACK);
			fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);

			fl_pop_clip();
			return;

		case CONTEXT_CELL:
			fl_push_clip(X, Y, W, H);

			switch (c) {
				case 0: snprintf(s, sizeof(s), "%s", chtlist[r].enabled ? "On" : "Off"); break;
				case 1: snprintf(s, sizeof(s), "%ls", chtlist[r].gg.c_str()); break;
				case 2: snprintf(s, sizeof(s), "%ls", chtlist[r].par.c_str()); break;
				case 3:
					if (chtlist[r].address) {
						snprintf(s, sizeof(s), "%04X %02X %02X", chtlist[r].address, chtlist[r].value, chtlist[r].compare);
					}
					else {
						snprintf(s, sizeof(s), "");
					}
					break;
				case 4: snprintf(s, sizeof(s), "%ls", chtlist[r].description.c_str()); break;
				default: break;
			}

			// Background
			fl_color( row_selected(r) ? selection_color() : FL_WHITE);
			fl_rectf(X, Y, W, H);

			// Text
			fl_color(FL_BLACK);
			fl_draw(s, X, Y, W, H, c ? FL_ALIGN_LEFT : FL_ALIGN_CENTER);

			fl_pop_clip();

			return;

		default: return;
	}
}

void NstChtWindow::refresh() {
	ctable->rows(chtlist.size());
	ctable->row_header(0);
}

void NstChtWindow::populate() {
	ctable = new ChtTable(19, 20, 642, 270);
	ctable->selection_color(FL_YELLOW);
	ctable->cols(5);
	ctable->col_header(1); // enable col header
	ctable->col_width(0, 80);
	ctable->col_width(1, 100);
	ctable->col_width(2, 90);
	ctable->col_width(3, 90);
	ctable->col_width(4, 260);

	ctable->callback(cb_table, 0);
	ctable->when(FL_WHEN_CHANGED);
	ctable->end();

	input_desc = new Fl_Input(380, 310, 260, 25, "Description:");
	input_gg = new Fl_Input(380, 340, 260, 25, "Game Genie:");
	input_par = new Fl_Input(380, 370, 260, 25, "Pro Action Rocky:");

	Fl_Button *btnadd = new Fl_Button(380, 400, 80, 25, "Add");
	btnadd->callback(cb_add, 0);

	Fl_Button *btntog = new Fl_Button(20, 300, 80, 25, "Toggle");
	btntog->callback(cb_toggle, 0);

	Fl_Button *btndel = new Fl_Button(110, 300, 80, 25, "Delete");
	btndel->callback(cb_del, 0);

	Fl_Button *btnclr = new Fl_Button(200, 300, 80, 25, "Clear");
	btnclr->callback(cb_clr, 0);

	Fl_Button *btnload = new Fl_Button(20, 360, 80, 25, "Load...");
	btnload->callback(cb_load, 0);

	Fl_Button *btnok = new Fl_Button(560, 460, 80, 25, "&OK");
	btnok->callback(cb_ok, 0);

	this->end();
}
