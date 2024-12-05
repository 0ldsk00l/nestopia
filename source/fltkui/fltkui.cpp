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

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>

#include <epoxy/gl.h>

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#ifdef __APPLE__
#include <FL/Fl_Sys_Menu_Bar.H>
#endif
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Table_Row.H>
#include <FL/gl.h>

#include <SDL.h>

#include "audiomanager.h"
#include "chtmanager.h"
#include "inputmanager.h"
#include "jgmanager.h"
#include "setmanager.h"
#include "videomanager.h"

#include "fltkui.h"
#include "fltkui_archive.h"
#include "fltkui_cheats.h"
#include "fltkui_settings.h"

#include "logdriver.h"

#include "version.h"

namespace {

int paused{0};
int speed{1};
int video_fullscreen{0};
int syncmode{0};
int refreshrate{60};
int screennum{0};

int frames{0};
int framefrags{0};

bool fdsgame{false};

NstWindow *nstwin{nullptr};
#ifdef __APPLE__
Fl_Sys_Menu_Bar *menubar{nullptr};
#else
Fl_Menu_Bar *menubar{nullptr};
#endif
NstGlArea *glarea{nullptr};
NstChtWindow *chtwin{nullptr};
NstSettingsWindow *setwin{nullptr};

JGManager *jgm{nullptr};
SettingManager *setmgr{nullptr};
InputManager *inputmgr{nullptr};
AudioManager *audiomgr{nullptr};
VideoManager *videomgr{nullptr};
CheatManager *chtmgr{nullptr};

std::vector<uint8_t> game;

Fl_Menu_Item menutable[] = {
    {"&File", FL_ALT + 'f', 0, 0, FL_SUBMENU},
        {"Open", 0, FltkUi::rom_open, 0, FL_MENU_DIVIDER},
        {"Load State...", 0, FltkUi::state_load, 0, FL_MENU_INACTIVE},
        {"Save State...", 0, FltkUi::state_save, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Quick Load", 0, 0, 0, FL_SUBMENU|FL_MENU_INACTIVE},
            {"Slot 0", 0, FltkUi::state_qload, (void*)"0", FL_MENU_INACTIVE},
            {"Slot 1", 0, FltkUi::state_qload, (void*)"1", FL_MENU_INACTIVE},
            {"Slot 2", 0, FltkUi::state_qload, (void*)"2", FL_MENU_INACTIVE},
            {"Slot 3", 0, FltkUi::state_qload, (void*)"3", FL_MENU_INACTIVE},
            {"Slot 4", 0, FltkUi::state_qload, (void*)"4", FL_MENU_INACTIVE},
            {0},
        {"Quick Save", 0, 0, 0, FL_SUBMENU|FL_MENU_DIVIDER|FL_MENU_INACTIVE},
            {"Slot 0", 0, FltkUi::state_qsave, (void*)"0", FL_MENU_INACTIVE},
            {"Slot 1", 0, FltkUi::state_qsave, (void*)"1", FL_MENU_INACTIVE},
            {"Slot 2", 0, FltkUi::state_qsave, (void*)"2", FL_MENU_INACTIVE},
            {"Slot 3", 0, FltkUi::state_qsave, (void*)"3", FL_MENU_INACTIVE},
            {"Slot 4", 0, FltkUi::state_qsave, (void*)"4", FL_MENU_INACTIVE},
            {0},
        {"Open Palette...", 0, FltkUi::palette_open, 0, FL_MENU_DIVIDER},
        {"Screenshot...", 0, FltkUi::screenshot_save, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        #ifndef __APPLE__
        {"&Quit", FL_ALT + 'q', FltkUi::quit, 0, 0},
        #endif
        {0}, // End File
    {"&Emulator", FL_ALT + 'e', 0, 0, FL_SUBMENU},
        {"Pause", 0, FltkUi::pause, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Reset (Soft)", 0, FltkUi::reset, (void*)"0", FL_MENU_INACTIVE},
        {"Reset (Hard)", 0, FltkUi::reset, (void*)"1", FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Fullscreen", 0, FltkUi::fullscreen, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Switch Disk Side", 0, FltkUi::fds_next, 0, FL_MENU_INACTIVE},
        {"Insert/Eject Disk", 0, FltkUi::fds_insert, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Cheats...", 0, FltkUi::chtwin_open, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Settings...", 0, FltkUi::setwin_open, 0, 0},
        {0}, // End Emulator
    #ifndef __APPLE__
    {"&Help", FL_ALT + 'h', 0, 0, FL_SUBMENU},
        {"About", 0, FltkUi::about, 0, 0},
        {0}, // End Help
    #endif
    {0} // End Menu
};

Fl_Menu_Item *get_menuitem(std::string label) {
    Fl_Menu_Item *m = menutable->first();
    for (int i = 0; i < menutable->size(); ++i) {
        if (m->label() != nullptr && std::string(m->label()) == label) {
            return m;
        }
        ++m;
    }
    return nullptr;
}

void update_refreshrate(void) {
    // Get the screen refresh rate using an SDL window
    if (syncmode) { // Don't use this in "Timer" sync mode
        return;
    }
    #ifdef __APPLE__
    refreshrate = 120; // Dirty hack for modern macOS
    return;
    #endif
    SDL_Window *sdlwin;
    sdlwin = SDL_CreateWindow(
        "refreshrate",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1, 1, SDL_WINDOW_HIDDEN
    );
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(screennum, &dm);
    refreshrate = dm.refresh_rate;
    SDL_DestroyWindow(sdlwin);
}

void exec_emu_vsync(void*) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        inputmgr->event(event);
    }

    int fps{jgm->get_frametime()};
    frames = (fps / refreshrate);
    framefrags += fps % refreshrate;

    if (framefrags >= refreshrate) {
        frames++;
        framefrags -= refreshrate;
    }

    if (!paused) {
        for (int i = 0; i < frames * speed; i++) {
            jgm->exec_frame();
        }
    }

    glarea->redraw();
}

void exec_emu_timer(void*) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        inputmgr->event(event);
    }

    if (!paused) {
        for (int i = 0; i < speed; i++) {
            jgm->exec_frame();
        }
    }

    Fl::repeat_timeout(1.0 / jgm->get_frametime(), exec_emu_timer);
    glarea->redraw();
}

}

void FltkUi::chtwin_open(Fl_Widget *w, void *data) {
    if (!jgm->is_loaded()) {
        return;
    }

    chtwin->refresh();
    chtwin->show();
}

void FltkUi::setwin_open(Fl_Widget *w, void *data) {
    setwin->show();
}

void FltkUi::load_file(const char *filename) {
    // First, check if it's an archive
    std::set<std::string> archive_exts = { ".zip", ".7z", ".gz", ".bz2", ".xz", ".zst" };
    std::string fileext = std::filesystem::path(filename).extension().string();
    std::transform(fileext.begin(), fileext.end(), fileext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (archive_exts.find(fileext) != archive_exts.end()) {
        std::string arcname{};
        if (fltkui_archive_select(filename, arcname)) {
            if (arcname.empty()) {
                return;
            }

            game.clear();
            fltkui_archive_load_file(filename, arcname, game);
            fileext = std::filesystem::path(arcname).extension().string();
            jgm->load_game(arcname.c_str(), game);
        }
        else {
            LogDriver::log(LogLevel::OSD, "No valid files in archive");
        }
    }
    else {
        std::ifstream stream(filename, std::ios::in | std::ios::binary);

        if (!stream.is_open()) {
            return;
        }

        game.clear();
        game = std::vector<uint8_t>(std::istreambuf_iterator<char>(stream),
                                    std::istreambuf_iterator<char>());
        stream.close();

        jgm->load_game(filename, game);
    }

    std::transform(fileext.begin(), fileext.end(), fileext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    fdsgame = fileext == ".fds";
}

void FltkUi::rom_open(Fl_Widget *w, void *data) {
    // Create native chooser
    Fl_Native_File_Chooser fc;
    fc.title("Select a ROM");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.filter("NES Games\t*.{nes,unf,fds,bin,zip,7z,gz,bz2,xz,xml,zst}");

    run_emulation(false);

    // Show file chooser
    switch (fc.show()) {
        case -1:
            LogDriver::log(LogLevel::Error, std::string(fc.errmsg()));
            break;
        case 1: break; // Cancel
        default:
            if (fc.filename()) {
                load_file(fc.filename());

                if (jgm->is_loaded()) {
                    chtmgr->clear();
                    chtwin->refresh();
                    FltkUi::enable_menu();
                    nstwin->label(jgm->get_gamename().c_str());
                    jgm->setup_audio();
                    jgm->setup_video();
                    inputmgr->reassign();
                    audiomgr->unpause();
                }
            }
            break;
    }

    run_emulation();
}

void FltkUi::screenshot(std::string filename) {
    if (filename.empty()) {
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::system_clock::from_time_t(0);
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                                                  (now.time_since_epoch()).count();
        std::string msecs = std::to_string(duration);
        filename = jgm->get_basepath() + "/screenshots/" + msecs + ".png";
    }
    videomgr->screenshot(filename);
    LogDriver::log(LogLevel::OSD, "Screenshot Saved");
}

void FltkUi::screenshot_save(Fl_Widget *w, void *data) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    run_emulation(false);

    Fl_Native_File_Chooser fc;
    fc.title("Save Screenshot");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.filter("Screenshots\t*.png");
    fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

    // Show file chooser
    if (fc.show()) {
        return;
    }

    screenshot(fc.filename());
    run_emulation();
}

void FltkUi::state_load(Fl_Widget *w, void *userdata) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    run_emulation(false);

    Fl_Native_File_Chooser fc;
    fc.title("Load State");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    //fc.directory((const char*)nstpaths.statepath);
    fc.filter("Nestopia States\t*.nst");

    // Show file chooser
    switch (fc.show()) {
        case -1:
            LogDriver::log(LogLevel::Error, std::string(fc.errmsg()));
            break;
        case 1: break; // Cancel
        default:
            if (fc.filename()) {
                std::string statefile{fc.filename()};
                jgm->state_load(statefile);
            }
            break;
    }

    run_emulation();
}

void FltkUi::state_save(Fl_Widget *w, void *data) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    run_emulation(false);

    Fl_Native_File_Chooser fc;
    fc.title("Save State");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.filter("Nestopia States\t*.nst");
    fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

    // Show file chooser
    if (fc.show()) {
        run_emulation();
        return;
    }

    std::string statefile{fc.filename()};
    jgm->state_save(statefile);
    run_emulation();
}

void FltkUi::palette_open(Fl_Widget *w, void *data) {
    // Create native chooser
    Fl_Native_File_Chooser fc;
    fc.title("Select a Palette");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.filter("NES Palettes\t*.pal");

    // Show file chooser
    switch (fc.show()) {
        case -1:
            LogDriver::log(LogLevel::Error, std::string(fc.errmsg()));
            break;
        case 1: break; // Cancel
        default:
            if (fc.filename()) {
                // Overwrite the custom.pal file with the loaded one
                std::filesystem::path src = fc.filename();
                std::filesystem::path dst = jgm->get_basepath() + "/custom.pal";
                std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);

                // Force custom palette
                jgm->get_setting("palette")->val = 11;
                jgm->rehash();
                setwin->set_choice_value("Emulator", "Palette", 11);
            }
            break;
    }
}

void FltkUi::state_qload(Fl_Widget *w, void *data) {
    int slot = atoi((const char*)data);
    jgm->state_qload(slot);
}

void FltkUi::state_qsave(Fl_Widget *w, void *data) {
    int slot = atoi((const char*)data);
    jgm->state_qsave(slot);
}

void FltkUi::pause(Fl_Widget *w, void *data) {
    Fl_Menu_Item* m = nullptr;

    m = w ? const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)w)->mvalue()) :
            get_menuitem(paused ? "Play" : "Pause");

    if (m == nullptr) {
        LogDriver::log(LogLevel::Warn, "Menu item does not exist");
        return;
    }

    paused ^= 1;

    if (paused) {
        audiomgr->pause();
    }
    else {
        audiomgr->unpause();
    }

    m->label(paused ? "Play" : "Pause");
    #ifdef __APPLE__
    menubar->update();
    #endif
}

void FltkUi::reset(Fl_Widget *w, void *data) {
    jgm->reset(atoi((const char*)data));
}

void NstWindow::resize(int x, int y, int w, int h) {
    Fl_Double_Window::resize(x, y, w, h);

    int nscreennum = Fl::screen_num(x, y, w, h);
    if (nscreennum != screennum) { // Window moved to a different screen
        screennum = nscreennum;
        update_refreshrate();
    }

    videomgr->set_dpiscale(glarea->pixels_per_unit());

    if (video_fullscreen) {
        glarea->resize(0, 0, w, h);
        videomgr->resize(w, h);
    }
    else {
        glarea->resize(0, UI_MBARHEIGHT, w, h - UI_MBARHEIGHT);
        videomgr->resize(w, h - UI_MBARHEIGHT);
    }
}

void FltkUi::rehash() {
    LogDriver::set_level(setmgr->get_setting("l_loglevel")->val);
    videomgr->rehash(true);
    audiomgr->rehash();
}

void FltkUi::fullscreen(Fl_Widget *w, void *data) {
    if (!jgm->is_loaded()) {
        return;
    }

    video_fullscreen ^= 1;

    if (video_fullscreen) {
        menubar->hide();
        nstwin->fullscreen();
    }
    else {
        menubar->show();
        nstwin->fullscreen_off();
    }
}

void FltkUi::fds_next(Fl_Widget *w, void *data) {
    jgm->media_select();
}

void FltkUi::fds_insert(Fl_Widget *w, void *data) {
    jgm->media_insert();
}

void FltkUi::about_close(Fl_Widget *w, void *data) {
    Fl_Window *about = (Fl_Window*)data;
    about->hide();
    run_emulation();
}

void FltkUi::about(Fl_Widget *w, void *data) {
    Fl_Window about(460, 440);
    Fl_Box iconbox(166, 16, 128, 128);

    Fl_Box text0(0, 144, 460, UI_SPACING, "Nestopia UE");
    text0.labelfont(FL_BOLD);

    Fl_Box text1(0, 166, 460, UI_SPACING, JG_VERSION);

    Fl_Box text2(0, 208, 460, UI_SPACING, "Cycle-Accurate Nintendo Entertainment System Emulator");

    Fl_Box text3(0, 256, 460, UI_SPACING,
                 "FLTK Frontend\n(c) 2012-2024, R. Danbrook\n\n"
                 "Portions derived from The Jolly Good Reference Frontend\n"
                 "(c) 2020-2024, Rupert Carmichael\n");
    text3.labelsize(10);

    Fl_Box text4(0, 320, 460, UI_SPACING,
                 "Nestopia Emulator\n"
                 "(c) 2020-2024, Rupert Carmichael\n"
                 "(c) 2012-2020, Nestopia UE Contributors\n"
                 "(c) 2003-2008, Martin Freij");
    text4.labelsize(10);

    Fl_Box text5(0, 360, 460, UI_SPACING, "Icon based on drawing by Trollekop");
    text5.labelsize(10);

    // Set up the icon
    std::string iconpath{"icons/128/nestopia.png"};
    if (!std::filesystem::exists(std::filesystem::path{iconpath})) {
        iconpath = std::string(NST_DATAROOTDIR) + "/icons/hicolor/128x128/apps/nestopia.png";
    }

    Fl_PNG_Image nsticon(iconpath.c_str());
    iconbox.image(nsticon);

    Fl_Button close(360, 400, 80, UI_SPACING, "&Close");
    close.shortcut(FL_ALT + 'c');
    close.callback(FltkUi::about_close, (void*)&about);

    about.set_modal();
    run_emulation(false);
    about.show();
    while (about.shown()) {
        Fl::wait();
    }
}

void FltkUi::quit(Fl_Widget *w, void *data) {
    videomgr->renderer_deinit();
    nstwin->hide();
}

int FltkUi::handle(int e) {
    // This is used to stop Esc from exiting the program
    return (e == FL_SHORTCUT); // eat all keystrokes
}

int NstWindow::handle(int e) {
    switch (e) {
        case FL_KEYDOWN: case FL_KEYUP: {
            #ifdef __APPLE__
            inputmgr->event(Fl::event_key(), e == FL_KEYDOWN);
            #else
            // GNOME has a bad habit of spamming keyup events while holding
            // a button down, so Fl::get_key is required here.
            int key = Fl::event_key();
            bool down = Fl::get_key(key);
            inputmgr->event(key, down);
            #endif

            if (jgm->is_loaded()) {
                inputmgr->ui_events();
            }
            break;
        }
    }
    return Fl_Double_Window::handle(e);
}

void NstGlArea::draw() {
    videomgr->render();
}

int NstGlArea::handle(int e) {
    int xc, yc;
    switch (e) {
        case FL_ENTER:
            if (inputmgr->get_lightgun()) {
                cursor(setmgr->get_setting("m_hidecrosshair")->val ? FL_CURSOR_NONE : FL_CURSOR_CROSS);
            }
            else if (setmgr->get_setting("m_hidecursor")->val) {
                cursor(FL_CURSOR_NONE);
            }
            break;
        case FL_LEAVE:
            cursor(FL_CURSOR_DEFAULT);
            break;
        case FL_PUSH:
            inputmgr->event(Fl::event_button() + 1000, true);
            return 1; // Must return 1 to receive drag events
        case FL_RELEASE:
            inputmgr->event(Fl::event_button() + 1000, false);
            break;
        case FL_MOVE:
            videomgr->get_scaled_coords(Fl::event_x(), Fl::event_y(), &xc, &yc);
            inputmgr->event(xc, yc);
            break;
        case FL_DRAG:
            videomgr->get_scaled_coords(Fl::event_x(), Fl::event_y(), &xc, &yc);
            inputmgr->event(xc, yc);
            inputmgr->event(Fl::event_button() + 1000, Fl::event_state() ? true : false);
            break;
        case FL_DND_ENTER: // return 1 for these events to accept Drag and Drop
        case FL_DND_DRAG:
        case FL_DND_RELEASE:
            return 1;
        case FL_PASTE: { // handle the actual drop event
            std::string filepath{std::regex_replace(std::string(Fl::event_text()),
                                                    std::regex("\\n|file://"), "")};
            FltkUi::load_file(filepath.c_str());
            if (jgm->is_loaded()) {
                FltkUi::enable_menu();
                nstwin->label(jgm->get_gamename().c_str());
                jgm->setup_audio();
                jgm->setup_video();
                inputmgr->reassign();
                audiomgr->unpause();
                // Restart if in timer sync mode
                FltkUi::run_emulation(false);
                FltkUi::run_emulation();
            }
            return 1;
        }
    }

    return Fl_Gl_Window::handle(e);
}

void FltkUi::enable_menu() {
    Fl_Menu_Item *mtable = (Fl_Menu_Item*)menubar->menu();
    for (int i = 0; i < mtable[0].size(); ++i) {
        if (!fdsgame && mtable[i].label() &&
            std::string(mtable[i].label()).find("Disk") != std::string::npos) {
            mtable[i].deactivate();
        }
        else {
            mtable[i].activate();
        }
    }
    #ifdef __APPLE__
    menubar->update();
    #endif
}

void FltkUi::show_inputmsg(int show) {
    setwin->show_inputmsg(show);
}

void FltkUi::nstwin_open() {
    int rw, rh;
    videomgr->set_dimensions();
    videomgr->get_dimensions(&rw, &rh);

    Fl::add_handler(FltkUi::handle);

    // Cheats Window
    chtwin = new NstChtWindow(720, 500, "Cheat Manager", *chtmgr);
    chtwin->populate();

    // Settings Window
    setwin = new NstSettingsWindow(500, 550, "Settings", *jgm, *setmgr, *inputmgr);
    setwin->set_crt_active(setmgr->get_setting("v_postproc")->val == 3);

    // Main Window
    nstwin = new NstWindow(rw, rh + UI_MBARHEIGHT);
    nstwin->color(FL_BLACK);
    nstwin->xclass("nestopia");

    #ifdef __APPLE__
    // Apple style menu bar (top of screen)
    menubar = new Fl_Sys_Menu_Bar(0, 0, nstwin->w(), UI_MBARHEIGHT);
    // Set the "About" callback and "Window" menu style
    Fl_Sys_Menu_Bar::about(about, nullptr);
    Fl_Sys_Menu_Bar::window_menu_style(Fl_Sys_Menu_Bar::tabbing_mode_none);
    #else
    // Normal menu bar and window icon
    menubar = new Fl_Menu_Bar(0, 0, nstwin->w(), UI_MBARHEIGHT);

    // Set up the window icon
    std::string iconpath{"icons/96/nestopia.png"};
    if (!std::filesystem::exists(std::filesystem::path{iconpath})) {
        iconpath = std::string(NST_DATAROOTDIR) + "/icons/hicolor/96x96/apps/nestopia.png";
    }

    Fl_PNG_Image nsticon(iconpath.c_str());
    nstwin->default_icon(&nsticon);
    #endif

    menubar->box(FL_FLAT_BOX);
    menubar->menu(menutable);
    menubar->selection_color(NstGreen);

    glarea = new NstGlArea(0, UI_MBARHEIGHT, nstwin->w(), nstwin->h() - UI_MBARHEIGHT);
    nstwin->resizable(glarea);
    glarea->color(FL_BLACK);
    #ifdef __APPLE__
    Fl::use_high_res_GL(1);
    glarea->mode(FL_RGB | FL_RGB8 | FL_INDEX | FL_DOUBLE | FL_ACCUM |
                 FL_ALPHA | FL_DEPTH | FL_STENCIL |
                 (setmgr->get_setting("v_renderer")->val ? 0 : FL_OPENGL3));
    #endif
    glarea->end();

    nstwin->end();
}

void FltkUi::set_ffspeed(bool on) {
    if (on) {
        speed = setmgr->get_setting("m_ffspeed")->val;
    }
    else {
        speed = 1;
    }

    audiomgr->set_speed(speed);
}

void FltkUi::run_emulation(bool run) {
    if (run) {
        if (syncmode) {
            Fl::add_timeout(0.001, exec_emu_timer);
        }
        else {
            Fl::add_idle(exec_emu_vsync);
        }
    }
    else {
        if (syncmode) {
            Fl::remove_timeout(exec_emu_timer);
        }
        else {
            Fl::remove_idle(exec_emu_vsync);
        }
    }
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    std::string filename{};
    std::vector<std::string> flags{};
    for (int i = 1; i < argc; ++i) {
        if (filename.empty() && argv[i][0] != '-') {
            // The first non-flag argument is considered the filename
            filename = std::string{argv[i]};
        }
        else if (argv[i][0] == '-') {
            flags.push_back(std::string{argv[i]});
        }
    }

    // Set default config options
    setmgr = new SettingManager();

    // Initialize SDL Audio and Joystick
    #ifdef _WIN32
    putenv("SDL_AUDIODRIVER=directsound");
    #endif
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        LogDriver::log(LogLevel::Error, "Failed to initialize SDL: " + std::string(SDL_GetError()));
        return 1;
    }

    jgm = new JGManager();

    // Read frontend and emulator settings
    setmgr->read(*jgm);
    LogDriver::set_level(setmgr->get_setting("l_loglevel")->val);

    // Bring up Audio/Video managers
    audiomgr = new AudioManager(*jgm, *setmgr);
    videomgr = new VideoManager(*jgm, *setmgr);

    inputmgr = new InputManager(*jgm, *setmgr);
    chtmgr = new CheatManager(*jgm);

    // Load a rom from the command line
    if (!filename.empty()) {
        FltkUi::load_file(filename.c_str());
        if (jgm->is_loaded()) {
            jgm->setup_audio();
            jgm->setup_video();
            inputmgr->reassign();
            video_fullscreen = setmgr->get_setting("v_fullscreen")->val ||
                               std::find(flags.begin(), flags.end(), "-f") != flags.end() ||
                               std::find(flags.begin(), flags.end(), "--fullscreen") != flags.end();
            audiomgr->unpause();
        }
    }

    FltkUi::nstwin_open();
    screennum = Fl::screen_num(nstwin->x_root(), nstwin->y_root());

    if (jgm->is_loaded()) {
        nstwin->label(jgm->get_gamename().c_str());
        FltkUi::enable_menu();
    }
    else {
        nstwin->label("Nestopia UE");
        if (!filename.empty()) {
            LogDriver::log(LogLevel::OSD, "Failed to load file from CLI");
        }
    }

    nstwin->show();
    menubar->show();

    glarea->make_current();
    glarea->show();
    videomgr->renderer_init();
    videomgr->set_dpiscale(glarea->pixels_per_unit());
    videomgr->resize(glarea->w(), glarea->h());

    if (video_fullscreen) {
        video_fullscreen = 0;
        FltkUi::fullscreen(NULL, NULL);
    }

    syncmode = setmgr->get_setting("m_syncmode")->val;
    LogDriver::log(LogLevel::Debug, syncmode ?
                   "Synchronization Mode: Timer" :
                   "Synchronization Mode: VSync");

    update_refreshrate();

    FltkUi::run_emulation();
    while (nstwin->shown()) {
        Fl::wait();
    }

    // Write frontend and emulator settings
    setmgr->write(*jgm);

    if (audiomgr) {
        delete audiomgr;
    }

    if (videomgr) {
        delete videomgr;
    }

    if (inputmgr) {
        delete inputmgr;
    }

    if (jgm) {
        delete jgm;
    }

    if (setmgr) {
        delete setmgr;
    }

    if (chtmgr) {
        delete chtmgr;
    }

    delete chtwin;
    delete setwin;
    delete glarea;
    delete menubar;
    delete nstwin;

    return 0;
}
