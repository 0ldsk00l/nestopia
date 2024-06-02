/*
 * Nestopia JG
 *
 * Copyright (C) 2008-2018 Nestopia UE Contributors
 * Copyright (C) 2020-2024 Rupert Carmichael
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stdint.h>

#include <jg/jg.h>
#include <jg/jg_nes.h>

#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiCheats.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiCartridge.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiFds.hpp"
#include "version.h"

#define SAMPLERATE 48000
#define FRAMERATE 60.098814
#define FRAMERATE_PAL 50.006978
#define CHANNELS 1
#define NUMINPUTS 5

#define ASPECT_NTSC 8.0 / 7.0
#define ASPECT_PAL 7375000.0 / 5320342.5
#define NTSC_FILTER_RATIO 2.3515625 // 602 / 256

using namespace Nes::Api;

static Emulator emulator;
static Video::Output *NstVideo;
static Sound::Output *NstSound;
static Input::Controllers *NstInput;

static jg_cb_audio_t jg_cb_audio;
static jg_cb_frametime_t jg_cb_frametime;
static jg_cb_log_t jg_cb_log;
static jg_cb_rumble_t jg_cb_rumble;

static jg_coreinfo_t coreinfo = {
    "nestopia", "Nestopia JG", JG_VERSION, "nes", NUMINPUTS, 0
};

static jg_fileinfo_t biosinfo;
static jg_fileinfo_t gameinfo;
static jg_pathinfo_t pathinfo;

static jg_videoinfo_t vidinfo = {
    JG_PIXFMT_XRGB8888,         // pixfmt
    Video::Output::NTSC_WIDTH,  // wmax
    Video::Output::HEIGHT,      // hmax
    Video::Output::WIDTH,       // w
    Video::Output::HEIGHT,      // h
    0,                          // x
    0,                          // y
    Video::Output::NTSC_WIDTH,  // p
    (Video::Output::WIDTH * ASPECT_NTSC) / Video::Output::HEIGHT, // aspect
    NULL
};

static jg_audioinfo_t audinfo = {
    JG_SAMPFMT_INT16,
    SAMPLERATE,
    CHANNELS,
    (SAMPLERATE / (unsigned)FRAMERATE) * CHANNELS,
    NULL
};

// Input Devices
static jg_inputinfo_t inputinfo[NUMINPUTS];
static jg_inputstate_t *input_device[NUMINPUTS];

// Emulator-specific settings
static jg_setting_t settings_nst[] = {
    { "port1", "Controller Port 1",
      "0 = Auto, 1 = Controller, 2 = Zapper, 3 = Arkanoid Paddle, "
      "4 = Power Pad, 5 = Power Glove",
      "Select the device plugged into controller port 1",
      0, 0, 5, JG_SETTING_INPUT
    },
    { "port2", "Controller Port 2",
      "0 = Auto, 1 = Controller, 2 = Zapper, 3 = Arkanoid Paddle, "
      "4 = Power Pad, 5 = Power Glove",
      "Select the device plugged into controller port 2",
      0, 0, 5, JG_SETTING_INPUT
    },
    { "port3", "Controller Port 3",
      "0 = Auto, 1 = Controller",
      "Select the device plugged into controller port 3",
      0, 0, 1, JG_SETTING_INPUT
    },
    { "port4", "Controller Port 4",
      "0 = Auto, 1 = Controller",
      "Select the device plugged into controller port 4",
      0, 0, 1, JG_SETTING_INPUT
    },
    { "portexp", "Expansion Port",
      "0 = Auto, 1 = Family Trainer, 2 = Pachinko, 3 = Oeka Kids Tablet, "
      "4 = Konami Hypershot, 5 = Bandai Hypershot, 6 = Crazy Climber, "
      "7 = Mahjong, 8 = Exciting Boxing, 9 = Top Rider, 10 = Pokkun Moguraa, "
      "11 = PartyTap",
      "Select the device plugged into the expansion port",
      0, 0, 11, JG_SETTING_INPUT
    },
    { "turbo_rate", "Turbo Pulse Rate",
      "N = Pulse every N frames",
      "Set the speed (in frames) at which the turbo buttons are pulsed",
      3, 2, 9, JG_SETTING_INPUT
    },
    { "palette", "Palette",
      "0 = Canonical, 1 = Consumer, 2 = Alternative, 3 = RGB, 4 = CXA2025AS, "
      "5 = Royaltea, 6 = Nobilitea, 7 = Digital Prime (FBX), "
      "8 = Magnum (FBX), 9 = PVM Style D93 (FBX), 10 = Smooth V2 (FBX), "
      "11 = Custom",
      "Set the colour palette",
      Video::DECODER_CONSUMER, 0, 11, 0
    },
    { "ntsc_filter", "NTSC Filter",
      "0 = Disable, 1 = Enable",
      "Use blargg's NTSC filter (required for Chroma Crosstalk emulation)",
      0, 0, 1, 0
    },
    { "ntsc_mode", "NTSC Filter Mode",
      "0 = Composite, 1 = S-Video, 2 = RGB, 3 = Monochrome",
      "Set the NTSC Filter Mode",
      0, 0, 3, 0
    },
    { "overscan_t", "Overscan Mask (Top)",
      "N = Hide N pixels of Overscan (Top)",
      "Hide N pixels of Overscan (Top)",
      8, 0, 12, 0
    },
    { "overscan_b", "Overscan Mask (Bottom)",
      "N = Hide N pixels of Overscan (Bottom)",
      "Hide N pixels of Overscan (Bottom)",
      8, 0, 12, 0
    },
    { "overscan_l", "Overscan Mask (Left)",
      "N = Hide N pixels of Overscan (Left)",
      "Hide N pixels of Overscan (Left)",
      0, 0, 12, 0
    },
    { "overscan_r", "Overscan Mask (Right)",
      "N = Hide N pixels of Overscan (Right)",
      "Hide N pixels of Overscan (Right)",
      0, 0, 12, 0
    },
    { "favored_system", "Favored/System",
      "0 = NTSC, 1 = PAL, 2 = Famicom, 3 = Dendy",
      "Set the desired system in cases where a database entry does not exist",
      0, 0, 3, JG_SETTING_RESTART
    },
    { "force_region", "Force Region",
      "0 = Auto, 1 = Force",
      "Force the system to be the Favored System",
      0, 0, 1, JG_SETTING_RESTART
    },
    { "ram_power_state", "RAM Power-on State",
      "0 = 0x00, 1 = 0xff, 2 = Random",
      "Set the initial values in System RAM when the system is powered on",
      2, 0, 2, JG_SETTING_RESTART
    },
    { "unlimited_sprites", "Unlimited Sprites",
      "0 = Disable, 1 = Enable",
      "Disables the 8 sprite per line limit - may cause glitches",
      0, 0, 1, 0
    },
    { "genie_distortion", "Game Genie Sound Distortion",
      "0 = Disable, 1 = Enable",
      "Some games do not clear APU registers at boot, causing distortion when "
      "a Game Genie is in use (Mega Man, Mega Man 2)",
      0, 0, 1, JG_SETTING_RESTART
    },
    { "softpatch", "Soft Patching",
      "0 = Disable, 1 = Enable",
      "Automatically patch games when a .ips or .ups patch file with the same "
      "name as the ROM is present",
      0, 0, 1, JG_SETTING_RESTART
    }
};

enum {
    PORT1,
    PORT2,
    PORT3,
    PORT4,
    PORTEXP,
    TURBORATE,
    PALETTE,
    NTSC,
    NTSCMODE,
    OVERSCAN_T,
    OVERSCAN_B,
    OVERSCAN_L,
    OVERSCAN_R,
    FAVSYSTEM,
    FORCEREGION,
    RAMPOWERSTATE,
    UNLIMITEDSPRITES,
    GENIEDISTORTION,
    SOFTPATCH
};

// Microphone Input
static unsigned micstate = 0;
static bool kstudio = false;

// Nestopia Helper Functions
static void nst_fds_info(void) {
    Fds fds(emulator);
    if (fds.IsAnyDiskInserted()) {
        jg_cb_log(JG_LOG_INF, "Disk %c Side %c\n",
            fds.GetCurrentDisk() == 0 ? '1' : '2',
            fds.GetCurrentDiskSide() == 0 ? 'A' : 'B');
        jg_cb_log(JG_LOG_SCR, "Disk %c Side %c.",
            fds.GetCurrentDisk() == 0 ? '1' : '2',
            fds.GetCurrentDiskSide() == 0 ? 'A' : 'B');
    }
    else {
        jg_cb_log(JG_LOG_INF, "Disk Drive Empty\n");
        jg_cb_log(JG_LOG_SCR, "Disk Drive Empty.");
    }
}

static int nst_db_load(void) {
    Cartridge::Database db(emulator);
    std::string dbpath = std::string(pathinfo.core) + "/NstDatabase.xml";
    std::ifstream dbfile(dbpath.c_str(),
        std::ifstream::in|std::ifstream::binary);

    if (dbfile.is_open()) {
        db.Load(dbfile);
        db.Enable(true);
        dbfile.close();
        return 1;
    }
    return 0;
}

static void nst_sample_load(User::File& file, const char *sampgame) {
    char wavfile[] = "xx.wav";
    unsigned id = file.GetId();
    if (id > 99) { return; }
    wavfile[0] = '0' + id / 10;
    wavfile[1] = '0' + id % 10;
    std::string filepath = std::string(pathinfo.user) + "/" +
        std::string(sampgame) + "/" + std::string(wavfile);
    std::ifstream is(filepath.c_str(), std::ifstream::binary);

    if (is) {
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);

        uint8_t buffer[length];
        is.read((char*)buffer, length);
        is.close();

        uint32_t datasize = buffer[0x2b] << 24 | buffer[0x2a] << 16 |
            buffer[0x29] << 8 | buffer[0x28];
        uint16_t blockalign = buffer[0x21] << 8 | buffer[0x20];
        uint16_t numchannels = buffer[0x17] << 8 | buffer[0x16];
        uint16_t bitspersample = buffer[0x23] << 8 | buffer[0x22];
        uint32_t samplerate = buffer[0x1b] << 24 | buffer[0x1a] << 16 |
            buffer[0x19] << 8 | buffer[0x18];
        file.SetSampleContent(&buffer[0x2c], datasize / blockalign,
            numchannels == 2, bitspersample, samplerate);
    }
}

// Nestopia Callbacks
static void NST_CALLBACK nst_cb_event(void *userdata, User::Event event,
    const void *data) {
    // Handle special events
    switch (event) {
        case User::EVENT_CPU_JAM:
            jg_cb_log(JG_LOG_SCR, "Cpu: Jammed.");
            break;
        case User::EVENT_CPU_UNOFFICIAL_OPCODE:
            jg_cb_log(JG_LOG_DBG, "Cpu: Unofficial Opcode %s\n",
                (const char*)data);
            break;
        case User::EVENT_DISPLAY_TIMER:
            jg_cb_log(JG_LOG_SCR, (const char*)data);
            break;
        default: break;
    }
}

static void NST_CALLBACK nst_cb_file(void *userdata, User::File& file) {
    switch (file.GetAction()) {
        case User::File::LOAD_ROM: {
            break;
        }
        case User::File::LOAD_SAMPLE: {
            break;
        }
        case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU: {
            nst_sample_load(file, "moepro");
            break;
        }
        case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88: {
            nst_sample_load(file, "moepro88");
            break;
        }
        case User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS: {
            nst_sample_load(file, "mptennis");
            break;
        }
        case User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU: {
            nst_sample_load(file, "terao");
            break;
        }
        case User::File::LOAD_SAMPLE_AEROBICS_STUDIO: {
            nst_sample_load(file, "ftaerobi");
            break;
        }
        case User::File::LOAD_BATTERY:
        case User::File::LOAD_EEPROM:
        case User::File::LOAD_TAPE:
        case User::File::LOAD_TURBOFILE: {
            std::string savename = std::string(pathinfo.save) + "/" +
                std::string(gameinfo.name) + ".sav";
            std::ifstream savefile(savename.c_str(),
                std::ifstream::in|std::ifstream::binary);
            if (savefile.is_open()) {
                file.SetContent(savefile);
                savefile.close();
            }
            break;
        }
        case User::File::SAVE_BATTERY:
        case User::File::SAVE_EEPROM:
        case User::File::SAVE_TAPE:
        case User::File::SAVE_TURBOFILE: {
            std::string savename = std::string(pathinfo.save) + "/" +
                std::string(gameinfo.name) + ".sav";
            std::ofstream savefile(savename.c_str(),
                std::ifstream::out|std::ifstream::binary);
            const void *savedata;
            unsigned long datasize;
            file.GetContent(savedata, datasize);
            if (savefile.is_open()) {
                savefile.write((const char*)savedata, datasize);
                savefile.close();
            }
            break;
        }
        case User::File::LOAD_FDS: {
            std::string savename = std::string(pathinfo.save) + "/" +
                std::string(gameinfo.name) + ".ups";
            std::ifstream savefile(savename.c_str(),
                std::ifstream::in|std::ifstream::binary);
            if (savefile.is_open()) {
                file.SetPatchContent(savefile);
                savefile.close();
            }
            break;
        }
        case User::File::SAVE_FDS: {
            std::string savename = std::string(pathinfo.save) + "/" +
                std::string(gameinfo.name) + ".ups";
            std::ofstream savefile(savename.c_str(),
                std::ifstream::out|std::ifstream::binary);
            if (savefile.is_open()) {
                file.GetPatchContent(User::File::PATCH_UPS, savefile);
                savefile.close();
            }
            break;
        }
    }
}

static void NST_CALLBACK nst_cb_log(void *udata, const char *str,
    unsigned long int length) {

    jg_cb_log(JG_LOG_DBG, str);
}

static bool NST_CALLBACK nst_cb_videolock(void *udata, Video::Output& video) {
    video.pixels = vidinfo.buf;
    video.pitch = Video::Output::NTSC_WIDTH * sizeof(uint32_t);
    return true;
}

static void NST_CALLBACK nst_cb_videounlock(void *udata, Video::Output& video) {
}

static bool NST_CALLBACK nst_cb_soundlock(void* udata, Sound::Output& sound) {
    jg_cb_audio(audinfo.spf);
    return true;
}

static void NST_CALLBACK nst_cb_soundunlock(void* udata, Sound::Output& sound) {
}

// Input Poll Callbacks
static uint8_t NESMap[] = {
    Input::Controllers::Pad::UP, Input::Controllers::Pad::DOWN,
    Input::Controllers::Pad::LEFT, Input::Controllers::Pad::RIGHT,
    Input::Controllers::Pad::SELECT, Input::Controllers::Pad::START,
    Input::Controllers::Pad::A, Input::Controllers::Pad::B
};

static bool NST_CALLBACK nst_poll_pad(Input::UserData data,
    Input::Controllers::Pad& pad, unsigned port) {

    unsigned buttons = 0;

    for (int i = 0; i < inputinfo[port].numbuttons - 2; ++i)
        if (input_device[port]->button[i]) buttons |= NESMap[i];

    // Turbo
    if (input_device[port]->button[8]) {
        if (input_device[port]->button[8] == settings_nst[TURBORATE].val) {
            buttons |= Input::Controllers::Pad::A;
            input_device[port]->button[8] = 1;
        }
        else {
            ++input_device[port]->button[8];
        }
    }
    if (input_device[port]->button[9]) {
        if (input_device[port]->button[9] == settings_nst[TURBORATE].val) {
            buttons |= Input::Controllers::Pad::B;
            input_device[port]->button[9] = 1;
        }
        else {
            ++input_device[port]->button[9];
        }
    }

    if (port == 1)
        pad.mic = micstate;

    pad.buttons = buttons;
    return true;
}

static bool NST_CALLBACK nst_poll_zapper(Input::UserData data,
    Input::Controllers::Zapper& zapper) {

    jg_inputstate_t *zdev = static_cast<jg_inputstate_t*>(data);
    zapper.fire = false;

    if (zdev->button[0]) {
        zapper.fire = true;

        if (settings_nst[NTSC].val)
            zapper.x = zdev->coord[0] / NTSC_FILTER_RATIO;
        else
            zapper.x = zdev->coord[0];

        zapper.y = zdev->coord[1];
    }
    else if (zdev->button[1]) {
        zapper.fire = true;
        zapper.x = ~0U;
        zapper.y = ~0U;
    }

    return true;
}

static bool NST_CALLBACK nst_poll_paddle(Input::UserData data,
    Input::Controllers::Paddle& paddle) {

    jg_inputstate_t *adev = static_cast<jg_inputstate_t*>(data);

    paddle.x = (adev->axis[0] / 546) + 106; // 46-166 - super magical numbers!
    paddle.button = adev->button[0];

    return true;
}

static bool NST_CALLBACK nst_poll_powerpad(Input::UserData data,
    Input::Controllers::PowerPad& powerPad) {

    jg_inputstate_t *ppdev = static_cast<jg_inputstate_t*>(data);

    for (int i = 0; i < 12; ++i)
        powerPad.sideA[i] = ppdev->button[i];

    for (int i = 0; i < 8; ++i)
        powerPad.sideB[i] = ppdev->button[i + 12];

    return true;
}

static bool NST_CALLBACK nst_poll_powerglove(Input::UserData data,
    Input::Controllers::PowerGlove& glove) {

    jg_inputstate_t *pgdev = static_cast<jg_inputstate_t*>(data);
    unsigned pad = 0;

    if (pgdev->button[0]) pad |= Input::Controllers::PowerGlove::SELECT;
    if (pgdev->button[1]) pad |= Input::Controllers::PowerGlove::START;
    glove.buttons = pad;

    if (pgdev->button[2])
        glove.distance = Input::Controllers::PowerGlove::DISTANCE_IN;
    else if (pgdev->button[3])
        glove.distance = Input::Controllers::PowerGlove::DISTANCE_OUT;
    else
        glove.distance = 0;

    if (pgdev->button[4])
        glove.wrist = Input::Controllers::PowerGlove::ROLL_LEFT;
    else if (pgdev->button[5])
        glove.wrist = Input::Controllers::PowerGlove::ROLL_RIGHT;
    else
        glove.wrist = 0;

    if (pgdev->button[6])
        glove.gesture = Input::Controllers::PowerGlove::GESTURE_FIST;
    else if (pgdev->button[7])
        glove.gesture = Input::Controllers::PowerGlove::GESTURE_FINGER;
    else
        glove.gesture = Input::Controllers::PowerGlove::GESTURE_OPEN;

    if (settings_nst[NTSC].val)
        glove.x = pgdev->coord[0] / NTSC_FILTER_RATIO;
    else
        glove.x = pgdev->coord[0];

    glove.y = pgdev->coord[1];

    return true;
}

static bool NST_CALLBACK nst_poll_familytrainer(Input::UserData data,
    Input::Controllers::FamilyTrainer& familyTrainer) {

    jg_inputstate_t *ftdev = static_cast<jg_inputstate_t*>(data);

    for (int i = 0; i < 12; ++i)
        familyTrainer.sideA[i] = ftdev->button[i];

    for (int i = 0; i < 8; ++i)
        familyTrainer.sideB[i] = ftdev->button[i + 12];

    return true;
}

static bool NST_CALLBACK nst_poll_pachinko(Input::UserData data,
    Input::Controllers::Pachinko& pachinko) {

    jg_inputstate_t *pchdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    for (int i = 0; i < 8; ++i)
        if (pchdev->button[i]) buttons |= NESMap[i];

    pachinko.buttons = buttons;

    pachinko.throttle = pchdev->axis[0] / 512;

    return true;
}

static bool NST_CALLBACK nst_poll_oekakids(Input::UserData data,
    Input::Controllers::OekaKidsTablet& tablet) {

    jg_inputstate_t *okdev = static_cast<jg_inputstate_t*>(data);

    if (settings_nst[NTSC].val)
        tablet.x = okdev->coord[0] / NTSC_FILTER_RATIO;
    else
        tablet.x = okdev->coord[0];

    tablet.y = okdev->coord[1];

    tablet.button = okdev->button[0];

    return true;
}

static bool NST_CALLBACK nst_poll_konamihypershot(Input::UserData data,
    Input::Controllers::KonamiHyperShot& konamiHyperShot) {

    jg_inputstate_t *khsdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    if (khsdev->button[0])
        buttons |= Input::Controllers::KonamiHyperShot::PLAYER1_BUTTON_1;

    if (khsdev->button[1])
        buttons |= Input::Controllers::KonamiHyperShot::PLAYER1_BUTTON_2;

    if (khsdev->button[2])
        buttons |= Input::Controllers::KonamiHyperShot::PLAYER2_BUTTON_1;

    if (khsdev->button[3])
        buttons |= Input::Controllers::KonamiHyperShot::PLAYER2_BUTTON_2;

    konamiHyperShot.buttons = buttons;
    return true;
}

static bool NST_CALLBACK nst_poll_bandaihypershot(Input::UserData data,
    Input::Controllers::BandaiHyperShot& bandaiHyperShot) {

    jg_inputstate_t *bhsdev = static_cast<jg_inputstate_t*>(data);
    bandaiHyperShot.fire = false;

    if (bhsdev->button[0]) {
        bandaiHyperShot.fire = true;

        if (settings_nst[NTSC].val)
            bandaiHyperShot.x = bhsdev->coord[0] / NTSC_FILTER_RATIO;
        else
            bandaiHyperShot.x = bhsdev->coord[0];

        bandaiHyperShot.y = bhsdev->coord[1];
    }
    else if (bhsdev->button[1]) {
        bandaiHyperShot.fire = true;
        bandaiHyperShot.x = ~0U;
        bandaiHyperShot.y = ~0U;
    }
    bandaiHyperShot.move = bhsdev->button[2];

    return true;
}

static bool NST_CALLBACK nst_poll_crazyclimber(Input::UserData data,
    Input::Controllers::CrazyClimber& crazyClimber) {

    jg_inputstate_t *ccdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    if (ccdev->button[0]) buttons |= Input::Controllers::CrazyClimber::UP;
    if (ccdev->button[1]) buttons |= Input::Controllers::CrazyClimber::DOWN;
    if (ccdev->button[2]) buttons |= Input::Controllers::CrazyClimber::LEFT;
    if (ccdev->button[3]) buttons |= Input::Controllers::CrazyClimber::RIGHT;
    crazyClimber.left = buttons;

    buttons = 0;

    if (ccdev->button[4]) buttons |= Input::Controllers::CrazyClimber::UP;
    if (ccdev->button[5]) buttons |= Input::Controllers::CrazyClimber::DOWN;
    if (ccdev->button[6]) buttons |= Input::Controllers::CrazyClimber::LEFT;
    if (ccdev->button[7]) buttons |= Input::Controllers::CrazyClimber::RIGHT;
    crazyClimber.right = buttons;

    return true;
}

static bool NST_CALLBACK nst_poll_mahjong(Input::UserData data,
    Input::Controllers::Mahjong& mahjong, unsigned part) {

    jg_inputstate_t *mjdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    switch (part) {
        case Input::Controllers::Mahjong::PART_1:
            if (mjdev->button[8])
                buttons |= Input::Controllers::Mahjong::PART_1_I;
            if (mjdev->button[9])
                buttons |= Input::Controllers::Mahjong::PART_1_J;
            if (mjdev->button[10])
                buttons |= Input::Controllers::Mahjong::PART_1_K;
            if (mjdev->button[11])
                buttons |= Input::Controllers::Mahjong::PART_1_L;
            if (mjdev->button[12])
                buttons |= Input::Controllers::Mahjong::PART_1_M;
            if (mjdev->button[13])
                buttons |= Input::Controllers::Mahjong::PART_1_N;
            break;
        case Input::Controllers::Mahjong::PART_2:
            if (mjdev->button[0])
                buttons |= Input::Controllers::Mahjong::PART_2_A;
            if (mjdev->button[1])
                buttons |= Input::Controllers::Mahjong::PART_2_B;
            if (mjdev->button[2])
                buttons |= Input::Controllers::Mahjong::PART_2_C;
            if (mjdev->button[3])
                buttons |= Input::Controllers::Mahjong::PART_2_D;
            if (mjdev->button[4])
                buttons |= Input::Controllers::Mahjong::PART_2_E;
            if (mjdev->button[5])
                buttons |= Input::Controllers::Mahjong::PART_2_F;
            if (mjdev->button[6])
                buttons |= Input::Controllers::Mahjong::PART_2_G;
            if (mjdev->button[7])
                buttons |= Input::Controllers::Mahjong::PART_2_H;
            break;
        case Input::Controllers::Mahjong::PART_3:
            if (mjdev->button[14])
                buttons |= Input::Controllers::Mahjong::PART_3_SELECT;
            if (mjdev->button[15])
                buttons |= Input::Controllers::Mahjong::PART_3_START;
            if (mjdev->button[16])
                buttons |= Input::Controllers::Mahjong::PART_3_KAN;
            if (mjdev->button[17])
                buttons |= Input::Controllers::Mahjong::PART_3_PON;
            if (mjdev->button[18])
                buttons |= Input::Controllers::Mahjong::PART_3_CHI;
            if (mjdev->button[19])
                buttons |= Input::Controllers::Mahjong::PART_3_REACH;
            if (mjdev->button[20])
                buttons |= Input::Controllers::Mahjong::PART_3_RON;
            break;
    }

    mahjong.buttons = buttons;
    return true;
}

static bool NST_CALLBACK nst_poll_excitingboxing(Input::UserData data,
    Input::Controllers::ExcitingBoxing& excitingBoxing, unsigned part) {

    jg_inputstate_t *ebdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    switch (part) {
        case Input::Controllers::ExcitingBoxing::PART_1:
            if (ebdev->button[5])
                buttons |=
                    Input::Controllers::ExcitingBoxing::PART_1_RIGHT_HOOK;
            if (ebdev->button[3])
                buttons |=
                    Input::Controllers::ExcitingBoxing::PART_1_RIGHT_MOVE;
            if (ebdev->button[4])
                buttons |= Input::Controllers::ExcitingBoxing::PART_1_LEFT_HOOK;
            if (ebdev->button[2])
                buttons |= Input::Controllers::ExcitingBoxing::PART_1_LEFT_MOVE;
            break;
        case Input::Controllers::ExcitingBoxing::PART_2:
            if (ebdev->button[0])
                buttons |= Input::Controllers::ExcitingBoxing::PART_2_STRAIGHT;
            if (ebdev->button[7])
                buttons |= Input::Controllers::ExcitingBoxing::PART_2_RIGHT_JAB;
            if (ebdev->button[1])
                buttons |= Input::Controllers::ExcitingBoxing::PART_2_BODY;
            if (ebdev->button[6])
                buttons |= Input::Controllers::ExcitingBoxing::PART_2_LEFT_JAB;
            break;
    }

    excitingBoxing.buttons = buttons;
    return true;
}

static bool NST_CALLBACK nst_poll_toprider(Input::UserData data,
    Input::Controllers::TopRider& topRider) {

    jg_inputstate_t *trdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    if (trdev->button[0]) buttons |= Input::Controllers::TopRider::SHIFT_GEAR;
    if (trdev->button[1]) buttons |= Input::Controllers::TopRider::REAR;
    if (trdev->button[2]) buttons |= Input::Controllers::TopRider::STEER_LEFT;
    if (trdev->button[3]) buttons |= Input::Controllers::TopRider::STEER_RIGHT;
    if (trdev->button[4]) buttons |= Input::Controllers::TopRider::SELECT;
    if (trdev->button[5]) buttons |= Input::Controllers::TopRider::START;
    if (trdev->button[6]) buttons |= Input::Controllers::TopRider::ACCEL;
    if (trdev->button[7]) buttons |= Input::Controllers::TopRider::BRAKE;

    topRider.buttons = buttons;
    return true;
}

static bool NST_CALLBACK nst_poll_pokkunmoguraa(Input::UserData data,
    Input::Controllers::PokkunMoguraa& pokkunMoguraa, unsigned row) {

    jg_inputstate_t *pmdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    if (row & Input::Controllers::PokkunMoguraa::ROW_1) {
        if (pmdev->button[0])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_1;
        if (pmdev->button[1])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_2;
        if (pmdev->button[2])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_3;
        if (pmdev->button[3])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_4;
    }
    if (row & Input::Controllers::PokkunMoguraa::ROW_2) {
        if (pmdev->button[4])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_1;
        if (pmdev->button[5])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_2;
        if (pmdev->button[6])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_3;
        if (pmdev->button[7])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_4;
    }
    if (row & Input::Controllers::PokkunMoguraa::ROW_3) {
        if (pmdev->button[8])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_1;
        if (pmdev->button[9])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_2;
        if (pmdev->button[10])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_3;
        if (pmdev->button[11])
            buttons |= Input::Controllers::PokkunMoguraa::BUTTON_4;
    }

    pokkunMoguraa.buttons = buttons;
    return true;
}

static bool NST_CALLBACK nst_poll_partytap(Input::UserData data,
    Input::Controllers::PartyTap& partyTap) {

    jg_inputstate_t *ptdev = static_cast<jg_inputstate_t*>(data);
    unsigned units = 0;

    if (ptdev->button[0]) units |= Input::Controllers::PartyTap::UNIT_1;
    if (ptdev->button[1]) units |= Input::Controllers::PartyTap::UNIT_2;
    if (ptdev->button[2]) units |= Input::Controllers::PartyTap::UNIT_3;
    if (ptdev->button[3]) units |= Input::Controllers::PartyTap::UNIT_4;
    if (ptdev->button[4]) units |= Input::Controllers::PartyTap::UNIT_5;
    if (ptdev->button[5]) units |= Input::Controllers::PartyTap::UNIT_6;

    partyTap.units = units;
    return true;
}

static bool NST_CALLBACK nst_poll_vssys(Input::UserData data,
    Input::Controllers::VsSystem& vsSystem) {

    jg_inputstate_t *vsdev = static_cast<jg_inputstate_t*>(data);
    unsigned slots = 0;

    if (vsdev->button[0]) slots |= Input::Controllers::VsSystem::COIN_1;
    if (vsdev->button[1]) slots |= Input::Controllers::VsSystem::COIN_2;

    vsSystem.insertCoin = slots;
    return true;
}

static bool NST_CALLBACK nst_poll_karaokestudio(Input::UserData data,
    Input::Controllers::KaraokeStudio& karaokeStudio) {

    jg_inputstate_t *ksdev = static_cast<jg_inputstate_t*>(data);
    unsigned buttons = 0;

    if (ksdev->button[0]) buttons |= Input::Controllers::KaraokeStudio::A;
    if (ksdev->button[1]) buttons |= Input::Controllers::KaraokeStudio::B;
    buttons |= micstate; // 2nd bit for mic, like Famicom built-in mic

    karaokeStudio.buttons = buttons;
    return true;
}

static void nst_params_input(void) {
    // Autoselect the adapter
    Input(emulator).AutoSelectAdapter();

    // Select input devices for controller ports
    for (int i = 0; i < 4; ++i) {
        switch (settings_nst[PORT1 + i].val) {
            case 0: default:
                Input(emulator).AutoSelectController(i); break;
            case 1:
                Input(emulator).ConnectController(i,
                    (Input::Type)(Input::PAD1 + i)); break;
            case 2:
                Input(emulator).ConnectController(i, Input::ZAPPER); break;
            case 3:
                Input(emulator).ConnectController(i, Input::PADDLE); break;
            case 4:
                Input(emulator).ConnectController(i, Input::POWERPAD); break;
            case 5:
                Input(emulator).ConnectController(i, Input::POWERGLOVE); break;
        }
    }

    // Select input device for expansion port
    switch (settings_nst[PORTEXP].val) {
        case 0: default:
            Input(emulator).AutoSelectController(4); break;
        case 1:
            Input(emulator).ConnectController(4, Input::FAMILYTRAINER); break;
        case 2:
            Input(emulator).ConnectController(4, Input::PACHINKO); break;
        case 3:
            Input(emulator).ConnectController(4, Input::OEKAKIDSTABLET); break;
        case 4:
            Input(emulator).ConnectController(4, Input::KONAMIHYPERSHOT); break;
        case 5:
            Input(emulator).ConnectController(4, Input::BANDAIHYPERSHOT); break;
        case 6:
            Input(emulator).ConnectController(4, Input::CRAZYCLIMBER); break;
        case 7:
            Input(emulator).ConnectController(4, Input::MAHJONG); break;
        case 8:
            Input(emulator).ConnectController(4, Input::EXCITINGBOXING); break;
        case 9:
            Input(emulator).ConnectController(4, Input::TOPRIDER); break;
        case 10:
            Input(emulator).ConnectController(4, Input::POKKUNMOGURAA); break;
        case 11:
            Input(emulator).ConnectController(4, Input::PARTYTAP); break;
    }

    for (int i = 0; i < NUMINPUTS; ++i) {
        int type = Input(emulator).GetConnectedController(i);
        i < 4 ? jg_cb_log(JG_LOG_DBG, "Controller Port %d: %s\n", i + 1,
            jg_nes_input_name[type]) :
        jg_cb_log(JG_LOG_DBG, "Expansion Port: %s\n", jg_nes_input_name[type]);

        // Assign input info for connected devices
        inputinfo[i] = jg_nes_inputinfo(i, type);

        if (kstudio)
            inputinfo[4] = jg_nes_inputinfo(4, JG_NES_KARAOKESTUDIO);

        switch (type) {
            case Input::ZAPPER: {
                Input::Controllers::Zapper::callback.Set(nst_poll_zapper,
                    input_device[i]);
                break;
            }
            case Input::PADDLE: {
                Input::Controllers::Paddle::callback.Set(nst_poll_paddle,
                    input_device[i]);
                break;
            }
            case Input::POWERPAD: {
                Input::Controllers::PowerPad::callback.Set(nst_poll_powerpad,
                    input_device[i]);
                break;
            }
            case Input::FAMILYTRAINER: {
                Input::Controllers::FamilyTrainer::callback.Set(
                    nst_poll_familytrainer, input_device[i]);
                break;
            }
            case Input::POWERGLOVE: {
                Input::Controllers::PowerGlove::callback.Set(
                    nst_poll_powerglove, input_device[i]);
                break;
            }
            case Input::PACHINKO: {
                Input::Controllers::Pachinko::callback.Set(
                    nst_poll_pachinko, input_device[i]);
                break;
            }
            case Input::OEKAKIDSTABLET: {
                Input::Controllers::OekaKidsTablet::callback.Set(
                    nst_poll_oekakids, input_device[i]);
                break;
            }
            case Input::KONAMIHYPERSHOT: {
                Input::Controllers::KonamiHyperShot::callback.Set(
                    nst_poll_konamihypershot, input_device[i]);
                break;
            }
            case Input::BANDAIHYPERSHOT: {
                Input::Controllers::BandaiHyperShot::callback.Set(
                    nst_poll_bandaihypershot, input_device[i]);
                break;
            }
            case Input::CRAZYCLIMBER: {
                Input::Controllers::CrazyClimber::callback.Set(
                    nst_poll_crazyclimber, input_device[i]);
                break;
            }
            case Input::MAHJONG: {
                Input::Controllers::Mahjong::callback.Set(nst_poll_mahjong,
                    input_device[i]);
                break;
            }
            case Input::EXCITINGBOXING: {
                Input::Controllers::ExcitingBoxing::callback.Set(
                    nst_poll_excitingboxing, input_device[i]);
                break;
            }
            case Input::TOPRIDER: {
                Input::Controllers::TopRider::callback.Set(nst_poll_toprider,
                    input_device[i]);
                break;
            }
            case Input::POKKUNMOGURAA: {
                Input::Controllers::PokkunMoguraa::callback.Set(
                    nst_poll_pokkunmoguraa, input_device[i]);
                break;
            }
            case Input::PARTYTAP: {
                Input::Controllers::PartyTap::callback.Set(nst_poll_partytap,
                    input_device[i]);
                break;
            }
            default: break;
        }
    }

    // Set Pad callback separately
    Input::Controllers::Pad::callback.Set(nst_poll_pad, 0);
}

static void nst_params_video(void) {
    // Set up video parameters
    int renderwidth = settings_nst[NTSC].val ?
        Video::Output::NTSC_WIDTH : Video::Output::WIDTH;

    unsigned aspect_w = Video::Output::WIDTH - (settings_nst[OVERSCAN_L].val +
        settings_nst[OVERSCAN_R].val);
    unsigned w = renderwidth - (settings_nst[OVERSCAN_L].val +
        settings_nst[OVERSCAN_R].val);
    unsigned h = Video::Output::HEIGHT - (settings_nst[OVERSCAN_T].val +
        settings_nst[OVERSCAN_B].val);

    vidinfo.w = w;
    vidinfo.h = h;
    vidinfo.x = settings_nst[OVERSCAN_L].val;
    vidinfo.y = settings_nst[OVERSCAN_T].val;

    bool pal = Machine(emulator).GetMode() == Machine::PAL;
    vidinfo.aspect = (aspect_w * (pal ? ASPECT_PAL : ASPECT_NTSC)) / (double)h;

    Video video(emulator);
    Video::RenderState renderState;

    renderState.bits.mask.r = 0x00ff0000;
    renderState.bits.mask.g = 0x0000ff00;
    renderState.bits.mask.b = 0x000000ff;

    renderState.filter = settings_nst[NTSC].val ?
        Video::RenderState::FILTER_NTSC : Video::RenderState::FILTER_NONE;
    renderState.width = renderwidth;
    renderState.height = Video::Output::HEIGHT;
    renderState.bits.count = 32;

    switch (settings_nst[NTSCMODE].val) {
        case 0: { // Composite
            video.SetSaturation(Video::DEFAULT_SATURATION_COMP);
            video.SetSharpness(Video::DEFAULT_SHARPNESS_COMP);
            video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_COMP);
            video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_COMP);
            video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_COMP);
            video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_COMP);
            break;
        }
        case 1: { // S-Video
            video.SetSaturation(Video::DEFAULT_SATURATION_SVIDEO);
            video.SetSharpness(Video::DEFAULT_SHARPNESS_SVIDEO);
            video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
            video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_SVIDEO);
            video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
            video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_SVIDEO);
            break;
        }
        case 2: { // RGB
            video.SetSaturation(Video::DEFAULT_SATURATION_RGB);
            video.SetSharpness(Video::DEFAULT_SHARPNESS_RGB);
            video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_RGB);
            video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_RGB);
            video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_RGB);
            video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_RGB);
            break;
        }
        case 3: { // Monochrome
            video.SetSaturation(Video::DEFAULT_SATURATION_MONO);
            video.SetSharpness(Video::DEFAULT_SHARPNESS_MONO);
            video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_MONO);
            video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_MONO);
            video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_MONO);
            video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_MONO);
            break;
        }
    }

    // Path to external palette file
    std::string palpath = std::string(pathinfo.core) + "/palettes/";

    switch (settings_nst[PALETTE].val) {
        case 0: case 1: case 2: // YUV Palettes
            video.GetPalette().SetMode(Video::Palette::MODE_YUV);
            video.SetDecoder((Video::DecoderPreset)settings_nst[PALETTE].val);
            break;
        case 3: // RGB Palette
            video.GetPalette().SetMode(Video::Palette::MODE_RGB);
            break;
        case 4: { // Sony CXA2025AS
            palpath += "SONY_CXA2025AS_US.pal";
            break;
        }
        case 5: { // Royaltea
            palpath += "Royaltea.pal";
            break;
        }
        case 6: { // Nobilitea
            palpath += "Nobilitea.pal";
            break;
        }
        case 7: { // Digital Prime (FBX)
            palpath += "Digital_Prime_FBX.pal";
            break;
        }
        case 8: { // Magnum (FBX)
            palpath += "Magnum_FBX.pal";
            break;
        }
        case 9: { // PVM Style D93 (FBX)
            palpath += "PVM_Style_D93_FBX.pal";
            break;
        }
        case 10: { // Smooth V2 (FBX)
            palpath += "Smooth_V2_FBX.pal";
            break;
        }
        case 11: { // Custom
            palpath = std::string(pathinfo.user) + "/custom.pal";
            break;
        }
        default: break;
    }

    // Load the palette from an external file
    if (settings_nst[PALETTE].val > 3) {
        std::ifstream ifs(palpath.c_str(), std::ifstream::binary);
        if (ifs.is_open()) {
            std::filebuf *pbuf = ifs.rdbuf();
            std::size_t size = pbuf->pubseekoff(0, ifs.end, ifs.in);
            pbuf->pubseekpos(0, ifs.in);
            char *buffer = new char[size];
            pbuf->sgetn(buffer, size);
            video.GetPalette().SetMode(Video::Palette::MODE_CUSTOM);
            video.GetPalette().SetCustom((Video::Palette::Colors)buffer,
                Video::Palette::STD_PALETTE);
            ifs.close();
            delete[] buffer;
        }
    }

    if (!Machine(emulator).Is(Machine::DISK)) {
        switch (Cartridge(emulator).GetProfile()->system.type) {
            case Cartridge::Profile::System::VS_UNISYSTEM:
            case Cartridge::Profile::System::VS_DUALSYSTEM: {
                if (settings_nst[PALETTE].val >= 4) {
                    video.GetPalette().SetMode(Video::Palette::MODE_YUV);
                    video.SetDecoder(Video::DECODER_CONSUMER);
                }
            }
            default: break;
        }
    }


    video.EnableUnlimSprites(settings_nst[UNLIMITEDSPRITES].val);

    if (NES_FAILED(video.SetRenderState(renderState))) {
        jg_cb_log(JG_LOG_ERR, "Nestopia core rejected render state\n");
    }
}

void jg_set_cb_audio(jg_cb_audio_t func) {
    jg_cb_audio = func;
}

void jg_set_cb_frametime(jg_cb_frametime_t func) {
    jg_cb_frametime = func;
}

void jg_set_cb_log(jg_cb_log_t func) {
    jg_cb_log = func;
}

void jg_set_cb_rumble(jg_cb_rumble_t func) {
    jg_cb_rumble = func;
}

int jg_init(void) {
    if (!nst_db_load()) {
        jg_cb_log(JG_LOG_ERR, "Failed to load NstDatabase.xml\n");
        return 0;
    }

    NstVideo = new Video::Output;
    NstSound = new Sound::Output;
    NstInput = new Input::Controllers;

    // Set internal callbacks
    Video::Output::lockCallback.Set(nst_cb_videolock, 0);
    Video::Output::unlockCallback.Set(nst_cb_videounlock, 0);
    User::eventCallback.Set(nst_cb_event, 0);
    User::fileIoCallback.Set(nst_cb_file, 0);
    User::logCallback.Set(nst_cb_log, 0);

    return 1;
}

void jg_deinit(void) {
    if (NstVideo)
        delete NstVideo;

    if (NstSound)
        delete NstSound;

    if (NstInput)
        delete NstInput;
}

void jg_reset(int hard) {
    if (hard)
        Machine(emulator).SetRamPowerState(settings_nst[RAMPOWERSTATE].val);

    Machine(emulator).Reset(hard);
    Fds(emulator).EjectDisk();
    Fds(emulator).InsertDisk(0, 0);
}

void jg_exec_frame(void) {
    emulator.Execute(NstVideo, NstSound, NstInput);
}

int jg_game_load(void) {
    Machine machine(emulator);
    Nes::Result result;

    // Load the game into memory
    std::string rombuf((const char*)gameinfo.data, gameinfo.size);
    std::istringstream file(rombuf);

    // Check if it's an FDS game
    if (strstr(gameinfo.fname, ".fds") || strstr(gameinfo.fname, ".FDS")) {
        Fds fds(emulator);

        // Load the FDS BIOS as an auxiliary file if one was specified
        if (biosinfo.size) {
            std::string biosbuf((const char*)biosinfo.data, biosinfo.size);
            std::istringstream fdsbios(biosbuf);
            fds.SetBIOS(&fdsbios);
        }
        else { // Load the BIOS from the default path
            // Set up the path to the FDS BIOS
            std::string fdspath = std::string(pathinfo.bios) + "/disksys.rom";
            jg_cb_log(JG_LOG_INF, "FDS BIOS Path: %s\n", fdspath.c_str());

            // Load the FDS BIOS
            std::ifstream *fdsbios = new std::ifstream(fdspath.c_str(),
                std::ifstream::in|std::ifstream::binary);
            if (fdsbios->is_open()) {
                fds.SetBIOS(fdsbios);
                delete fdsbios;
            }
            else {
                jg_cb_log(JG_LOG_INF, "Failed to load FDS BIOS\n");
            }
        }
    }

    if (settings_nst[SOFTPATCH].val) {
        // Attempt to open an IPS patch matching the filename of the loaded ROM
        std::string ppath(gameinfo.path);
        ppath = ppath.substr(0, ppath.find_last_of("."))  + ".ips";
        std::ifstream pfile(ppath.c_str(), std::ios::in|std::ios::binary);

        // If there is no IPS patch, try to find a UPS patch
        if (!pfile.is_open()) {
            ppath = ppath.substr(0, ppath.find_last_of("."))  + ".ups";
            std::ifstream pfile(ppath.c_str(), std::ios::in|std::ios::binary);
        }

        // If a patch was found, apply it
        if (pfile.is_open()) {
            Machine::Patch patch(pfile);
            result = machine.Load(file,
                (Machine::FavoredSystem)settings_nst[FAVSYSTEM].val, patch);
            jg_cb_log(JG_LOG_INF, "Patch Applied: %s\n", ppath.c_str());
        }
        else {
            result = machine.Load(file,
                (Machine::FavoredSystem)settings_nst[FAVSYSTEM].val);
        }
    }
    else {
        result = machine.Load(file,
            (Machine::FavoredSystem)settings_nst[FAVSYSTEM].val);
    }

    if (NES_FAILED(result)) {
        switch (result) {
            case Nes::RESULT_ERR_INVALID_FILE:
                jg_cb_log(JG_LOG_ERR, "Invalid file\n");
                break;
            case Nes::RESULT_ERR_OUT_OF_MEMORY:
                jg_cb_log(JG_LOG_ERR, "Out of Memory\n");
                break;
            case Nes::RESULT_ERR_CORRUPT_FILE:
                jg_cb_log(JG_LOG_ERR, "Corrupt or Missing File\n");
                break;
            case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:
                jg_cb_log(JG_LOG_ERR, "Unsupported Mapper\n");
                break;
            case Nes::RESULT_ERR_MISSING_BIOS:
                jg_cb_log(JG_LOG_ERR, "Missing FDS BIOS\n");
                break;
            default:
                jg_cb_log(JG_LOG_ERR, "%d\n", result);
                break;
        }
        return 0;
    }

    // Force the region based on "Favored System", or use the automatic region
    if (settings_nst[FORCEREGION].val) {
        if (settings_nst[FAVSYSTEM].val & 0x1)
            machine.SetMode(Machine::PAL);
        else
            machine.SetMode(Machine::NTSC);
    }
    else {
        machine.SetMode(machine.GetDesiredMode());
    }

    // Adjustments for PAL mode
    if (machine.GetMode() == Machine::PAL) {
        audinfo.spf = (SAMPLERATE / (unsigned)FRAMERATE_PAL) * CHANNELS;
        jg_cb_frametime(FRAMERATE_PAL);
    }
    else {
        audinfo.spf = (SAMPLERATE / (unsigned)FRAMERATE) * CHANNELS;
        jg_cb_frametime(FRAMERATE);
    }

    nst_params_input();

    // Set up FDS and Vs. System
    if (machine.Is(Machine::DISK)) { // Auto-insert FDS Disk
        coreinfo.hints |= JG_HINT_INPUT_AUDIO;
        Fds fds(emulator);
        fds.InsertDisk(0, 0);
    }
    else { // Famicom and VS. System
        const Cartridge::Profile *profile = Cartridge(emulator).GetProfile();

        switch (profile->system.type) {
            case Cartridge::Profile::System::FAMICOM: {
                coreinfo.hints |= JG_HINT_INPUT_AUDIO;

                // Jump through flaming hoops to get the mapper number
                unsigned mapper = Cartridge::Database(emulator).FindEntry(
                    profile->hash, Machine::FAVORED_FAMICOM).GetMapper();

                // Check if it's Karaoke Studio
                if (mapper == 188) {
                    inputinfo[4] = jg_nes_inputinfo(4, JG_NES_KARAOKESTUDIO);
                    Input::Controllers::KaraokeStudio::callback.Set(
                        nst_poll_karaokestudio, input_device[4]);
                    kstudio = true;
                }
                break;
            }
            case Cartridge::Profile::System::VS_UNISYSTEM:
            case Cartridge::Profile::System::VS_DUALSYSTEM: {
                inputinfo[4] = jg_nes_inputinfo(4, JG_NES_VSSYS);
                Input::Controllers::VsSystem::callback.Set(nst_poll_vssys,
                    input_device[4]);
                break;
            }
            default: break;
        }
    }

    // Set the RAM Power State
    machine.SetRamPowerState(settings_nst[RAMPOWERSTATE].val);

    // Start the machine
    machine.Power(true);

    return 1;
}

int jg_game_unload(void) {
    Machine(emulator).Power(false);
    Machine(emulator).Unload();
    return 1;
}

int jg_state_load(const char *filename) {
    std::ifstream statefile(filename, std::ifstream::in|std::ifstream::binary);
    if (statefile.is_open()) {
        Machine(emulator).LoadState(statefile);
        statefile.close();
        return 1;
    }
    return 0;
}

void jg_state_load_raw(const void *data) {
    if (data) { }
}

int jg_state_save(const char *filename) {
    std::ofstream statefile(filename, std::ifstream::out|std::ifstream::binary);
    if (statefile.is_open()) {
        Machine(emulator).SaveState(statefile,
            Nes::Api::Machine::NO_COMPRESSION);
        statefile.close();
        return 1;
    }
    return 0;
}

const void* jg_state_save_raw(void) {
    return NULL;
}

size_t jg_state_size(void) {
    return 0;
}

void jg_media_select(void) {
    if (Machine(emulator).Is(Machine::DISK)) {
        Fds fds(emulator);
        int disk = fds.GetCurrentDisk();
        int side = fds.GetCurrentDiskSide();

        // Switch disks if it's a multi-disk game on Side B
        if (fds.GetNumDisks() > 1 && side == 1) {
            fds.EjectDisk();
            fds.InsertDisk(!disk, 0);
        }
        else if (fds.CanChangeDiskSide()) {
            fds.ChangeSide();
        }

        nst_fds_info();
    }
}

void jg_media_insert(void) {
    if (Machine(emulator).Is(Machine::DISK)) {
        Fds fds(emulator);
        fds.IsAnyDiskInserted() ? fds.EjectDisk() : fds.InsertDisk(0, 0);
        nst_fds_info();
    }
}

void jg_cheat_clear(void) {
    Cheats cheats(emulator);
    cheats.ClearCodes();
}

void jg_cheat_set(const char *code) {
    Cheats cheats(emulator);
    Cheats::Code cheatcode;

    // Check if the cheat is Raw
    if (strstr(code, "0x")) {
        std::vector<std::string> tokens;
        std::stringstream rawcode(code);
        std::string token;

        while (std::getline(rawcode, token, ' '))
            tokens.push_back(token);

        if ((tokens.size() == 2) || (tokens.size() == 3)) {
            cheatcode.address = strtoul(tokens[0].c_str(), NULL, 16);
            cheatcode.value = strtoul(tokens[1].c_str(), NULL, 16);

            if (tokens.size() == 3) {
                cheatcode.compare = strtoul(tokens[2].c_str(), NULL, 16);
                cheatcode.useCompare = true;
            }

            cheats.SetCode(cheatcode);
        }
        else {
            jg_cb_log(JG_LOG_WRN, "Invalid cheat: %s\n", code);
        }
    }
    // Game Genie
    else if (cheats.GameGenieDecode(code, cheatcode) == Nes::RESULT_OK) {
        cheats.SetCode(cheatcode);
    }
    // Pro Action Rocky
    else if (cheats.ProActionRockyDecode(code, cheatcode) == Nes::RESULT_OK) {
        cheats.SetCode(cheatcode);
    }
    else {
        jg_cb_log(JG_LOG_WRN, "Invalid cheat: %s\n", code);
    }
}

void jg_rehash(void) {
    nst_params_video();
    nst_params_input();
}

void jg_data_push(uint32_t type, int port, const void *ptr, size_t size) {
    if (port) { }

    if (type == JG_DATA_AUDIO) {
        int16_t *buf = (int16_t*)(((jg_audioinfo_t*)ptr)->buf);
        size_t total = 0;

        for (size_t i = 0; i < size; ++i)
            total += abs(buf[i]);

        if (total / size > 2500)
            micstate ^= 0x04;
    }
}

jg_coreinfo_t* jg_get_coreinfo(const char *sys) {
    return &coreinfo;
}

jg_videoinfo_t* jg_get_videoinfo(void) {
    return &vidinfo;
}

jg_audioinfo_t* jg_get_audioinfo(void) {
    return &audinfo;
}

jg_inputinfo_t* jg_get_inputinfo(int port) {
    return &inputinfo[port];
}

jg_setting_t* jg_get_settings(size_t *numsettings) {
    *numsettings = sizeof(settings_nst) / sizeof(jg_setting_t);
    return settings_nst;
}

void jg_setup_video(void) {
    nst_params_video();
}

void jg_setup_audio(void) {
    NstSound->samples[0] = audinfo.buf;
    NstSound->length[0] = audinfo.spf;
    NstSound->samples[1] = NULL;
    NstSound->length[1] = 0;

    // Set up sound parameters
    Sound sound(emulator);
    sound.SetSampleRate(audinfo.rate);
    sound.SetSpeaker(Sound::SPEAKER_MONO);
    sound.SetSpeed(Sound::DEFAULT_SPEED);
    sound.SetVolume(Sound::ALL_CHANNELS, 100);
    sound.SetGenie(settings_nst[GENIEDISTORTION].val);

    Sound::Output::lockCallback.Set(nst_cb_soundlock, 0);
    Sound::Output::unlockCallback.Set(nst_cb_soundunlock, 0);
}

void jg_set_inputstate(jg_inputstate_t *ptr, int port) {
    input_device[port] = ptr;
}

void jg_set_gameinfo(jg_fileinfo_t info) {
    gameinfo = info;
}

void jg_set_auxinfo(jg_fileinfo_t info, int index) {
    if (index)
        return;
    biosinfo = info;
}

void jg_set_paths(jg_pathinfo_t paths) {
    pathinfo = paths;
}
