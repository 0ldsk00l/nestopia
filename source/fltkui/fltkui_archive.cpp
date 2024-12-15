/*
Copyright (c) 2012-2024 R. Danbrook
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <set>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <archive.h>
#include <archive_entry.h>

#include "fltkui.h"
#include "fltkui_archive.h"

static Fl_Double_Window *window{nullptr};
static Fl_Select_Browser *browser{nullptr};
static char *romfile{nullptr};

static void fltkui_archive_ok(Fl_Widget *w, long) {
    window->hide();
}

static void fltkui_archive_cancel(Fl_Widget *w, long) {
    snprintf(romfile, 256, "%s", "");
    window->hide();
}

static void fltkui_archive_setfile(Fl_Widget *w, long) {
    int r = ((Fl_Browser*)w)->value();
    if (r) {
        snprintf(romfile, 256, "%s", ((Fl_Browser*)w)->text(r));
        if (Fl::event_clicks()) {
            window->hide();
        }
    }
    else {
        snprintf(romfile, 256, "%s", "");
    }
}

void fltkui_archive_load_file(const char *filename, std::string& arcname, std::vector<uint8_t>& game) {
    struct archive *a;
    struct archive_entry *entry;
    int r = 0;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    archive_read_support_format_raw(a);
    r = archive_read_open_filename(a, filename, 10240);

    // Test if it's actually an archive
    if (r != ARCHIVE_OK) {
        r = archive_read_free(a);
        return;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *currentfile = archive_entry_pathname(entry);
        if (std::string(currentfile) == arcname) { // Found the file
            la_int64_t entrysize = archive_entry_size(entry);
            char *rombuf = (char*)calloc(1, entrysize);
            archive_read_data(a, rombuf, entrysize);
            archive_read_data_skip(a);
            std::copy(rombuf, rombuf + entrysize, std::back_inserter(game));
            free(rombuf);
            break;
        }
    }

    // Free the archive
    r = archive_read_free(a);
}

bool fltkui_archive_select(const char *filename, std::string& arcname) {
    // Select a filename to pull out of the archive
    char reqfile[256];
    size_t reqsize = 256;

    struct archive *a;
    struct archive_entry *entry;
    int r, numfiles = 0;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    archive_read_support_format_raw(a);
    r = archive_read_open_filename(a, filename, 16384);

    // Test if it's actually an archive
    if (r != ARCHIVE_OK) {
        r = archive_read_free(a);
        return false;
    }

    // If it is an archive, handle it
    if (window) {
        delete window;
        window = nullptr;
    }

    window = new Fl_Double_Window(420, 260, "Load from Archive");
    browser = new Fl_Select_Browser(0, 0, window->w(), 200, 0);
    browser->type(FL_HOLD_BROWSER);
    browser->callback(fltkui_archive_setfile, 0);
    browser->selection_color(NstGreen);
    Fl_Button btncancel(260, 220, 80, 24, "&Cancel");
    btncancel.callback(fltkui_archive_cancel, 0);
    Fl_Button btnok(350, 220, 40, 24, "&OK");
    btnok.callback(fltkui_archive_ok, 0);

    romfile = reqfile;

    std::set<std::string> nes_exts = { ".nes", ".fds", ".unf", ".unif", ".bin", ".xml" };

    // Fill the treestore with the filenames
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *currentfile = archive_entry_pathname(entry);
        std::string fileext = std::filesystem::path(currentfile).extension().string();
        std::transform(fileext.begin(), fileext.end(), fileext.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (nes_exts.find(fileext) != nes_exts.end()) {
            browser->add(currentfile);
            numfiles++;
            snprintf(reqfile, reqsize, "%s", currentfile);
        }

        archive_read_data_skip(a);
    }

    // Free the archive
    r = archive_read_free(a);

    // If there are no valid files in the archive, return
    if (numfiles == 0) {
        return false;
    }

    // If there's only one file, don't bring up the selector
    if (numfiles == 1) {
        arcname = std::string(romfile);
        return true;
    }

    // Show selector
    browser->select(1);
    snprintf(romfile, 256, "%s", browser->text(1));

    window->resizable(browser);
    window->show();
    window->set_modal();
    window->show();
    while (window->shown()) {
        Fl::wait();
    }

    if (strlen(romfile)) {
        arcname = std::string(romfile);
        return true;
    }

    return false;
}
