/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2024 R. Danbrook
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

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Table_Row.H>
#include <FL/fl_draw.H>

#include "fltkui.h"
#include "fltkui_settings.h"

namespace {

constexpr int UI_TABHEIGHT = 450;
constexpr int UI_TABWIDTH = 480;
constexpr unsigned UI_SETTINGS_PER_COL = 8;

NstSettingsWindow *win = nullptr;

}

NstSettingsWindow::NstSettingsWindow(int w, int h, const char* t, JGManager& j, SettingManager& m, InputManager& i)
        : Fl_Double_Window(w, h, t), jgm(j), setmgr(m), inputmgr(i) {
    win = this;

    Fl_Tooltip::color(NstPurple);
    Fl_Tooltip::textcolor(NstLightGrey);

    Fl_Tabs *tabs = new Fl_Tabs(10, 5, UI_TABWIDTH, UI_TABHEIGHT);
    tabs->selection_color(NstGreen);
    tabs->labelcolor(NstWhite);

    Fl_Group *fetab = new Fl_Group(10, 30, UI_TABWIDTH, UI_TABHEIGHT, "Interface");
    populate(*setmgr.get_settings());
    fetab->end();

    Fl_Group *emutab = new Fl_Group(10, 30, UI_TABWIDTH, UI_TABHEIGHT, "Emulator");
    populate(*jgm.get_settings());
    emutab->end();

    Fl_Group *inputtab = new Fl_Group(10, 30, UI_TABWIDTH, UI_TABHEIGHT, "Input");
    populate(*jgm.get_settings(), true);
    populate_input();
    inputtab->end();

    tabs->end();

    Fl_Button *btn_ok = new Fl_Button(UI_TABWIDTH - 30, UI_TABHEIGHT + 15, 40, UI_ELEMHEIGHT, "&OK");
    btn_ok->callback(cb_ok_s, nullptr);
    btn_ok->shortcut(FL_ALT + 'o');

    this->end();
}

void NstSettingsWindow::set_choice_value(std::string tab, std::string label, int val) {
    // Tabs
    Fl_Group *g = this->as_group()->child(0)->as_group();
    bool tab_found = false;

    // Search for the tab
    for (int i = 0; i < g->children(); ++i) {
        if (std::string(g->child(i)->label()) == tab) {
            g = g->child(i)->as_group();
            tab_found = true;
            break;
        }
    }

    if (!tab_found) {
        return;
    }

    for (int i = 0; i < g->children(); ++i) {
        if (g->child(i)->label()) {
            if (std::string(g->child(i)->label()) == label) {
                ((Fl_Choice*)g->child(i))->value(val);
                break;
            }
        }
    }
}

void NstSettingsWindow::cb_chooser(Fl_Widget *w, void *data) {
    jg_setting_t *setting = (jg_setting_t*)data;
    setting->val = ((Fl_Choice*)w)->value();

    if (setting->flags & FLAG_FRONTEND) {
        FltkUi::rehash();
    }
    else {
        jgm.rehash();
    }

    if (setting->flags & JG_SETTING_INPUT) {
        inputmgr.reassign();
    }
}

void NstSettingsWindow::cb_chooser_s(Fl_Widget *w, void *data) {
    win->cb_chooser(w, data);
}

void NstSettingsWindow::cb_slider(Fl_Widget *w, void *data) {
    jg_setting_t *setting = (jg_setting_t*)data;
    setting->val = ((Fl_Hor_Value_Slider*)w)->value();

    if (setting->flags & FLAG_FRONTEND) {
        FltkUi::rehash();
    }
    else {
        jgm.rehash();
        if (std::string(setting->name).find("overscan") != std::string::npos) {
            FltkUi::rehash();
        }
    }
}

void NstSettingsWindow::cb_slider_s(Fl_Widget *w, void *data) {
    win->cb_slider(w, data);
}

void NstSettingsWindow::cb_ok(Fl_Widget*, void*) {
    win->hide();
}

void NstSettingsWindow::cb_ok_s(Fl_Widget *w, void*) {
    win->cb_ok(w, nullptr);
}

void NstSettingsWindow::populate(const std::vector<jg_setting_t*>& settings, bool input_settings) {
    int xpos = 20;
    int ypos = 10;
    size_t j = 0;

    for (size_t i = 0; i < settings.size(); ++i) {
        if (input_settings == !(settings[i]->flags & JG_SETTING_INPUT)) {
            continue;
        }

        std::vector<std::string> opts;
        std::stringstream unparsed(settings[i]->opts);
        std::string temp;
        bool range = false;

        while (getline(unparsed, temp, ',')) {
            opts.push_back(temp.substr(temp.find("= ") + 2));
            if (temp[0] == 'N') {
                range = true;
                break;
            }
        }

        if (j && j % UI_SETTINGS_PER_COL == 0) {
            xpos += 220;
            ypos = 10;
        }
        ++j;

        ypos += UI_ELEMHEIGHT * 2;

        // Create a slider for range-based settings
        if (range) {
            Fl_Hor_Value_Slider *sld = new Fl_Hor_Value_Slider(xpos, ypos,
                                                               UI_ELEMWIDTH, UI_ELEMHEIGHT,
                                                               settings[i]->fname);
            sld->tooltip(settings[i]->desc);
            sld->align(FL_ALIGN_TOP_LEFT);
            sld->bounds(settings[i]->min, settings[i]->max);
            sld->box(FL_FLAT_BOX);
            sld->step(1);
            sld->selection_color(NstGreen);
            sld->type(FL_HOR_NICE_SLIDER);
            sld->callback(cb_slider_s, settings[i]);
            sld->value(settings[i]->val);
            continue;
        }

        // Create pulldown for non-range settings
        Fl_Choice *ch = new Fl_Choice(xpos, ypos,
                                      UI_ELEMWIDTH, UI_ELEMHEIGHT,
                                      settings[i]->fname);
        ch->tooltip(settings[i]->desc);
        ch->selection_color(NstGreen);
        ch->align(FL_ALIGN_TOP_LEFT);

        // Add options to the pulldown
        for (const auto& opt : opts) {
            ch->add(opt.c_str());
        }
        ch->callback(cb_chooser_s, settings[i]);
        ch->value(settings[i]->val);
    }
}

void NstSettingsWindow::cb_iselect(Fl_Widget *w, void *data) {
    itable->set_devicenum(((Fl_Choice*)w)->value());
    itable->rows(input_info[itable->get_devicenum()].numaxes +
                 input_info[itable->get_devicenum()].numbuttons);
    itable->redraw();
}

void NstSettingsWindow::cb_iselect_s(Fl_Widget *w, void *data) {
    win->cb_iselect(w, data);
}

void NstSettingsWindow::cb_itable(Fl_Widget *w, void *data) {
    InputTable *t = (InputTable*)w;
    int row = t->callback_row();

    if (Fl::event_clicks() > 0) {
        Fl::event_clicks(0); // Reset double-click counter
        itable->take_focus();
        show_msgbox(true);
        inputmgr.set_inputcfg(input_info[t->get_devicenum()].name,
                              input_info[t->get_devicenum()].defs[t->callback_row()],
                              t->callback_row());
    }
}

void NstSettingsWindow::cb_itable_s(Fl_Widget *w, void *data) {
    win->cb_itable(w, data);
}

int InputTable::handle(int e) {
    switch (e) {
        case FL_KEYUP: {
            if (inputmgr.get_cfg_running()) {
                if (Fl::event_key() == FL_Escape) { // Clear it
                    inputmgr.clear_inputdef();
                }
                else {
                    inputmgr.set_inputdef(Fl::event_key());
                }
                win->show_msgbox(false);
                inputmgr.set_cfg_running(false);
                redraw();
                return 1;
            }
            break;
        }
        case FL_PUSH: {
            if (inputmgr.get_cfg_running()) {
                inputmgr.set_inputdef(Fl::event_button() + 1000);
                win->show_msgbox(false);
                inputmgr.set_cfg_running(false);
                redraw();
                return 1;
            }
            break;
        }
        case FL_KEYDOWN: case FL_RELEASE: {
            if (inputmgr.get_cfg_running()) {
                return 1;
            }
            break;
        }
    }
    return Fl_Table_Row::handle(e);
}

void NstSettingsWindow::populate_input() {
    Fl_Choice *iselect = new Fl_Choice(200, UI_ELEMHEIGHT * 2 + 10,
                                       UI_ELEMWIDTH + 5, UI_ELEMHEIGHT,
                                       "Configure Input");
    iselect->tooltip("Select emulated device to configure");
    iselect->selection_color(NstGreen);
    iselect->align(FL_ALIGN_TOP_LEFT);

    itable = new InputTable(inputmgr, input_info,
                            200, 110, 275, 280);
    itable->set_devicenum(0);
    itable->cols(3);
    itable->col_width(0, 115);
    itable->col_width(1, 90);
    itable->col_width(2, 50);
    itable->color(NstWhite);
    itable->callback(cb_itable_s);

    input_info = inputmgr.get_inputinfo();

    for (auto& device : input_info) {
        iselect->add(device.fname);
    }

    iselect->callback(cb_iselect_s);

    iselect->value(0);
    itable->rows(input_info[0].numaxes + input_info[0].numbuttons);

    msgbox = new Fl_Box(200, 400, 240, UI_ELEMHEIGHT);
    msgbox->label("Press the desired key, ESC to clear");
    msgbox->hide();
}

void InputTable::draw_cell(TableContext context, int r, int c, int x, int y, int w, int h) {
    switch (context) {
        case CONTEXT_CELL: {
            fl_push_clip(x, y, w, h);

            // Background
            fl_color(row_selected(r) ? NstGreen : NstWhite);
            fl_rectf(x, y, w, h);

            // Text
            const char *defname = input_info[devicenum].defs[r];
            fl_color(row_selected(r) ? NstWhite : FL_BLACK);
            const char *text;
            if (c == 0) {
                text = defname;
            }
            else if (c == 1) {
                text = inputmgr.get_inputdef(input_info[devicenum].name, defname).c_str();
            }
            else if (c == 2) {
                text = inputmgr.get_inputdef(std::string(input_info[devicenum].name) + "j", defname).c_str();
            }

            fl_draw(text,
                    x, y, w, h,
                    c ? FL_ALIGN_LEFT : FL_ALIGN_CENTER);
            fl_pop_clip();

            return;
        }

        default: return;
    }
}
