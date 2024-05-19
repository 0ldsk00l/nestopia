#pragma once

#include "chtmanager.h"

class NstChtWindow : public Fl_Double_Window {

public:
    NstChtWindow(int w, int h, const char* t, CheatManager& chtmgr);
    void refresh();
    void populate();

private:
    void cb_del(Fl_Widget *w, void *data);
    static void cb_del_s(Fl_Widget *w, void *data);

    void cb_toggle(Fl_Widget *w, void *data);
    static void cb_toggle_s(Fl_Widget *w, void *data);

    void cb_add(Fl_Widget *w, void *data);
    static void cb_add_s(Fl_Widget *w, void *data);

    void cb_clear(Fl_Widget *w, void *data);
    static void cb_clear_s(Fl_Widget *w, void *data);

    void cb_load(Fl_Widget *w, void *data);
    static void cb_load_s(Fl_Widget *w, void *data);

    void cb_save(Fl_Widget *w, void *data);
    static void cb_save_s(Fl_Widget *w, void *data);

    void cb_table(Fl_Widget *w, long rn);
    static void cb_table_s(Fl_Widget *w, long rn);

    int rsel{0};

    CheatManager& chtmgr;
};
