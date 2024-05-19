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
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Table_Row.H>
#include <FL/gl.h>

#include <SDL.h>

#include "jgmanager.h"
#include "setmanager.h"
#include "inputmanager.h"
#include "chtmanager.h"

#include "audio.h"
#include "video.h"

#include "fltkui.h"
#include "fltkui_archive.h"
#include "fltkui_cheats.h"
#include "fltkui_settings.h"

static int paused = 0;
static int speed = 1;
static int video_fullscreen = 0;

static NstWindow *nstwin;
static Fl_Menu_Bar *menubar;
static NstGlArea *glarea;
static NstChtWindow *chtwin;
static NstSettingsWindow *setwin;

static JGManager *jgm = nullptr;
static SettingManager *setmgr = nullptr;
static InputManager *inputmgr = nullptr;
static CheatManager *chtmgr = nullptr;

static std::vector<uint8_t> game;

static int fltkui_refreshrate(void) {
    // Get the screen refresh rate using an SDL window
    int refresh = 60;
    SDL_Window *sdlwin;
    sdlwin = SDL_CreateWindow(
        "refreshrate",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1, 1, SDL_WINDOW_HIDDEN
    );
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(sdlwin), &dm);
    refresh = dm.refresh_rate;
    SDL_DestroyWindow(sdlwin);
    return refresh;
}

static void fltkui_cheats(Fl_Widget* w, void* userdata) {
    if (!jgm->is_loaded()) {
        return;
    }

    chtwin->refresh();
    chtwin->show();
}

static void fltkui_settings(Fl_Widget* w, void* userdata) {
    setwin->show();
}

static void fltkui_load_file(const char *filename) {
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
            jgm->load_game(arcname.c_str(), game);
        }
        else {
            nst_video_print("No valid files in archive", 8, 212, 2, true);
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
}

static void fltkui_rom_open(Fl_Widget* w, void* userdata) {
    // Create native chooser
    Fl_Native_File_Chooser fc;
    fc.title("Select a ROM");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.filter("NES Games\t*.{nes,unf,fds,zip,7z,gz,bz2,xz,zst}");

    // Show file chooser
    switch (fc.show()) {
        case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
        case 1: break; // Cancel
        default:
            if (fc.filename()) {
                fltkui_load_file(fc.filename());

                if (jgm->is_loaded()) {
                    chtmgr->clear();
                    chtwin->refresh();
                    fltkui_enable_menu();
                    nstwin->label(jgm->get_gamename().c_str());
                    jg_setup_audio();
                    jg_setup_video();
                    inputmgr->reassign();
                }
            }
            break;
    }
}

static void fltkui_movie_load(Fl_Widget* w, void* userdata) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    Fl_Native_File_Chooser fc;
    fc.title("Select a Movie");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    //fc.directory((const char*)nstpaths.nstdir);
    fc.filter("Nestopia Movies\t*.nsv");

    // Show file chooser
    switch (fc.show()) {
        case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
        case 1: break; // Cancel
        default:
            if (fc.filename()) {
                //nst_movie_load(fc.filename());
            }
            break;
    }
}

static void fltkui_movie_save(Fl_Widget* w, void* userdata) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    Fl_Native_File_Chooser fc;
    fc.title("Save Movie");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    //fc.directory((const char*)nstpaths.nstdir);
    fc.filter("Nestopia Moviess\t*.nsv");
    fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

    // Show file chooser
    if (fc.show()) { return; }

    //nst_movie_save(fc.filename());
}

static void fltkui_movie_stop(Fl_Widget* w, void* userdata) {
    //nst_movie_stop();
}

static void fltkui_state_load(Fl_Widget* w, void* userdata) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    Fl_Native_File_Chooser fc;
    fc.title("Load State");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    //fc.directory((const char*)nstpaths.statepath);
    fc.filter("Nestopia States\t*.nst");

    // Show file chooser
    switch (fc.show()) {
        case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
        case 1: break; // Cancel
        default:
            if (fc.filename()) {
                std::string statefile{fc.filename()};
                jgm->state_load(statefile);
            }
            break;
    }
}

static void fltkui_state_save(Fl_Widget* w, void* userdata) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    Fl_Native_File_Chooser fc;
    fc.title("Save State");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.filter("Nestopia States\t*.nst");
    fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

    // Show file chooser
    if (fc.show()) {
        return;
    }

    std::string statefile{fc.filename()};
    jgm->state_save(statefile);
}

static void fltkui_screenshot(Fl_Widget* w, void* userdata) {
    // Create native chooser
    if (!jgm->is_loaded()) {
        return;
    }

    Fl_Native_File_Chooser fc;
    fc.title("Save Screenshot");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.filter("PNG Screenshots\t*.png");
    fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

    // Show file chooser
    if (fc.show()) {
        return;
    }

    video_screenshot(fc.filename());
}

static void fltkui_palette_open(Fl_Widget* w, void* userdata) {
    // Create native chooser
    Fl_Native_File_Chooser fc;
    fc.title("Select a Palette");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.filter("NES Palettes\t*.pal");

    // Show file chooser
    switch (fc.show()) {
        case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
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

static void fltkui_state_qload(Fl_Widget* w, void* userdata) {
    int slot = atoi((const char*)userdata);
    jgm->state_qload(slot);
}

static void fltkui_state_qsave(Fl_Widget* w, void* userdata) {
    int slot = atoi((const char*)userdata);
    jgm->state_qsave(slot);
}

static void fltkui_pause(Fl_Widget* w, void* userdata) {
    paused ^= 1;
    if (paused) {
        audio_pause();
        Fl_Menu_Item* m = const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)w)->mvalue());
        m->label("Play");
    }
    else {
        audio_unpause();
        Fl_Menu_Item* m = const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)w)->mvalue());
        m->label("Pause");
    }
}

static void fltkui_reset(Fl_Widget* w, void* userdata) {
    jgm->reset(atoi((const char*)userdata));
}

void NstWindow::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
    menubar->resize(0, 0, w, UI_MBARHEIGHT);

    if (video_fullscreen) {
        glarea->resize(0, 0, w, h);
    }
    else {
        glarea->resize(0, UI_MBARHEIGHT, w, h - UI_MBARHEIGHT);
    }
}

void NstGlArea::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
    nst_video_resize(w, h);
}

void fltkui_fullscreen(Fl_Widget *w, void *data) {
    if (!jgm->is_loaded()) {
        return;
    }

    video_fullscreen ^= 1;

    if (video_fullscreen) {
        int x, y, w, h;
        Fl::screen_xywh(x, y, w, h);
        menubar->hide();
        nstwin->fullscreen();
    }
    else {
        int rw, rh;
        nst_video_dimensions(&rw, &rh);
        nstwin->fullscreen_off();
        menubar->show();
    }
}

static void fltkui_fds_next(Fl_Widget* w, void* userdata) {
    jg_media_select();
}

static void fltkui_fds_insert(Fl_Widget* w, void* userdata) {
    jg_media_insert();
}

static void fltkui_about_close(Fl_Widget* w, void* userdata) {
    Fl_Window *about = (Fl_Window*)userdata;
    about->hide();
}

static void fltkui_about(Fl_Widget* w, void* userdata) {
    Fl_Window about(460, 440);
    Fl_Box iconbox(166, 16, 128, 128);

    Fl_Box text0(0, 144, 460, UI_SPACING, "Nestopia UE");
    text0.labelfont(FL_BOLD);

    Fl_Box text1(0, 166, 460, UI_SPACING, "1.52.1");

    Fl_Box text2(0, 208, 460, UI_SPACING, "Cycle-Accurate Nintendo Entertainment System Emulator");

    Fl_Box text3(0, 256, 460, UI_SPACING, "FLTK Frontend\n(c) 2012-2024, R. Danbrook");
    text3.labelsize(10);

    Fl_Box text4(0, 320, 460, UI_SPACING, "Nestopia Emulator\n(c) 2020-2024, Rupert Carmichael\n(c) 2012-2020, Nestopia UE Contributors\n(c) 2003-2008, Martin Freij");
    text4.labelsize(10);

    Fl_Box text5(0, 360, 460, UI_SPACING, "Icon based on drawing by Trollekop");
    text5.labelsize(10);

    // Set up the icon
    char iconpath[512];
    snprintf(iconpath, sizeof(iconpath), "%s/icons/hicolor/128x128/apps/nestopia.png", NST_DATAROOTDIR);
    // Load the SVG from local source dir if make install hasn't been done
    struct stat svgstat;
    if (stat(iconpath, &svgstat) == -1) {
        snprintf(iconpath, sizeof(iconpath), "icons/128/nestopia.png");
    }

    Fl_PNG_Image nsticon(iconpath);
    iconbox.image(nsticon);

    Fl_Button close(360, 400, 80, UI_SPACING, "&Close");
    close.shortcut(FL_ALT + 'c');
    close.callback(fltkui_about_close, (void*)&about);

    about.set_modal();
    about.show();
    while (about.shown()) { Fl::wait(); }
}

static void quit_cb(Fl_Widget* w, void* userdata) {
    nstwin->hide();
}

// this is used to stop Esc from exiting the program:
int handle(int e) {
    return (e == FL_SHORTCUT); // eat all keystrokes
}

int NstWindow::handle(int e) {
    switch (e) {
        case FL_KEYDOWN: case FL_KEYUP:
            inputmgr->event(Fl::event_key(), e == FL_KEYDOWN);
            if (jgm->is_loaded()) {
                inputmgr->ui_events();
            }
            break;
    }
    return Fl_Double_Window::handle(e);
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
            video_scaled_coords(Fl::event_x(), Fl::event_y(), &xc, &yc);
            inputmgr->event(xc, yc);
            break;
        case FL_DRAG:
            video_scaled_coords(Fl::event_x(), Fl::event_y(), &xc, &yc);
            inputmgr->event(xc, yc);
            inputmgr->event(Fl::event_button() + 1000, Fl::event_state() ? true : false);
            break;
    }

    return Fl_Gl_Window::handle(e);
}

static Fl_Menu_Item menutable[] = {
    {"&File", FL_ALT + 'f', 0, 0, FL_SUBMENU},
        {"Open", 0, fltkui_rom_open, 0, FL_MENU_DIVIDER},
        {"Load State...", 0, fltkui_state_load, 0, FL_MENU_INACTIVE},
        {"Save State...", 0, fltkui_state_save, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Quick Load", 0, 0, 0, FL_SUBMENU|FL_MENU_INACTIVE},
            {"Slot 0", 0, fltkui_state_qload, (void*)"0", FL_MENU_INACTIVE},
            {"Slot 1", 0, fltkui_state_qload, (void*)"1", FL_MENU_INACTIVE},
            {"Slot 2", 0, fltkui_state_qload, (void*)"2", FL_MENU_INACTIVE},
            {"Slot 3", 0, fltkui_state_qload, (void*)"3", FL_MENU_INACTIVE},
            {"Slot 4", 0, fltkui_state_qload, (void*)"4", FL_MENU_INACTIVE},
            {0},
        {"Quick Save", 0, 0, 0, FL_SUBMENU|FL_MENU_DIVIDER|FL_MENU_INACTIVE},
            {"Slot 0", 0, fltkui_state_qsave, (void*)"0", FL_MENU_INACTIVE},
            {"Slot 1", 0, fltkui_state_qsave, (void*)"1", FL_MENU_INACTIVE},
            {"Slot 2", 0, fltkui_state_qsave, (void*)"2", FL_MENU_INACTIVE},
            {"Slot 3", 0, fltkui_state_qsave, (void*)"3", FL_MENU_INACTIVE},
            {"Slot 4", 0, fltkui_state_qsave, (void*)"4", FL_MENU_INACTIVE},
            {0},
        {"Open Palette...", 0, fltkui_palette_open, 0, FL_MENU_DIVIDER},
        {"Screenshot...", 0, fltkui_screenshot, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Load Movie...", 0, fltkui_movie_load, 0, FL_MENU_INACTIVE},
        {"Record Movie...", 0, fltkui_movie_save, 0, FL_MENU_INACTIVE},
        {"Stop Movie", 0, fltkui_movie_stop, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"&Quit", FL_ALT + 'q', quit_cb, 0, 0},
        {0}, // End File
    {"&Emulator", FL_ALT + 'e', 0, 0, FL_SUBMENU},
        {"Pause", 0, fltkui_pause, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Reset (Soft)", 0, fltkui_reset, (void*)"0", FL_MENU_INACTIVE},
        {"Reset (Hard)", 0, fltkui_reset, (void*)"1", FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Fullscreen", 0, fltkui_fullscreen, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Switch Disk Side", 0, fltkui_fds_next, 0, FL_MENU_INACTIVE},
        {"Insert/Eject Disk", 0, fltkui_fds_insert, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Cheats...", 0, fltkui_cheats, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE},
        {"Settings...", 0, fltkui_settings, 0, 0},
        {0}, // End Emulator
    {"&Help", FL_ALT + 'h', 0, 0, FL_SUBMENU},
        {"About", 0, fltkui_about, 0, 0},
        {0}, // End Help
    {0} // End Menu
};

void fltkui_enable_menu() {
    for (int i = 0; i < menutable[0].size(); ++i) {
        menutable[i].activate();
    }
}

void fltkui_show_msgbox(bool show) {
    setwin->show_msgbox(show);
}

void makenstwin(const char *name) {
    int rw, rh;
    nst_video_dimensions(&rw, &rh);

    Fl::add_handler(handle);

    // Cheats Window
    chtwin = new NstChtWindow(720, 500, "Cheat Manager", *chtmgr);
    chtwin->populate();

    // Settings Window
    setwin = new NstSettingsWindow(500, 500, "Settings", *jgm, *setmgr, *inputmgr);

    // Main Window
    nstwin = new NstWindow(rw, rh + UI_MBARHEIGHT, name);
    nstwin->color(FL_BLACK);
    nstwin->xclass("nestopia");

    nstwin->begin();

    // Menu Bar
    menubar = new Fl_Menu_Bar(0, 0, nstwin->w(), UI_MBARHEIGHT);
    menubar->box(FL_FLAT_BOX);
    menubar->menu(menutable);
    menubar->selection_color(NstGreen);

    glarea = new NstGlArea(0, UI_MBARHEIGHT, nstwin->w(), nstwin->h() - UI_MBARHEIGHT);
    glarea->color(FL_BLACK);

    nstwin->end();
}

void fltkui_set_ffspeed(bool on) {
    if (on) {
        speed = setmgr->get_setting("m_ffspeed")->val;
    }
    else {
        speed = 1;
    }

    audio_set_speed(speed);
}

int main(int argc, char *argv[]) {
    // Set default config options
    setmgr = new SettingManager();

    // Initialize SDL Audio and Joystick
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    int refreshrate = fltkui_refreshrate();

    jgm = new JGManager();

    // Read frontend and emulator settings
    setmgr->read(*jgm);

    audio_init(jgm);
    audio_unpause();

    // Initialize video params
    video_init(setmgr, jgm);

    // Set archive handler function pointer
    //nst_archive_select = &fltkui_archive_select;

    inputmgr = new InputManager(*jgm, *setmgr);
    chtmgr = new CheatManager(*jgm);

    makenstwin(argv[0]);
    nstwin->label("Nestopia UE");
    nstwin->show();
    menubar->show();
    glarea->make_current();
    glarea->show();
    Fl::check();

    nst_ogl_init();

    // Load a rom from the command line
    if (argc > 1 && argv[argc - 1][0] != '-') {
        fltkui_load_file(argv[argc - 1]);
        //jgm->load_game(argv[argc - 1]);
        if (jgm->is_loaded()) {
            nstwin->label(jgm->get_gamename().c_str());
            fltkui_enable_menu();
            jg_setup_audio();
            jg_setup_video();
            inputmgr->reassign();
        }
    }
    else if (video_fullscreen) {
        video_fullscreen = 0;
    }

    if (video_fullscreen) {
        video_fullscreen = 0;
        fltkui_fullscreen(NULL, NULL);
    }

    int frames = 0;
    int framefrags = 0;

    while (true) {
        Fl::check();
        if (!nstwin->shown()) {
            break;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            inputmgr->event(event);
        }

        int fps = jgm->get_frametime();
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

    // Deinitialize audio
    audio_deinit();

    // Write frontend and emulator settings
    setmgr->write(*jgm);

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
