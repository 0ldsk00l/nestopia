#pragma once

#include <string>

constexpr Fl_Color NstGreen = 0x255f6500;
constexpr Fl_Color NstPurple = 0x5f578700;
constexpr Fl_Color NstRed = 0xb51e2c00;
constexpr Fl_Color NstWhite = 0xffffff00;
constexpr Fl_Color NstBlueGrey = 0x383c4a00;
constexpr Fl_Color NstLightGrey = 0xd3dae300;
#ifdef __APPLE__
constexpr int UI_MBARHEIGHT = 0;
#else
constexpr int UI_MBARHEIGHT = 24;
#endif
constexpr int UI_SPACING = 24;
constexpr int UI_ELEMHEIGHT = 25;
constexpr int UI_ELEMWIDTH = 160;
constexpr int UI_DIAL_LG = 100;
constexpr int UI_DIAL_SM = 40;

class NstWindow : public Fl_Double_Window {
private:
    int handle(int e) override;

public:
    NstWindow(int w, int h, const char* t = 0) : Fl_Double_Window(w, h, t) {}
    void resize(int x, int y, int w, int h) override;
};

class NstGlArea : public Fl_Gl_Window {
private:
    void draw() override;
    int handle(int e) override;

public:
    NstGlArea(int x, int y, int w, int h, const char *l = 0) : Fl_Gl_Window(x, y, w, h, l) {}
};

class FltkUi {
public:
    static void enable_menu();
    static void rehash();
    static void set_ffspeed(bool on);
    static void show_inputmsg(int show);
    static void fullscreen(Fl_Widget *w = nullptr, void *data = nullptr);
    static void quit(Fl_Widget *w = nullptr, void *data = nullptr);
    static void about(Fl_Widget *w = nullptr, void *data = nullptr);
    static void about_close(Fl_Widget *w = nullptr, void *data = nullptr);
    static void rom_open(Fl_Widget *w = nullptr, void *data = nullptr);
    static void screenshot(std::string filename = "");
    static void screenshot_save(Fl_Widget *w = nullptr, void *data = nullptr);
    static void load_file(const char *filename);
    static void fds_next(Fl_Widget *w = nullptr, void *data = nullptr);
    static void fds_insert(Fl_Widget *w = nullptr, void *data = nullptr);
    static void state_load(Fl_Widget *w = nullptr, void *data = nullptr);
    static void state_save(Fl_Widget *w = nullptr, void *data = nullptr);
    static void state_qload(Fl_Widget *w = nullptr, void *data = nullptr);
    static void state_qsave(Fl_Widget *w = nullptr, void *data = nullptr);
    static void pause(Fl_Widget *w = nullptr, void *data = nullptr);
    static void reset(Fl_Widget *w = nullptr, void *data = nullptr);
    static void palette_open(Fl_Widget *w = nullptr, void *data = nullptr);
    static void setwin_open(Fl_Widget *w = nullptr, void *data = nullptr);
    static void chtwin_open(Fl_Widget *w = nullptr, void *data = nullptr);
    static void nstwin_open();
    static void run_emulation(bool run = true);
    static int handle(int e);
};
