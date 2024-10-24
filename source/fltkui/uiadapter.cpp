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

#include "fltkui.h"
#include "uiadapter.h"

void UiAdapter::fullscreen() {
    FltkUi::fullscreen();
}

void UiAdapter::fastforward(bool on) {
    FltkUi::set_ffspeed(on);
}

void UiAdapter::pause() {
    FltkUi::pause();
}

void UiAdapter::screenshot() {
    FltkUi::screenshot();
}

void UiAdapter::quit() {
    FltkUi::quit();
}

void UiAdapter::show_inputmsg(int show) {
    FltkUi::show_inputmsg(show);
}
