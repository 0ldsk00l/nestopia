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

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include "jgmanager.h"

#include "logdriver.h"

namespace {

int frametime = 0;

void jg_frametime(double interval) {
    frametime = interval + 0.5;
}

} // namespace

JGManager::JGManager() {
    set_paths();

    size_t numsettings;
    jg_setting_t *jg_settings = jg_get_settings(&numsettings);
    for (size_t i = 0; i < numsettings; ++i) {
        settings.push_back(&jg_settings[i]);
    }

    jg_set_cb_frametime(jg_frametime);
    jg_set_cb_log(&LogDriver::jg_log);
    jg_init();
}

JGManager::~JGManager() {
    jg_deinit();
    unload_game();
}

void JGManager::unload_game() {
    if (loaded) {
        jg_game_unload();
        loaded = false;
    }
}

void JGManager::load_game(const char *filename, std::vector<uint8_t>& game) {
    // Make sure no game is currently loaded
    unload_game();

    // Set game data and size
    gameinfo.data = game.data();
    gameinfo.size = game.size();

    // Set path and name information
    gamepath = std::string(filename);
    gameinfo.path = gamepath.c_str();

    gamename = std::filesystem::path(filename).stem().string();
    gameinfo.name = gamename.c_str();

    gamefname = std::filesystem::path(filename).filename().string();
    gameinfo.fname = gamefname.c_str();

    jg_set_gameinfo(gameinfo);

    if (!jg_game_load()) {
        unload_game();
        return;
    }

    loaded = true;
}

void JGManager::set_paths() {
    if (const char *env_xdg_data = std::getenv("XDG_DATA_HOME")) {
        basepath = std::string(env_xdg_data) + "/nestopia";
    }
    else {
        basepath = std::string(std::getenv("HOME")) + "/.local/share/nestopia";
    }

    // Base path is used for BIOS and user assets
    pathinfo.base = basepath.c_str();
    pathinfo.bios = basepath.c_str();
    pathinfo.user = basepath.c_str();

    // Save path is a subdirectory in the base path
    savepath = basepath + "/save";
    pathinfo.save = savepath.c_str();

    // Create the save path, which includes creating the base path
    std::filesystem::create_directories(savepath);

    // Create paths for states and screenshots (Not part of the JG API)
    std::filesystem::create_directories(basepath + "/state");
    std::filesystem::create_directories(basepath + "/screenshots");

    // If the binary is run from the source directory, core asset path is PWD
    if (std::filesystem::exists(std::filesystem::path{"NstDatabase.xml"})) {
        corepath = std::string(std::getenv("PWD"));
    }
    else {
        corepath = std::string(NST_DATADIR);
    }

    pathinfo.core = corepath.c_str();

    jg_set_paths(pathinfo);
}

bool JGManager::is_loaded() {
    return loaded;
}

std::string &JGManager::get_basepath() {
    return basepath;
}

std::string &JGManager::get_gamename() {
    return gamename;
}

std::vector<jg_setting_t*> *JGManager::get_settings() {
    return &settings;
}

jg_setting_t *JGManager::get_setting(std::string name) {
    for (size_t i = 0; i < settings.size(); ++i ) {
        if (std::string(settings[i]->name) == name) {
            return settings[i];
        }
    }
    return nullptr;
}

void JGManager::exec_frame() {
    jg_exec_frame();
}

void JGManager::reset(int hard) {
    if (loaded) {
        jg_reset(hard);
    }
}

int JGManager::state_load(std::string &filename) {
    if (!loaded) {
        return 2;
    }

    return jg_state_load(filename.c_str());
}

int JGManager::state_qload(int slot) {
    if (!loaded) {
        return 2;
    }

    std::string slotpath = basepath + "/state/" + gamename + "_" +
                           std::to_string(slot) + ".nst";
    int result = state_load(slotpath);

    switch (result) {
        case 0: LogDriver::jg_log(JG_LOG_SCR, "State Load Failed"); break;
        case 1: LogDriver::jg_log(JG_LOG_SCR, "State Loaded"); break;
        default: LogDriver::jg_log(JG_LOG_SCR, "State Load Unknown"); break;
    }

    return result;
}

int JGManager::state_save(std::string &filename) {
    if (!loaded) {
        return 2;
    }

    return jg_state_save(filename.c_str());
}

int JGManager::state_qsave(int slot) {
    if (!loaded) {
        return 2;
    }

    std::string slotpath = basepath + "/state/" + gamename + "_" +
                           std::to_string(slot) + ".nst";
    int result = state_save(slotpath);

    switch (result) {
        case 0: LogDriver::jg_log(JG_LOG_SCR, "State Save Failed"); break;
        case 1: LogDriver::jg_log(JG_LOG_SCR, "State Saved"); break;
        default: LogDriver::jg_log(JG_LOG_SCR, "State Save Unknown"); break;
    }

    return result;
}

void JGManager::media_select() {
    if (!loaded) {
        return;
    }
    jg_media_select();
}

void JGManager::media_insert() {
    if (!loaded) {
        return;
    }
    jg_media_insert();
}

void JGManager::cheat_clear() {
    jg_cheat_clear();
}

void JGManager::cheat_set(const char *code) {
    jg_cheat_set(code);
}

int JGManager::get_frametime() {
    return frametime;
}

void JGManager::rehash() {
    if (loaded) {
        jg_rehash();
    }
}

jg_coreinfo_t *JGManager::get_coreinfo() {
    return jg_get_coreinfo("nes");
}

jg_inputinfo_t *JGManager::get_inputinfo(int port) {
    return jg_get_inputinfo(port);
}

jg_audioinfo_t *JGManager::get_audioinfo() {
    return jg_get_audioinfo();
}

void JGManager::set_audio_cb(jg_cb_audio_t cb) {
    jg_set_cb_audio(cb);
}

void JGManager::data_push(uint32_t type, int port, const void *p, size_t sz) {
    jg_data_push(type, port, p, sz);
}

void JGManager::setup_audio() {
    jg_setup_audio();
}

void JGManager::setup_video() {
    jg_setup_video();
}
