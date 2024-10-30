#pragma once

#include <string>
#include <vector>

#include "jgmanager.h"
#include "setmanager.h"
#include "inputmanager.h"

class InputTable : public Fl_Table_Row {
public:
    InputTable(InputManager &im, std::vector<jg_inputinfo_t>& i,
               int x, int y, int w, int h, const char *l = 0)
            : inputmgr(im), input_info(i), Fl_Table_Row(x, y, w, h, l) {
        end();
    }

    int get_devicenum() { return devicenum; }
    void set_devicenum(int num) { devicenum = num; }

private:
    int handle(int e);

    int devicenum;

    InputManager& inputmgr;
    std::vector<jg_inputinfo_t>& input_info;

protected:
    void draw_cell(TableContext context, int r = 0, int c = 0,
                   int x = 0, int y = 0, int w = 0, int h = 0);
};

class NstSettingsWindow : public Fl_Double_Window {

public:
    NstSettingsWindow(int w, int h, const char* t, JGManager& j, SettingManager& m, InputManager& i);

    void set_choice_value(std::string tab, std::string label, int val);
    void set_crt_active(bool active);
    void show_inputmsg(int show);

private:
    void cb_chooser(Fl_Widget *w, void *data);
    static void cb_chooser_s(Fl_Widget *w, void *data);

    void cb_slider(Fl_Widget *w, void *data);
    static void cb_slider_s(Fl_Widget *w, void *data);

    void cb_ok(Fl_Widget *w, void *data);
    static void cb_ok_s(Fl_Widget *w, void *data);

    void cb_iselect(Fl_Widget *w, void *data);
    static void cb_iselect_s(Fl_Widget *w, void *data);

    void cb_itable(Fl_Widget *w, void *data);
    static void cb_itable_s(Fl_Widget *w, void *data);

    void populate(const std::vector<jg_setting_t*>& settings, bool input_settings = false);
    void populate_input();

    std::vector<jg_inputinfo_t> input_info;

    InputTable *itable;

    JGManager& jgm;
    SettingManager& setmgr;
    InputManager& inputmgr;
};

