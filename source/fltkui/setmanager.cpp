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

#include <jg/jg.h>
#include <jg/jg_nes.h>

#include "ini.h"

#include "setmanager.h"

namespace {

jg_setting_t fe_settings[] = {
    { "v_renderer", "Video Renderer (Restart)",
      "0 = Modern, 1 = Legacy",
      "Use Modern (Core Profile, GLES) or Legacy (Compatibility Profile) OpenGL. "
      "Use Modern unless you are on extremely weak or incapable hardware.",
      0, 0, 1, FLAG_FRONTEND | JG_SETTING_RESTART
    },
    { "v_postproc", "Post-processing",
      "0 = Nearest Neighbour, 1 = Linear, 2 = Sharp Bilinear, 3 = CRT, 4 = MMPX, 5 = OmniScale",
      "Select a video post-processing effect. Advanced effects are only available to the Modern renderer.",
      2, 0, 5, FLAG_FRONTEND
    },
    { "v_aspect", "Aspect Ratio",
      "0 = Auto, 1 = 1:1, 2 = 4:3, 3 = 5:4",
      "Set the aspect ratio to the correct TV aspect (Auto), 1:1 (square pixels), 4:3, or 5:4",
      0, 0, 3, FLAG_FRONTEND
    },
    { "v_fullscreen", "Start in Fullscreen Mode",
      "0 = Disabled, 1 = Enabled",
      "Start the emulator in fullscreen mode if a valid ROM is entered on the command line",
      0, 0, 1, FLAG_FRONTEND | JG_SETTING_RESTART
    },
    { "v_scale", "Initial Window Scale",
      "N = Window scale factor at startup",
      "Set the window's initial scale factor (multiple of NES resolution)",
      3, 1, 16, FLAG_FRONTEND | JG_SETTING_RESTART
    },
    { "a_rsqual", "Audio Resampler",
      "0 = Sinc (Best), 1 = Sinc (Medium), 2 = Sinc (Fast), 3 = Zero Order Hold, 4 = Linear",
      "Set the frontend's audio resampling quality. Use Sinc unless you are on extremely weak hardware.",
      2, 0, 4, FLAG_FRONTEND
    },
    { "m_ffspeed", "Fast-forward Speed",
      "N = Fast-forward speed multiplier",
      "Set the speed multiplier to run emulation faster",
      2, 2, 8, FLAG_FRONTEND
    },
    { "m_hidecursor", "Hide Cursor",
      "0 = Disabled, 1 = Enabled",
      "Hide the cursor when hovering over the UI",
      0, 0, 1, FLAG_FRONTEND
    },
    { "m_hidecrosshair", "Hide Crosshair",
      "0 = Disabled, 1 = Enabled",
      "Hide the crosshair when a Zapper is present",
      0, 0, 1, FLAG_FRONTEND
    },
    { "m_syncmode", "Synchronization Mode (Restart)",
      "0 = VSync, 1 = Timer",
      "Set the Synchronization Mode: VSync to sync to VBLANK, Timer in cases where VSync is unreliable",
      0, 0, 1, FLAG_FRONTEND | JG_SETTING_RESTART
    },
    { "s_crtmasktype", "CRT Mask Type",
      "0 = No Mask, 1 = Aperture Grille Lite, 2 = Aperture Grille, "
      "3 = Shadow Mask",
      "Set the type of CRT mask. Set no mask for pure scanlines, or use Aperture "
      "Grille or Shadow Mask options to mimic the appearance of a real CRT screen.",
      0, 0, 3, FLAG_FRONTEND
    },
    { "s_crtmaskstr", "CRT Mask Strength",
      "N = CRT Mask Strength",
      "Set the strength of the CRT mask",
      5, 0, 10, FLAG_FRONTEND
    },
    { "s_crtscanstr", "CRT Scanline Strength",
      "N = CRT Scanline Strength",
      "Set the strength of the scanlines",
      6, 0, 10, FLAG_FRONTEND
    },
    { "s_crtsharp", "CRT Sharpness",
      "N = CRT Sharpness",
      "Set the level of blur/sharpness",
      3, 0, 10, FLAG_FRONTEND
    },
    { "s_crtcurve", "CRT Curve",
      "N = CRT Curvature",
      "Set the level of screen curvature on the horizontal and vertical axes",
      3, 0, 10, FLAG_FRONTEND
    },
    { "s_crtcorner", "CRT Corner",
      "N = CRT Corner",
      "Set the size of the corner mask",
      3, 0, 10, FLAG_FRONTEND
    },
    { "s_crttcurve", "CRT Trinitron Curve",
      "N = CRT Trinitron Curvature",
      "Set the level of Trinitron Curvature, which reduces the curve on the vertical axis",
      10, 0, 10, FLAG_FRONTEND
    },
    { "l_loglevel", "Console Log Level",
      "0 = Debug, 1 = Info, 2 = Warn, 3 = Error",
      "Set the level of logs printed to the console. Debug shows all, Error shows only critical errors.",
      1, 0, 3, FLAG_FRONTEND
    },
};

jg_setting_t nullsetting;

// Input config is really better staying alive in memory in .ini form
mINI::INIStructure inputini;

}

SettingManager::SettingManager() {
    // Set defaults
    size_t numsettings = sizeof(fe_settings) / sizeof(jg_setting_t);
    for (size_t i = 0; i < numsettings; ++i ) {
        settings.push_back(&fe_settings[i]);
    }

    // Create config directory
    if (const char *env_xdg_config = std::getenv("XDG_CONFIG_HOME")) {
        confpath = std::string(env_xdg_config) + "/nestopia";
    }
    else {
        confpath = std::string(std::getenv("HOME")) + "/.config/nestopia";
    }

    // Create the directory if it does not exist
    std::filesystem::create_directories(confpath);
}

void SettingManager::read(JGManager& jgm) {
    // Read in any settings
    mINI::INIFile file(confpath + "/nestopia.conf");
    mINI::INIStructure ini;
    file.read(ini);

    for (const auto& setting : settings) {
        std::string& strval = ini["frontend"][setting->name];

        if (strval.empty()) {
            continue;
        }

        int val = std::stoi(strval);
        if (val >= setting->min && val <= setting->max) {
            setting->val = val;
        }
    }

    for (const auto& setting : *jgm.get_settings()) {
        std::string& strval = ini["nestopia"][setting->name];

        if (strval.empty()) {
            continue;
        }

        int val = std::stoi(strval);
        if (val >= setting->min && val <= setting->max) {
            setting->val = val;
        }
    }

    jgm.rehash();

    // Read input config
    mINI::INIFile inputfile(confpath + "/input.conf");
    inputfile.read(inputini);
}

void SettingManager::write(JGManager& jgm) {
    std::string filepath(confpath + "/nestopia.conf");
    std::ofstream os(filepath);

    if (!os.is_open()) {
        return;
    }

    os << "; Nestopia UE Configuration File\n\n";

    // Write out frontend settings
    os << "[frontend]\n";
    for (const auto& setting : settings) {
        os << "; " << setting->desc << "\n";
        os << "; " << setting->opts << "\n";
        os << setting->name << " = " << setting->val << "\n\n";
    }

    // Write out emulator core settings
    os << "[nestopia]\n";
    for (const auto& setting : *jgm.get_settings()) {
        os << "; " << setting->desc << "\n";
        os << "; " << setting->opts << "\n";
        os << setting->name << " = " << setting->val << "\n\n";
    }

    os.close();

    mINI::INIFile inputfile(confpath + "/input.conf");
    inputfile.write(inputini, true);

    /*for (auto const& it : inputini) {
        auto const& section = it.first;
        auto const& collection = it.second;

        std::cout << "[" << section << "]" << std::endl;

        for (auto const& it2 : collection) {
            auto const& key = it2.first;
            auto const& value = it2.second;
            std::cout << key << " = " << value << std::endl;
        }
    }*/
}

std::vector<jg_setting_t*> *SettingManager::get_settings() {
    return &settings;
}

jg_setting_t* SettingManager::get_setting(std::string name) {
    size_t numsettings = sizeof(fe_settings) / sizeof(jg_setting_t);
    for (size_t i = 0; i < numsettings; ++i ) {
        if (std::string(fe_settings[i].name) == name) {
            return &fe_settings[i];
        }
    }
    return &nullsetting;
}

std::string& SettingManager::get_input(std::string name, std::string def) {
    return inputini[name][def];
}

void SettingManager::set_input(std::string name, std::string def, std::string val) {
    inputini[name][def] = val;
}
