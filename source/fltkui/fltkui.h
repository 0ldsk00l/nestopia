#pragma once

#include <string>

#include <video.h>

#define UI_MBARHEIGHT 24
#define UI_SPACING 24
#define UI_ELEMHEIGHT 25
#define UI_ELEMWIDTH 160
#define UI_DIAL_LG 100
#define UI_DIAL_SM 40

class NstWindow : public Fl_Double_Window {
private:
    int handle(int e);

public:
    NstWindow(int w, int h, const char* t = 0) : Fl_Double_Window(w, h, t) { }

    void resize(int x, int y, int w, int h);
};

class NstGlArea : public Fl_Gl_Window {
private:
    void draw() { nst_ogl_render(); }
    int handle(int e);

public:
    NstGlArea(int x, int y, int w, int h, const char *l = 0) : Fl_Gl_Window(x, y, w, h, l) {
        box(FL_DOWN_FRAME);
    }

    void resize(int x, int y, int w, int h);
};

constexpr Fl_Color NstGreen = 0x255f6500;
constexpr Fl_Color NstPurple = 0x5f578700;
constexpr Fl_Color NstRed = 0xb51e2c00;
constexpr Fl_Color NstWhite = 0xffffff00;
constexpr Fl_Color NstBlueGrey = 0x383c4a00;
constexpr Fl_Color NstLightGrey = 0xd3dae300;

void fltkui_rehash();
void fltkui_fullscreen(Fl_Widget *w = nullptr, void *data = nullptr);
void fltkui_enable_menu();
void fltkui_set_ffspeed(bool on);
void fltkui_show_msgbox(bool show);

std::string& fltkui_get_confpath();
