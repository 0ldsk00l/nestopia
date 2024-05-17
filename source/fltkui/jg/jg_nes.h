/*
zlib License

Copyright (c) 2020-2022 Rupert Carmichael

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef JG_NES_H
#define JG_NES_H

enum jg_nes_input_type {
    JG_NES_UNCONNECTED,
    JG_NES_PAD1,
    JG_NES_PAD2,
    JG_NES_PAD3,
    JG_NES_PAD4,
    JG_NES_ZAPPER,
    JG_NES_ARKANOID,
    JG_NES_POWERPAD,
    JG_NES_POWERGLOVE,
    JG_NES_MOUSE,
    JG_NES_ROB,
    JG_NES_FAMILYTRAINER,
    JG_NES_FAMILYKEYBOARD,
    JG_NES_SUBORKEYBOARD,
    JG_NES_DOREMIKKOKEYBOARD,
    JG_NES_HORITRACK,
    JG_NES_PACHINKO,
    JG_NES_OEKAKIDSTABLET,
    JG_NES_KONAMIHYPERSHOT,
    JG_NES_BANDAIHYPERSHOT,
    JG_NES_CRAZYCLIMBER,
    JG_NES_MAHJONG,
    JG_NES_EXCITINGBOXING,
    JG_NES_TOPRIDER,
    JG_NES_POKKUNMOGURAA,
    JG_NES_PARTYTAP,
    JG_NES_TURBOFILE,
    JG_NES_BARCODEWORLD,
    JG_NES_VSSYS,
    JG_NES_KARAOKESTUDIO
};

static const char *jg_nes_input_name[] = {
    "Unconnected",
    "Controller 1", "Controller 2", "Controller 3", "Controller 4",
    "Zapper",
    "Arkanoid Paddle",
    "Power Pad",
    "Power Glove",
    "Mouse", // todo
    "R.O.B.",
    "Family Trainer",
    "Family Keyboard", // todo
    "Subor Keyboard", // todo
    "Doremikko Keyboard", // todo
    "Hori Track", // todo
    "Pachinko",
    "Oeka Kids Tablet",
    "Konami Hypershot",
    "Bandai Hypershot",
    "Crazy Climber",
    "Mahjong",
    "Exciting Boxing",
    "Top Rider",
    "Pokkun Moguraa",
    "PartyTap",
    "Turbo File", // edge case, not really an input device
    "Barcode World", // todo
    "Vs. System",
    "Karaoke Studio"
};

// NES Controller (Pad)
#define NDEFS_NESPAD 10
static const char *defs_nespad[NDEFS_NESPAD] = {
    "Up", "Down", "Left", "Right","Select", "Start",
    "A", "B", "TurboA", "TurboB"
};

// NES Zapper
#define NDEFS_ZAPPER 2
static const char *defs_zapper[NDEFS_ZAPPER] = { "Fire", "Offscreen" };

// Arkanoid Paddle
#define NDEFS_ARKANOID 2
static const char *defs_arkanoid[NDEFS_ARKANOID] = { "XAxis", "Button" };

// Power Pad
#define NDEFS_POWERPAD 20
static const char *defs_powerpad[NDEFS_POWERPAD] = {
    "SideA1", "SideA2", "SideA3", "SideA4", "SideA5",
    "SideA6", "SideA7", "SideA8", "SideA9", "SideA10",
    "SideA11", "SideA12", "SideB3", "SideB2", "SideB8",
    "SideB7", "SideB6", "SideB5", "SideB11", "SideB10"
};

// Power Glove
#define NDEFS_POWERGLOVE 8
static const char *defs_powerglove[NDEFS_POWERGLOVE] = {
    "Select", "Start", "MoveIn", "MoveOut",
    "RollLeft", "RollRight", "Fist", "Finger"
};

// Pachinko
#define NDEFS_PACHINKO 9
static const char *defs_pachinko[NDEFS_PACHINKO] = {
    "Throttle", "Up", "Down", "Left", "Right", "Select", "Start", "A", "B"
};

// Oeka Kids Tablet
#define NDEFS_OEKAKIDS 1
static const char *defs_oekakids[NDEFS_OEKAKIDS] = { "Button" };

// Konami Hypershot
#define NDEFS_KONAMIHYPERSHOT 4
static const char *defs_konamihypershot[NDEFS_KONAMIHYPERSHOT] = {
    "Player1B1", "Player1B2", "Player2B1", "Player2B2"
};

// Bandai Hypershot
#define NDEFS_BANDAIHYPERSHOT 3
static const char *defs_bandaihypershot[NDEFS_BANDAIHYPERSHOT] = {
    "Fire", "Offscreen", "Move"
};

#define NDEFS_CRAZYCLIMBER 8
static const char *defs_crazyclimber[NDEFS_CRAZYCLIMBER] = {
    "LeftUp", "LeftDown", "LeftLeft", "LeftRight",
    "RightUp", "RightDown", "RightLeft", "RightRight",
};

// Mahjong
#define NDEFS_MAHJONG 21
static const char *defs_mahjong[NDEFS_MAHJONG] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "Start", "Select", "Kan", "Pon", "Chi", "Reach", "Ron"
};

// Exciting Boxing
#define NDEFS_EXCITINGBOXING 8
static const char *defs_excitingboxing[NDEFS_EXCITINGBOXING] = {
    "Straight", "Body", "Left", "Right",
    "LeftHook", "RightHook", "LeftJab", "RightJab"
};

// Top Rider
#define NDEFS_TOPRIDER 8
static const char *defs_toprider[NDEFS_TOPRIDER] = {
    "ShiftGear", "Rear", "Left", "Right",
    "Select", "Start", "Accelerate", "Brake"
};

// Pokkun Moguraa
#define NDEFS_POKKUNMOGURAA 12
static const char *defs_pokkunmoguraa[NDEFS_POKKUNMOGURAA] = {
    "Row1B1", "Row1B2", "Row1B3", "Row1B4",
    "Row2B1", "Row2B2", "Row2B3", "Row2B4",
    "Row3B1", "Row3B2", "Row3B3", "Row3B4"
};

// PartyTap
#define NDEFS_PARTYTAP 6
static const char *defs_partytap[NDEFS_PARTYTAP] = {
    "Unit1", "Unit2", "Unit3", "Unit4", "Unit5", "Unit6" 
};

// Vs. System
#define NDEFS_VSSYS 2
static const char *defs_vssys[NDEFS_VSSYS] = { "Coin1", "Coin2" };

// Vs. System
#define NDEFS_KARAOKESTUDIO 2
static const char *defs_karaokestudio[NDEFS_VSSYS] = { "A", "B" };

static jg_inputinfo_t jg_nes_inputinfo(int index, int type) {
    jg_inputinfo_t ret;
    switch (type) {
        case JG_NES_PAD1: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "nespad1";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_nespad;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_NESPAD;
            return ret;
        }
        case JG_NES_PAD2: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "nespad2";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_nespad;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_NESPAD;
            return ret;
        }
        case JG_NES_PAD3: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "nespad3";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_nespad;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_NESPAD;
            return ret;
        }
        case JG_NES_PAD4: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "nespad4";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_nespad;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_NESPAD;
            return ret;
        }
        case JG_NES_ZAPPER: {
            ret.type = JG_INPUT_GUN;
            ret.index = index;
            ret.name = "zapper";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_zapper;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_ZAPPER;
            return ret;
        }
        case JG_NES_ARKANOID: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "arkanoid";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_arkanoid;
            ret.numaxes = 1;
            ret.numbuttons = NDEFS_ARKANOID - 1;
            return ret;
        }
        case JG_NES_POWERPAD: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "powerpad";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_powerpad;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_POWERPAD;
            return ret;
        }
        case JG_NES_FAMILYTRAINER: { // Family Trainer == Power Pad
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "powerpad";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_powerpad;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_POWERPAD;
            return ret;
        }
        case JG_NES_POWERGLOVE: {
            ret.type = JG_INPUT_POINTER;
            ret.index = index;
            ret.name = "powerglove";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_powerglove;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_POWERGLOVE;
            return ret;
        }
        case JG_NES_PACHINKO: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "pachinko";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_pachinko;
            ret.numaxes = 1;
            ret.numbuttons = NDEFS_PACHINKO - 1;
            return ret;
        }
        case JG_NES_OEKAKIDSTABLET: {
            ret.type = JG_INPUT_POINTER;
            ret.index = index;
            ret.name = "oekakids";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_oekakids;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_OEKAKIDS;
            return ret;
        }
        case JG_NES_KONAMIHYPERSHOT: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "konamihypershot";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_konamihypershot;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_KONAMIHYPERSHOT;
            return ret;
        }
        case JG_NES_BANDAIHYPERSHOT: {
            ret.type = JG_INPUT_GUN;
            ret.index = index;
            ret.name = "bandaihypershot";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_bandaihypershot;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_BANDAIHYPERSHOT;
            return ret;
        }
        case JG_NES_CRAZYCLIMBER: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "crazyclimber";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_crazyclimber;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_CRAZYCLIMBER;
            return ret;
        }
        case JG_NES_MAHJONG: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "mahjong";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_mahjong;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_MAHJONG;
            return ret;
        }
        case JG_NES_EXCITINGBOXING: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "excitingboxing";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_excitingboxing;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_EXCITINGBOXING;
            return ret;
        }
        case JG_NES_TOPRIDER: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "toprider";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_toprider;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_TOPRIDER;
            return ret;
        }
        case JG_NES_POKKUNMOGURAA: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "pokkunmoguraa";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_pokkunmoguraa;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_POKKUNMOGURAA;
            return ret;
        }
        case JG_NES_PARTYTAP: {
            ret.type = JG_INPUT_CONTROLLER;
            ret.index = index;
            ret.name = "partytap";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_partytap;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_PARTYTAP;
            return ret;
        }
        case JG_NES_VSSYS: {
            ret.type = JG_INPUT_EXTERNAL;
            ret.index = index;
            ret.name = "vssys";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_vssys;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_VSSYS;
            return ret;
        }
        case JG_NES_KARAOKESTUDIO: {
            ret.type = JG_INPUT_EXTERNAL;
            ret.index = index;
            ret.name = "karaokestudio";
            ret.fname = jg_nes_input_name[type];
            ret.defs = defs_karaokestudio;
            ret.numaxes = 0;
            ret.numbuttons = NDEFS_KARAOKESTUDIO;
            return ret;
        }
        default: {
            ret.type = JG_INPUT_EXTERNAL;
            ret.index = index;
            ret.name = "unconnected";
            ret.fname = jg_nes_input_name[0];
            ret.defs = NULL;
            ret.numaxes = ret.numbuttons = 0;
            return ret;
        }
    }
}

#endif
