////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include "NstResourceString.hpp"
#include "NstIoLog.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstManagerEmulator.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDynamic.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogInput.hpp"
#include <Shlwapi.h>

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			DirectX::DirectInput::AXIS_X        == 0x001 &&
			DirectX::DirectInput::AXIS_Y        == 0x002 &&
			DirectX::DirectInput::AXIS_Z        == 0x004 &&
			DirectX::DirectInput::AXIS_RX       == 0x008 &&
			DirectX::DirectInput::AXIS_RY       == 0x010 &&
			DirectX::DirectInput::AXIS_RZ       == 0x020 &&
			DirectX::DirectInput::AXIS_SLIDER_0 == 0x040 &&
			DirectX::DirectInput::AXIS_SLIDER_1 == 0x080 &&
			DirectX::DirectInput::AXIS_POV_0    == 0x100 &&
			DirectX::DirectInput::AXIS_POV_1    == 0x200 &&
			DirectX::DirectInput::AXIS_POV_2    == 0x400 &&
			DirectX::DirectInput::AXIS_POV_3    == 0x800 &&

			IDC_INPUT_JOYSTICKS_ENABLE        == IDC_INPUT_JOYSTICKS +  1 &&
			IDC_INPUT_JOYSTICKS_X             == IDC_INPUT_JOYSTICKS +  2 &&
			IDC_INPUT_JOYSTICKS_Y             == IDC_INPUT_JOYSTICKS +  3 &&
			IDC_INPUT_JOYSTICKS_Z             == IDC_INPUT_JOYSTICKS +  4 &&
			IDC_INPUT_JOYSTICKS_RX            == IDC_INPUT_JOYSTICKS +  5 &&
			IDC_INPUT_JOYSTICKS_RY            == IDC_INPUT_JOYSTICKS +  6 &&
			IDC_INPUT_JOYSTICKS_RZ            == IDC_INPUT_JOYSTICKS +  7 &&
			IDC_INPUT_JOYSTICKS_S0            == IDC_INPUT_JOYSTICKS +  8 &&
			IDC_INPUT_JOYSTICKS_S1            == IDC_INPUT_JOYSTICKS +  9 &&
			IDC_INPUT_JOYSTICKS_POV0          == IDC_INPUT_JOYSTICKS + 10 &&
			IDC_INPUT_JOYSTICKS_POV1          == IDC_INPUT_JOYSTICKS + 11 &&
			IDC_INPUT_JOYSTICKS_POV2          == IDC_INPUT_JOYSTICKS + 12 &&
			IDC_INPUT_JOYSTICKS_POV3          == IDC_INPUT_JOYSTICKS + 13 &&
			IDC_INPUT_JOYSTICKS_DEADZONE      == IDC_INPUT_JOYSTICKS + 14 &&
			IDC_INPUT_JOYSTICKS_DEADZONE_NUM  == IDC_INPUT_JOYSTICKS + 15 &&
			IDC_INPUT_JOYSTICKS_DEADZONE_TEXT == IDC_INPUT_JOYSTICKS + 16 &&
			IDC_INPUT_JOYSTICKS_CALIBRATE     == IDC_INPUT_JOYSTICKS + 17 &&
			IDC_INPUT_JOYSTICKS_DEFAULT       == IDC_INPUT_JOYSTICKS + 18 &&

			IDM_MACHINE_INPUT_PORT1_PAD1                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  1 &&
			IDM_MACHINE_INPUT_PORT1_PAD2                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  2 &&
			IDM_MACHINE_INPUT_PORT1_PAD3                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  3 &&
			IDM_MACHINE_INPUT_PORT1_PAD4                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  4 &&
			IDM_MACHINE_INPUT_PORT1_ZAPPER              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  5 &&
			IDM_MACHINE_INPUT_PORT1_PADDLE              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  6 &&
			IDM_MACHINE_INPUT_PORT1_POWERPAD            == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  7 &&
			IDM_MACHINE_INPUT_PORT1_POWERGLOVE          == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  8 &&
			IDM_MACHINE_INPUT_PORT1_MOUSE               == IDM_MACHINE_INPUT_PORT1_UNCONNECTED +  9 &&
			IDM_MACHINE_INPUT_PORT1_ROB                 == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 10 &&
			IDM_MACHINE_INPUT_PORT2_UNCONNECTED         == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 11 &&
			IDM_MACHINE_INPUT_PORT2_PAD1                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 12 &&
			IDM_MACHINE_INPUT_PORT2_PAD2                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 13 &&
			IDM_MACHINE_INPUT_PORT2_PAD3                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 14 &&
			IDM_MACHINE_INPUT_PORT2_PAD4                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 15 &&
			IDM_MACHINE_INPUT_PORT2_ZAPPER              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 16 &&
			IDM_MACHINE_INPUT_PORT2_PADDLE              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 17 &&
			IDM_MACHINE_INPUT_PORT2_POWERPAD            == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 18 &&
			IDM_MACHINE_INPUT_PORT2_POWERGLOVE          == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 19 &&
			IDM_MACHINE_INPUT_PORT2_MOUSE               == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 20 &&
			IDM_MACHINE_INPUT_PORT2_ROB                 == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 21 &&
			IDM_MACHINE_INPUT_PORT3_UNCONNECTED         == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 22 &&
			IDM_MACHINE_INPUT_PORT3_PAD1                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 23 &&
			IDM_MACHINE_INPUT_PORT3_PAD2                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 24 &&
			IDM_MACHINE_INPUT_PORT3_PAD3                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 25 &&
			IDM_MACHINE_INPUT_PORT3_PAD4                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 26 &&
			IDM_MACHINE_INPUT_PORT4_UNCONNECTED         == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 27 &&
			IDM_MACHINE_INPUT_PORT4_PAD1                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 28 &&
			IDM_MACHINE_INPUT_PORT4_PAD2                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 29 &&
			IDM_MACHINE_INPUT_PORT4_PAD3                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 30 &&
			IDM_MACHINE_INPUT_PORT4_PAD4                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 31 &&
			IDM_MACHINE_INPUT_EXP_UNCONNECTED           == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 32 &&
			IDM_MACHINE_INPUT_EXP_FAMILYTRAINER         == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 33 &&
			IDM_MACHINE_INPUT_EXP_FAMILYBASICKEYBOARD   == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 34 &&
			IDM_MACHINE_INPUT_EXP_SUBORKEYBOARD         == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 35 &&
			IDM_MACHINE_INPUT_EXP_DOREMIKKOKEYBOARD     == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 36 &&
			IDM_MACHINE_INPUT_EXP_HORITRACK             == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 37 &&
			IDM_MACHINE_INPUT_EXP_PACHINKO              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 38 &&
			IDM_MACHINE_INPUT_EXP_PADDLE                == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 39 &&
			IDM_MACHINE_INPUT_EXP_OEKAKIDSTABLET        == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 40 &&
			IDM_MACHINE_INPUT_EXP_KONAMIHYPERSHOT       == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 41 &&
			IDM_MACHINE_INPUT_EXP_BANDAIHYPERSHOT       == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 42 &&
			IDM_MACHINE_INPUT_EXP_CRAZYCLIMBER          == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 43 &&
			IDM_MACHINE_INPUT_EXP_MAHJONG               == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 44 &&
			IDM_MACHINE_INPUT_EXP_EXCITINGBOXING        == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 45 &&
			IDM_MACHINE_INPUT_EXP_TOPRIDER              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 46 &&
			IDM_MACHINE_INPUT_EXP_POKKUNMOGURAA         == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 47 &&
			IDM_MACHINE_INPUT_EXP_PARTYTAP              == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 48 &&
			IDM_MACHINE_INPUT_EXP_TURBOFILE             == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 49 &&
			IDM_MACHINE_INPUT_EXP_BARCODEWORLD          == IDM_MACHINE_INPUT_PORT1_UNCONNECTED + 50
		);

		const Input::Settings::Type Input::Settings::types[OFFSET_COUNT] =
		{
			{ PAD1_KEYS,           IDS_INPUT_KEY_SECTION_PAD1           },
			{ PAD2_KEYS,           IDS_INPUT_KEY_SECTION_PAD2           },
			{ PAD3_KEYS,           IDS_INPUT_KEY_SECTION_PAD3           },
			{ PAD4_KEYS,           IDS_INPUT_KEY_SECTION_PAD4           },
			{ POWERPAD_KEYS,       IDS_INPUT_KEY_SECTION_POWERPAD       },
			{ POWERGLOVE_KEYS,     IDS_INPUT_KEY_SECTION_POWERGLOVE     },
			{ HORITRACK_KEYS,      IDS_INPUT_KEY_SECTION_HORITRACK      },
			{ PACHINKO_KEYS,       IDS_INPUT_KEY_SECTION_PACHINKO       },
			{ CRAZYCLIMBER_KEYS,   IDS_INPUT_KEY_SECTION_CRAZYCLIMBER   },
			{ MAHJONG_KEYS,        IDS_INPUT_KEY_SECTION_MAHJONG        },
			{ EXCITINGBOXING_KEYS, IDS_INPUT_KEY_SECTION_EXCITINGBOXING },
			{ POKKUNMOGURAA_KEYS,  IDS_INPUT_KEY_SECTION_POKKUNMOGURAA  },
			{ PARTYTAP_KEYS,       IDS_INPUT_KEY_SECTION_PARTYTAP       },
			{ KARAOKESTUDIO_KEYS,  IDS_INPUT_KEY_SECTION_KARAOKESTUDIO  },
			{ EMULATION_KEYS,      IDS_INPUT_KEY_SECTION_EMULATION      },
			{ FILE_KEYS,           IDS_INPUT_KEY_SECTION_FILE           },
			{ MACHINE_KEYS,        IDS_INPUT_KEY_SECTION_MACHINE        },
			{ NSF_KEYS,            IDS_INPUT_KEY_SECTION_NSF            },
			{ VIEW_KEYS,           IDS_INPUT_KEY_SECTION_VIEW           },
			{ HELP_KEYS,           IDS_INPUT_KEY_SECTION_HELP           },
			{ NUM_KEYS,            0                                    }
		};

		const Input::Settings::Mapping Input::Settings::map[NUM_KEYS] =
		{
			{ PAD1_KEYS + PAD_KEY_LEFT,       DIK_LEFT,   IDS_INPUT_PAD_KEY_LEFT,       "pad-1", "left"        },
			{ PAD1_KEYS + PAD_KEY_UP,         DIK_UP,     IDS_INPUT_PAD_KEY_UP,         "pad-1", "up"          },
			{ PAD1_KEYS + PAD_KEY_RIGHT,      DIK_RIGHT,  IDS_INPUT_PAD_KEY_RIGHT,      "pad-1", "right"       },
			{ PAD1_KEYS + PAD_KEY_DOWN,       DIK_DOWN,   IDS_INPUT_PAD_KEY_DOWN,       "pad-1", "down"        },
			{ PAD1_KEYS + PAD_KEY_SELECT,     DIK_RSHIFT, IDS_INPUT_PAD_KEY_SELECT,     "pad-1", "select"      },
			{ PAD1_KEYS + PAD_KEY_START,      DIK_RETURN, IDS_INPUT_PAD_KEY_START,      "pad-1", "start"       },
			{ PAD1_KEYS + PAD_KEY_B,          DIK_COMMA,  IDS_INPUT_PAD_KEY_B,          "pad-1", "b"           },
			{ PAD1_KEYS + PAD_KEY_A,          DIK_PERIOD, IDS_INPUT_PAD_KEY_A,          "pad-1", "a"           },
			{ PAD1_KEYS + PAD_KEY_AUTOFIRE_B, DIK_K,      IDS_INPUT_PAD_KEY_AUTOFIRE_B, "pad-1", "auto-fire-b" },
			{ PAD1_KEYS + PAD_KEY_AUTOFIRE_A, DIK_L,      IDS_INPUT_PAD_KEY_AUTOFIRE_A, "pad-1", "auto-fire-a" },
			{ PAD1_KEYS + PAD_KEY_MIC,        NO_KEY,     IDS_INPUT_PAD_KEY_MIC,        "pad-1", "mic"         },

			{ PAD2_KEYS + PAD_KEY_LEFT,       DIK_C, IDS_INPUT_PAD_KEY_LEFT,       "pad-2", "left"        },
			{ PAD2_KEYS + PAD_KEY_UP,         DIK_F, IDS_INPUT_PAD_KEY_UP,         "pad-2", "up"          },
			{ PAD2_KEYS + PAD_KEY_RIGHT,      DIK_B, IDS_INPUT_PAD_KEY_RIGHT,      "pad-2", "right"       },
			{ PAD2_KEYS + PAD_KEY_DOWN,       DIK_V, IDS_INPUT_PAD_KEY_DOWN,       "pad-2", "down"        },
			{ PAD2_KEYS + PAD_KEY_SELECT,     DIK_A, IDS_INPUT_PAD_KEY_SELECT,     "pad-2", "select"      },
			{ PAD2_KEYS + PAD_KEY_START,      DIK_S, IDS_INPUT_PAD_KEY_START,      "pad-2", "start"       },
			{ PAD2_KEYS + PAD_KEY_B,          DIK_Z, IDS_INPUT_PAD_KEY_B,          "pad-2", "b"           },
			{ PAD2_KEYS + PAD_KEY_A,          DIK_X, IDS_INPUT_PAD_KEY_A,          "pad-2", "a"           },
			{ PAD2_KEYS + PAD_KEY_AUTOFIRE_B, DIK_Q, IDS_INPUT_PAD_KEY_AUTOFIRE_B, "pad-2", "auto-fire-b" },
			{ PAD2_KEYS + PAD_KEY_AUTOFIRE_A, DIK_W, IDS_INPUT_PAD_KEY_AUTOFIRE_A, "pad-2", "auto-fire-a" },
			{ PAD2_KEYS + PAD_KEY_MIC,        DIK_M, IDS_INPUT_PAD_KEY_MIC,        "pad-2", "mic"         },

			{ PAD3_KEYS + PAD_KEY_LEFT,       NO_KEY, IDS_INPUT_PAD_KEY_LEFT,       "pad-3", "left"        },
			{ PAD3_KEYS + PAD_KEY_UP,         NO_KEY, IDS_INPUT_PAD_KEY_UP,         "pad-3", "up"          },
			{ PAD3_KEYS + PAD_KEY_RIGHT,      NO_KEY, IDS_INPUT_PAD_KEY_RIGHT,      "pad-3", "right"       },
			{ PAD3_KEYS + PAD_KEY_DOWN,       NO_KEY, IDS_INPUT_PAD_KEY_DOWN,       "pad-3", "down"        },
			{ PAD3_KEYS + PAD_KEY_SELECT,     NO_KEY, IDS_INPUT_PAD_KEY_SELECT,     "pad-3", "select"      },
			{ PAD3_KEYS + PAD_KEY_START,      NO_KEY, IDS_INPUT_PAD_KEY_START,      "pad-3", "start"       },
			{ PAD3_KEYS + PAD_KEY_B,          NO_KEY, IDS_INPUT_PAD_KEY_B,          "pad-3", "b"           },
			{ PAD3_KEYS + PAD_KEY_A,          NO_KEY, IDS_INPUT_PAD_KEY_A,          "pad-3", "a"           },
			{ PAD3_KEYS + PAD_KEY_AUTOFIRE_B, NO_KEY, IDS_INPUT_PAD_KEY_AUTOFIRE_B, "pad-3", "auto-fire-b" },
			{ PAD3_KEYS + PAD_KEY_AUTOFIRE_A, NO_KEY, IDS_INPUT_PAD_KEY_AUTOFIRE_A, "pad-3", "auto-fire-a" },
			{ PAD3_KEYS + PAD_KEY_MIC,        NO_KEY, IDS_INPUT_PAD_KEY_MIC,        "pad-3", "mic"         },

			{ PAD4_KEYS + PAD_KEY_LEFT,       NO_KEY, IDS_INPUT_PAD_KEY_LEFT,       "pad-4", "left"        },
			{ PAD4_KEYS + PAD_KEY_UP,         NO_KEY, IDS_INPUT_PAD_KEY_UP,         "pad-4", "up"          },
			{ PAD4_KEYS + PAD_KEY_RIGHT,      NO_KEY, IDS_INPUT_PAD_KEY_RIGHT,      "pad-4", "right"       },
			{ PAD4_KEYS + PAD_KEY_DOWN,       NO_KEY, IDS_INPUT_PAD_KEY_DOWN,       "pad-4", "down"        },
			{ PAD4_KEYS + PAD_KEY_SELECT,     NO_KEY, IDS_INPUT_PAD_KEY_SELECT,     "pad-4", "select"      },
			{ PAD4_KEYS + PAD_KEY_START,      NO_KEY, IDS_INPUT_PAD_KEY_START,      "pad-4", "start"       },
			{ PAD4_KEYS + PAD_KEY_B,          NO_KEY, IDS_INPUT_PAD_KEY_B,          "pad-4", "b"           },
			{ PAD4_KEYS + PAD_KEY_A,          NO_KEY, IDS_INPUT_PAD_KEY_A,          "pad-4", "a"           },
			{ PAD4_KEYS + PAD_KEY_AUTOFIRE_B, NO_KEY, IDS_INPUT_PAD_KEY_AUTOFIRE_B, "pad-4", "auto-fire-b" },
			{ PAD4_KEYS + PAD_KEY_AUTOFIRE_A, NO_KEY, IDS_INPUT_PAD_KEY_AUTOFIRE_A, "pad-4", "auto-fire-a" },
			{ PAD4_KEYS + PAD_KEY_MIC,        NO_KEY, IDS_INPUT_PAD_KEY_MIC,        "pad-4", "mic"         },

			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_1,  DIK_Q, IDS_INPUT_POWERPAD_KEY_SIDE_A_1,  "powerpad", "side-a-1"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_2,  DIK_W, IDS_INPUT_POWERPAD_KEY_SIDE_A_2,  "powerpad", "side-a-2"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_3,  DIK_E, IDS_INPUT_POWERPAD_KEY_SIDE_A_3,  "powerpad", "side-a-3"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_4,  DIK_R, IDS_INPUT_POWERPAD_KEY_SIDE_A_4,  "powerpad", "side-a-4"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_5,  DIK_A, IDS_INPUT_POWERPAD_KEY_SIDE_A_5,  "powerpad", "side-a-5"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_6,  DIK_S, IDS_INPUT_POWERPAD_KEY_SIDE_A_6,  "powerpad", "side-a-6"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_7,  DIK_D, IDS_INPUT_POWERPAD_KEY_SIDE_A_7,  "powerpad", "side-a-7"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_8,  DIK_F, IDS_INPUT_POWERPAD_KEY_SIDE_A_8,  "powerpad", "side-a-8"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_9,  DIK_Z, IDS_INPUT_POWERPAD_KEY_SIDE_A_9,  "powerpad", "side-a-9"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_10, DIK_X, IDS_INPUT_POWERPAD_KEY_SIDE_A_10, "powerpad", "side-a-10" },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_11, DIK_C, IDS_INPUT_POWERPAD_KEY_SIDE_A_11, "powerpad", "side-a-11" },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_12, DIK_V, IDS_INPUT_POWERPAD_KEY_SIDE_A_12, "powerpad", "side-a-12" },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_3,  DIK_Y, IDS_INPUT_POWERPAD_KEY_SIDE_B_3,  "powerpad", "side-b-3"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_2,  DIK_U, IDS_INPUT_POWERPAD_KEY_SIDE_B_2,  "powerpad", "side-b-2"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_8,  DIK_G, IDS_INPUT_POWERPAD_KEY_SIDE_B_8,  "powerpad", "side-b-8"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_7,  DIK_H, IDS_INPUT_POWERPAD_KEY_SIDE_B_7,  "powerpad", "side-b-7"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_6,  DIK_J, IDS_INPUT_POWERPAD_KEY_SIDE_B_6,  "powerpad", "side-b-6"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_5,  DIK_K, IDS_INPUT_POWERPAD_KEY_SIDE_B_5,  "powerpad", "side-b-5"  },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_11, DIK_N, IDS_INPUT_POWERPAD_KEY_SIDE_B_11, "powerpad", "side-b-11" },
			{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_10, DIK_M, IDS_INPUT_POWERPAD_KEY_SIDE_B_10, "powerpad", "side-b-10" },

			{ POWERGLOVE_KEYS + POWERGLOVE_KEY_SELECT,     DIK_RSHIFT, IDS_INPUT_POWERGLOVE_KEY_SELECT,     "powerglove", "select"     },
			{ POWERGLOVE_KEYS + POWERGLOVE_KEY_START,      DIK_RETURN, IDS_INPUT_POWERGLOVE_KEY_START,      "powerglove", "start"      },
			{ POWERGLOVE_KEYS + POWERGLOVE_KEY_MOVE_IN,    DIK_A,      IDS_INPUT_POWERGLOVE_KEY_MOVE_IN,    "powerglove", "move-in"    },
			{ POWERGLOVE_KEYS + POWERGLOVE_KEY_MOVE_OUT,   DIK_Z,      IDS_INPUT_POWERGLOVE_KEY_MOVE_OUT,   "powerglove", "move-out"   },
			{ POWERGLOVE_KEYS + POWERGLOVE_KEY_ROLL_LEFT,  DIK_X,      IDS_INPUT_POWERGLOVE_KEY_ROLL_LEFT,  "powerglove", "roll-left"  },
			{ POWERGLOVE_KEYS + POWERGLOVE_KEY_ROLL_RIGHT, DIK_C,      IDS_INPUT_POWERGLOVE_KEY_ROLL_RIGHT, "powerglove", "roll-right" },

			{ HORITRACK_KEYS + HORITRACK_KEY_LEFT,        DIK_LEFT,   IDS_INPUT_HORITRACK_KEY_LEFT,        "horitrack", "left"        },
			{ HORITRACK_KEYS + HORITRACK_KEY_UP,          DIK_UP,     IDS_INPUT_HORITRACK_KEY_UP,          "horitrack", "up"          },
			{ HORITRACK_KEYS + HORITRACK_KEY_RIGHT,       DIK_RIGHT,  IDS_INPUT_HORITRACK_KEY_RIGHT,       "horitrack", "right"       },
			{ HORITRACK_KEYS + HORITRACK_KEY_DOWN,        DIK_DOWN,   IDS_INPUT_HORITRACK_KEY_DOWN,        "horitrack", "down"        },
			{ HORITRACK_KEYS + HORITRACK_KEY_SELECT,      DIK_RSHIFT, IDS_INPUT_HORITRACK_KEY_SELECT,      "horitrack", "select"      },
			{ HORITRACK_KEYS + HORITRACK_KEY_START,       DIK_RETURN, IDS_INPUT_HORITRACK_KEY_START,       "horitrack", "start"       },
			{ HORITRACK_KEYS + HORITRACK_KEY_B,           DIK_COMMA,  IDS_INPUT_HORITRACK_KEY_B,           "horitrack", "b"           },
			{ HORITRACK_KEYS + HORITRACK_KEY_A,           DIK_PERIOD, IDS_INPUT_HORITRACK_KEY_A,           "horitrack", "a"           },
			{ HORITRACK_KEYS + HORITRACK_KEY_SPEED,       DIK_S,      IDS_INPUT_HORITRACK_KEY_SPEED,       "horitrack", "speed"       },
			{ HORITRACK_KEYS + HORITRACK_KEY_ORIENTATION, DIK_D,      IDS_INPUT_HORITRACK_KEY_ORIENTATION, "horitrack", "orientation" },

			{ PACHINKO_KEYS + PACHINKO_KEY_LEFT,          DIK_LEFT,   IDS_INPUT_PACHINKO_KEY_LEFT,   "pachinko", "left"   },
			{ PACHINKO_KEYS + PACHINKO_KEY_UP,            DIK_UP,     IDS_INPUT_PACHINKO_KEY_UP,     "pachinko", "up"     },
			{ PACHINKO_KEYS + PACHINKO_KEY_RIGHT,         DIK_RIGHT,  IDS_INPUT_PACHINKO_KEY_RIGHT,  "pachinko", "right"  },
			{ PACHINKO_KEYS + PACHINKO_KEY_DOWN,          DIK_DOWN,   IDS_INPUT_PACHINKO_KEY_DOWN,   "pachinko", "down"   },
			{ PACHINKO_KEYS + PACHINKO_KEY_SELECT,        DIK_RSHIFT, IDS_INPUT_PACHINKO_KEY_SELECT, "pachinko", "select" },
			{ PACHINKO_KEYS + PACHINKO_KEY_START,         DIK_RETURN, IDS_INPUT_PACHINKO_KEY_START,  "pachinko", "start"  },
			{ PACHINKO_KEYS + PACHINKO_KEY_B,             DIK_COMMA,  IDS_INPUT_PACHINKO_KEY_B,      "pachinko", "b"      },
			{ PACHINKO_KEYS + PACHINKO_KEY_A,             DIK_PERIOD, IDS_INPUT_PACHINKO_KEY_A,      "pachinko", "a"      },

			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_UP,     DIK_W, IDS_INPUT_CRAZYCLIMBER_KEY_LEFT_UP,     "crazyclimber", "left-up"     },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_RIGHT,  DIK_D, IDS_INPUT_CRAZYCLIMBER_KEY_LEFT_RIGHT,  "crazyclimber", "left-right"  },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_DOWN,   DIK_S, IDS_INPUT_CRAZYCLIMBER_KEY_LEFT_DOWN,   "crazyclimber", "left-down"   },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_LEFT,   DIK_A, IDS_INPUT_CRAZYCLIMBER_KEY_LEFT_LEFT,   "crazyclimber", "left-left"   },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_UP,    DIK_Y, IDS_INPUT_CRAZYCLIMBER_KEY_RIGHT_UP,    "crazyclimber", "right-up"    },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_RIGHT, DIK_J, IDS_INPUT_CRAZYCLIMBER_KEY_RIGHT_RIGHT, "crazyclimber", "right-right" },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_DOWN,  DIK_H, IDS_INPUT_CRAZYCLIMBER_KEY_RIGHT_DOWN,  "crazyclimber", "right-down"  },
			{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_LEFT,  DIK_G, IDS_INPUT_CRAZYCLIMBER_KEY_RIGHT_LEFT,  "crazyclimber", "right-left"  },

			{ MAHJONG_KEYS + MAHJONG_KEY_A,      DIK_Q, IDS_INPUT_MAHJONG_KEY_A,      "mahjong", "a"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_B,      DIK_W, IDS_INPUT_MAHJONG_KEY_B,      "mahjong", "b"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_C,      DIK_E, IDS_INPUT_MAHJONG_KEY_C,      "mahjong", "c"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_D,      DIK_R, IDS_INPUT_MAHJONG_KEY_D,      "mahjong", "d"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_E,      DIK_T, IDS_INPUT_MAHJONG_KEY_E,      "mahjong", "e"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_F,      DIK_A, IDS_INPUT_MAHJONG_KEY_F,      "mahjong", "f"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_G,      DIK_S, IDS_INPUT_MAHJONG_KEY_G,      "mahjong", "g"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_H,      DIK_D, IDS_INPUT_MAHJONG_KEY_H,      "mahjong", "h"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_I,      DIK_F, IDS_INPUT_MAHJONG_KEY_I,      "mahjong", "i"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_J,      DIK_G, IDS_INPUT_MAHJONG_KEY_J,      "mahjong", "j"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_K,      DIK_H, IDS_INPUT_MAHJONG_KEY_K,      "mahjong", "k"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_L,      DIK_J, IDS_INPUT_MAHJONG_KEY_L,      "mahjong", "l"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_M,      DIK_K, IDS_INPUT_MAHJONG_KEY_M,      "mahjong", "m"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_N,      DIK_L, IDS_INPUT_MAHJONG_KEY_N,      "mahjong", "n"      },
			{ MAHJONG_KEYS + MAHJONG_KEY_START,  DIK_Z, IDS_INPUT_MAHJONG_KEY_START,  "mahjong", "start"  },
			{ MAHJONG_KEYS + MAHJONG_KEY_SELECT, DIK_X, IDS_INPUT_MAHJONG_KEY_SELECT, "mahjong", "select" },
			{ MAHJONG_KEYS + MAHJONG_KEY_KAN,    DIK_C, IDS_INPUT_MAHJONG_KEY_KAN,    "mahjong", "kan"    },
			{ MAHJONG_KEYS + MAHJONG_KEY_PON,    DIK_V, IDS_INPUT_MAHJONG_KEY_PON,    "mahjong", "pon"    },
			{ MAHJONG_KEYS + MAHJONG_KEY_CHI,    DIK_B, IDS_INPUT_MAHJONG_KEY_CHI,    "mahjong", "chi"    },
			{ MAHJONG_KEYS + MAHJONG_KEY_REACH,  DIK_N, IDS_INPUT_MAHJONG_KEY_REACH,  "mahjong", "reach"  },
			{ MAHJONG_KEYS + MAHJONG_KEY_RON,    DIK_M, IDS_INPUT_MAHJONG_KEY_RON,    "mahjong", "ron"    },

			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_LEFT_HOOK,  DIK_K,      IDS_INPUT_EXCITINGBOXING_KEY_LEFT_HOOK,  "excitingboxing", "left-hook"  },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_RIGHT_HOOK, DIK_L,      IDS_INPUT_EXCITINGBOXING_KEY_RIGHT_HOOK, "excitingboxing", "right-hook" },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_LEFT_JAB,   DIK_COMMA,  IDS_INPUT_EXCITINGBOXING_KEY_LEFT_JAB,   "excitingboxing", "left-jab"   },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_RIGHT_JAB,  DIK_PERIOD, IDS_INPUT_EXCITINGBOXING_KEY_RIGHT_JAB,  "excitingboxing", "right-jab"  },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_STRAIGHT,   DIK_UP,     IDS_INPUT_EXCITINGBOXING_KEY_STRAIGHT,   "excitingboxing", "straight"   },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_BODY,       DIK_DOWN,   IDS_INPUT_EXCITINGBOXING_KEY_BODY,       "excitingboxing", "body"       },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_LEFT_MOVE,  DIK_LEFT,   IDS_INPUT_EXCITINGBOXING_KEY_LEFT_MOVE,  "excitingboxing", "left-move"  },
			{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_RIGHT_MOVE, DIK_RIGHT,  IDS_INPUT_EXCITINGBOXING_KEY_RIGHT_MOVE, "excitingboxing", "right-move" },

			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_1, DIK_T,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_1_1, "pokkunmoguraa", "row-1-button-1" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_2, DIK_Y,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_1_2, "pokkunmoguraa", "row-1-button-2" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_3, DIK_U,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_1_3, "pokkunmoguraa", "row-1-button-3" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_4, DIK_I,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_1_4, "pokkunmoguraa", "row-1-button-4" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_1, DIK_G,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_2_1, "pokkunmoguraa", "row-2-button-1" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_2, DIK_H,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_2_2, "pokkunmoguraa", "row-2-button-2" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_3, DIK_J,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_2_3, "pokkunmoguraa", "row-2-button-3" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_4, DIK_K,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_2_4, "pokkunmoguraa", "row-2-button-4" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_1, DIK_B,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_3_1, "pokkunmoguraa", "row-3-button-1" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_2, DIK_N,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_3_2, "pokkunmoguraa", "row-3-button-2" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_3, DIK_M,     IDS_INPUT_POKKUNMOGURAA_KEY_ROW_3_3, "pokkunmoguraa", "row-3-button-3" },
			{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_4, DIK_COMMA, IDS_INPUT_POKKUNMOGURAA_KEY_ROW_3_4, "pokkunmoguraa", "row-3-button-4" },

			{ PARTYTAP_KEYS + PARTYTAP_UNIT_1, DIK_Q, IDS_INPUT_PARTYTAP_UNIT_1, "partytap", "unit-1" },
			{ PARTYTAP_KEYS + PARTYTAP_UNIT_2, DIK_W, IDS_INPUT_PARTYTAP_UNIT_2, "partytap", "unit-2" },
			{ PARTYTAP_KEYS + PARTYTAP_UNIT_3, DIK_E, IDS_INPUT_PARTYTAP_UNIT_3, "partytap", "unit-3" },
			{ PARTYTAP_KEYS + PARTYTAP_UNIT_4, DIK_R, IDS_INPUT_PARTYTAP_UNIT_4, "partytap", "unit-4" },
			{ PARTYTAP_KEYS + PARTYTAP_UNIT_5, DIK_T, IDS_INPUT_PARTYTAP_UNIT_5, "partytap", "unit-5" },
			{ PARTYTAP_KEYS + PARTYTAP_UNIT_6, DIK_Y, IDS_INPUT_PARTYTAP_UNIT_6, "partytap", "unit-6" },

			{ KARAOKESTUDIO_KEYS + KARAOKESTUDIO_MIC, DIK_G, IDS_INPUT_KARAOKESTUDIO_MIC, "karaokestudio", "mic" },
			{ KARAOKESTUDIO_KEYS + KARAOKESTUDIO_A,   DIK_H, IDS_INPUT_KARAOKESTUDIO_A,   "karaokestudio", "a"   },
			{ KARAOKESTUDIO_KEYS + KARAOKESTUDIO_B,   DIK_J, IDS_INPUT_KARAOKESTUDIO_B,   "karaokestudio", "b"   },

			{ EMULATION_KEYS + EMULATION_KEY_ALT_SPEED,     DIK_TAB,  IDS_INPUT_EMULATION_KEY_ALT_SPEED,     "emulation", "alt-speed"     },
			{ EMULATION_KEYS + EMULATION_KEY_REWIND,        DIK_BACK, IDS_INPUT_EMULATION_KEY_REWIND,        "emulation", "rewind"        },
			{ EMULATION_KEYS + EMULATION_KEY_INSERT_COIN_1, DIK_F2,   IDS_INPUT_EMULATION_KEY_INSERT_COIN_1, "emulation", "insert-coin-1" },
			{ EMULATION_KEYS + EMULATION_KEY_INSERT_COIN_2, DIK_F3,   IDS_INPUT_EMULATION_KEY_INSERT_COIN_2, "emulation", "insert-coin-2" },

			{ FILE_KEYS + FILE_KEY_OPEN,                  CTRL | 'O',         IDS_INPUT_FILE_KEY_OPEN,                  "file", "open"                    },
			{ FILE_KEYS + FILE_KEY_SAVE_STATE,            VK_F5,              IDS_INPUT_FILE_KEY_SAVE_STATE,            "file", "save-state"              },
			{ FILE_KEYS + FILE_KEY_LOAD_STATE,            VK_F7,              IDS_INPUT_FILE_KEY_LOAD_STATE,            "file", "load-state"              },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_1,    '1',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_1,    "file", "quick-load-state-1"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_2,    '2',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_2,    "file", "quick-load-state-2"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_3,    '3',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_3,    "file", "quick-load-state-3"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_4,    '4',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_4,    "file", "quick-load-state-4"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_5,    '5',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_5,    "file", "quick-load-state-5"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_6,    '6',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_6,    "file", "quick-load-state-6"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_7,    '7',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_7,    "file", "quick-load-state-7"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_8,    '8',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_8,    "file", "quick-load-state-8"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_9,    '9',                IDS_INPUT_FILE_KEY_QUICK_LOAD_STATE_9,    "file", "quick-load-state-9"      },
			{ FILE_KEYS + FILE_KEY_QUICK_LOAD_LAST_STATE, '0',                IDS_INPUT_FILE_KEY_QUICK_LOAD_LAST_STATE, "file", "quick-load-state-newest" },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_1,    SHIFT | '1',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_1,    "file", "quick-save-state-1"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_2,    SHIFT | '2',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_2,    "file", "quick-save-state-2"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_3,    SHIFT | '3',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_3,    "file", "quick-save-state-3"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_4,    SHIFT | '4',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_4,    "file", "quick-save-state-4"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_5,    SHIFT | '5',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_5,    "file", "quick-save-state-5"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_6,    SHIFT | '6',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_6,    "file", "quick-save-state-6"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_7,    SHIFT | '7',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_7,    "file", "quick-save-state-7"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_8,    SHIFT | '8',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_8,    "file", "quick-save-state-8"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_9,    SHIFT | '9',        IDS_INPUT_FILE_KEY_QUICK_SAVE_STATE_9,    "file", "quick-save-state-9"      },
			{ FILE_KEYS + FILE_KEY_QUICK_SAVE_NEXT_STATE, SHIFT | '0',        IDS_INPUT_FILE_KEY_QUICK_SAVE_NEXT_STATE, "file", "quick-save-state-oldest" },
			{ FILE_KEYS + FILE_KEY_SAVE_SCREENSHOT,       ALT | 'E',          IDS_INPUT_FILE_KEY_SAVE_SCREENSHOT,       "file", "save-screenshot"         },
			{ FILE_KEYS + FILE_KEY_LAUNCHER,              ALT | 'L',          IDS_INPUT_FILE_KEY_LAUNCHER,              "file", "launcher"                },
			{ FILE_KEYS + FILE_KEY_EXIT,                  ALT | 'X',          IDS_INPUT_FILE_KEY_EXIT,                  "file", "exit"                    },

			{ MACHINE_KEYS + MACHINE_KEY_POWER,             SHIFT | 'D', IDS_INPUT_MACHINE_KEY_POWER,             "machine", "power"                  },
			{ MACHINE_KEYS + MACHINE_KEY_RESET_SOFT,        SHIFT | 'R', IDS_INPUT_MACHINE_KEY_RESET_SOFT,        "machine", "soft-reset"             },
			{ MACHINE_KEYS + MACHINE_KEY_RESET_HARD,        SHIFT | 'T', IDS_INPUT_MACHINE_KEY_RESET_HARD,        "machine", "hard-reset"             },
			{ MACHINE_KEYS + MACHINE_KEY_PAUSE,             SHIFT | 'P', IDS_INPUT_MACHINE_KEY_PAUSE,             "machine", "pause"                  },
			{ MACHINE_KEYS + MACHINE_KEY_UNLIMITED_SPRITES, SHIFT | 'U', IDS_INPUT_MACHINE_KEY_UNLIMITED_SPRITES, "machine", "no-sprite-limit-toggle" },
			{ MACHINE_KEYS + MACHINE_KEY_CHANGE_DISK_SIDE,  SHIFT | 'B', IDS_INPUT_MACHINE_KEY_CHANGE_DISK_SIDE,  "machine", "change-disk-side"       },

			{ NSF_KEYS + NSF_KEY_PLAY, SHIFT | VK_UP,    IDS_INPUT_NSF_KEY_PLAY, "nsf", "play-song"     },
			{ NSF_KEYS + NSF_KEY_STOP, SHIFT | VK_DOWN,  IDS_INPUT_NSF_KEY_STOP, "nsf", "stop-song"     },
			{ NSF_KEYS + NSF_KEY_NEXT, SHIFT | VK_RIGHT, IDS_INPUT_NSF_KEY_NEXT, "nsf", "next-song"     },
			{ NSF_KEYS + NSF_KEY_PREV, SHIFT | VK_LEFT,  IDS_INPUT_NSF_KEY_PREV, "nsf", "previous-song" },

			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_1X,  ALT | '1',        IDS_INPUT_VIEW_KEY_SCREENSIZE_1X,  "view", "screen-size-1x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_2X,  ALT | '2',        IDS_INPUT_VIEW_KEY_SCREENSIZE_2X,  "view", "screen-size-2x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_3X,  ALT | '3',        IDS_INPUT_VIEW_KEY_SCREENSIZE_3X,  "view", "screen-size-3x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_4X,  ALT | '4',        IDS_INPUT_VIEW_KEY_SCREENSIZE_4X,  "view", "screen-size-4x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_5X,  ALT | '5',        IDS_INPUT_VIEW_KEY_SCREENSIZE_5X,  "view", "screen-size-5x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_6X,  ALT | '6',        IDS_INPUT_VIEW_KEY_SCREENSIZE_6X,  "view", "screen-size-6x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_7X,  ALT | '7',        IDS_INPUT_VIEW_KEY_SCREENSIZE_7X,  "view", "screen-size-7x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_8X,  ALT | '8',        IDS_INPUT_VIEW_KEY_SCREENSIZE_8X,  "view", "screen-size-8x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_9X,  ALT | '9',        IDS_INPUT_VIEW_KEY_SCREENSIZE_9X,  "view", "screen-size-9x"       },
			{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_MAX, ALT | 'S',        IDS_INPUT_VIEW_KEY_SCREENSIZE_MAX, "view", "screen-size-max"      },
			{ VIEW_KEYS + VIEW_KEY_SHOW_MENU,      VK_ESCAPE,        IDS_INPUT_VIEW_KEY_SHOW_MENU,      "view", "toggle-menu"          },
			{ VIEW_KEYS + VIEW_KEY_SHOW_STATUSBAR, CTRL | 'B',       IDS_INPUT_VIEW_KEY_SHOW_STATUSBAR, "view", "toggle-status-bar"    },
			{ VIEW_KEYS + VIEW_KEY_SHOW_ONTOP,     CTRL | 'T',       IDS_INPUT_VIEW_KEY_SHOW_ONTOP,     "view", "toggle-window-on-top" },
			{ VIEW_KEYS + VIEW_KEY_SHOW_FPS,       CTRL | 'F',       IDS_INPUT_VIEW_KEY_SHOW_FPS,       "view", "toggle-fps"           },
			{ VIEW_KEYS + VIEW_KEY_FULLSCREEN,     ALT  | VK_RETURN, IDS_INPUT_VIEW_KEY_FULLSCREEN,     "view", "fullscreen"           },

			{ HELP_KEYS + HELP_KEY_HELP, VK_F1, IDS_INPUT_HELP_KEY_HELP, "help", "readme" }
		};

		struct Input::Handlers
		{
			static const MsgHandler::Entry<Input> messages[];
			static const MsgHandler::Entry<Input> commands[];
		};

		const MsgHandler::Entry<Input> Input::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Input::OnInitDialog },
			{ WM_HSCROLL,    &Input::OnHScroll    },
			{ WM_DESTROY,    &Input::OnDestroy    }
		};

		const MsgHandler::Entry<Input> Input::Handlers::commands[] =
		{
			{ IDC_INPUT_MAP,                 &Input::OnCmdDblClk           },
			{ IDC_INPUT_DEVICES,             &Input::OnCmdDevice           },
			{ IDC_INPUT_JOYSTICKS,           &Input::OnCmdJoysticks        },
			{ IDC_INPUT_SET,                 &Input::OnCmdSet              },
			{ IDC_INPUT_SETALL,              &Input::OnCmdSetAll           },
			{ IDC_INPUT_CLEAR,               &Input::OnCmdClear            },
			{ IDC_INPUT_CLEARALL,            &Input::OnCmdClearAll         },
			{ IDC_INPUT_JOYSTICKS_ENABLE,    &Input::OnCmdJoystickEnable   },
			{ IDC_INPUT_JOYSTICKS_X,         &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_Y,         &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_Z,         &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_RX,        &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_RY,        &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_RZ,        &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_S0,        &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_S1,        &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_POV0,      &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_POV1,      &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_POV2,      &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_POV3,      &Input::OnCmdJoystickAxis     },
			{ IDC_INPUT_JOYSTICKS_CALIBRATE, &Input::OnCmdCalibrate        },
			{ IDC_INPUT_JOYSTICKS_DEFAULT,   &Input::OnCmdJoysticksDefault },
			{ IDC_INPUT_DEFAULT,             &Input::OnCmdDefault          },
			{ IDC_INPUT_DEFAULT_CATEGORY,    &Input::OnCmdDefaultCategory  },
			{ IDC_INPUT_AUTOFIRE_DEFAULT,    &Input::OnCmdAutoFireDefault  }
		};

		Input::Settings::Settings()
		: autoFireSpeed(AUTOFIRE_DEFAULT_SPEED) {}

		inline uint Input::Settings::Mapping::Code() const
		{
			return key & 0xFF;
		}

		inline uint Input::Settings::Mapping::Alt() const
		{
			return (key & ALT) ? VK_MENU : 0;
		}

		inline uint Input::Settings::Mapping::Shift() const
		{
			return (key & SHIFT) ? VK_SHIFT : 0;
		}

		inline uint Input::Settings::Mapping::Ctrl() const
		{
			return (key & CTRL) ? VK_CONTROL : 0;
		}

		inline const Input::Settings::Mapping& Input::Settings::GetMapping(uint type,uint index)
		{
			NST_ASSERT( index < NumTypeKeys(type) );
			return map[types[type].offset + index];
		}

		inline const Input::Settings::Type& Input::Settings::GetType(uint type)
		{
			NST_ASSERT( type < NUM_TYPES );
			return types[type];
		}

		inline void Input::Settings::Unmap(uint index)
		{
			NST_ASSERT( index < NUM_KEYS );
			keys[index].Unmap();
		}

		bool Input::Settings::Map(const uint index,const Key& key)
		{
			NST_ASSERT( index < NUM_KEYS );

			if (key.Assigned())
			{
				if (index >= COMMAND_KEYS)
				{
					for (uint i=0; i < COMMAND_KEYS; ++i)
					{
						if (key == keys[i])
							return (i == index);
					}
				}

				for (uint i=COMMAND_KEYS; i < NUM_KEYS; ++i)
				{
					if (key == keys[i])
						return (i == index);
				}
			}

			keys[index] = key;

			return true;
		}

		void Input::Settings::Clear()
		{
			for (uint i=0; i < NUM_KEYS; ++i)
				keys[i].Unmap();
		}

		void Input::Settings::Clear(const uint type)
		{
			NST_ASSERT( type < NUM_TYPES );

			for (uint i=types[type].offset, n=types[type+1].offset; i < n; ++i)
				keys[i].Unmap();
		}

		void Input::Settings::Reset(const DirectX::DirectInput& directInput)
		{
			for (uint i=0; i < NUM_TYPES; ++i)
				Reset( directInput, i );
		}

		void Input::Settings::Reset(const DirectX::DirectInput& directInput,const uint type)
		{
			NST_ASSERT( type < NUM_TYPES );

			const Mapping* it = map + types[type].offset;
			const Mapping* const end = map + types[type + 1].offset;

			NST_ASSERT( it < end );

			Key key;

			if (type < TYPE_FILE)
			{
				do
				{
					directInput.MapKeyboard( key, it->key );
					Map( it->index, key );
				}
				while (++it != end);
			}
			else
			{
				do
				{
					key.MapVirtualKey( it->Code(), it->Alt(), it->Ctrl(), it->Shift() );
					Map( it->index, key );
				}
				while (++it != end);
			}
		}

		Input::Input(DirectX::DirectInput& di,Managers::Emulator& emulator,const Configuration& cfg)
		:
		nes         ( emulator ),
		directInput ( di ),
		dialog      ( IDD_INPUT, this, Handlers::messages, Handlers::commands )
		{
			Configuration::ConstSection input( cfg["input"] );

			settings.Clear();
			settings.autoFireSpeed = input["auto-fire-speed"].Int( Settings::AUTOFIRE_DEFAULT_SPEED );
			settings.allowSimulAxes = input["allow-simultaneous-axes"].Yes();

			if (settings.autoFireSpeed > Settings::AUTOFIRE_MAX_SPEED)
				settings.autoFireSpeed = Settings::AUTOFIRE_DEFAULT_SPEED;

			System::Guid joyGuids[DirectX::DirectInput::MAX_JOYSTICKS];
			uint maxGuids = 0;

			if (const uint numJoysticks = directInput.NumJoysticks())
			{
				bool referenced[DirectX::DirectInput::MAX_JOYSTICKS] = {false};

				if (Configuration::ConstSection joysticks=input["joysticks"])
				{
					bool alert = true;

					for (uint i=0; i < numJoysticks; ++i)
					{
						Configuration::ConstSection joystick( joysticks["joystick"][i] );

						if (!joystick)
							break;

						uint index = UINT_MAX;

						const System::Guid guid( joystick["device"].Str() );

						for (uint j=0; j < numJoysticks; ++j)
						{
							if (guid == directInput.GetJoystickGuid(j))
							{
								index = j;
								referenced[j] = true;
								break;
							}
						}

						if (index == UINT_MAX)
						{
							for (uint j=0; j < numJoysticks; ++j)
							{
								if (!referenced[j])
								{
									index = j;
									referenced[j] = true;
									break;
								}
							}

							if (index == UINT_MAX || (alert && !Window::User::Confirm( IDS_INPUT_JOY_GUID_MISMATCH )))
								break;

							alert = false;
						}

						joyGuids[maxGuids++] = directInput.GetJoystickGuid(index);

						directInput.ScanEnableJoystick( index, !joystick["enabled"].No() );

						uint deadzone = joystick["deadzone"].Int( DirectX::DirectInput::DEFAULT_DEADZONE );

						if (deadzone > DirectX::DirectInput::DEADZONE_MAX)
							deadzone = DirectX::DirectInput::DEFAULT_DEADZONE;

						directInput.SetAxisDeadZone( index, deadzone );

						if (Configuration::ConstSection scan=joystick["scan"])
						{
							uint axes = 0;

							static cstring const names[] =
							{
								"x","y","z","rx","ry","rz","z0","z1","p0","p1","p2","p3"
							};

							for (uint j=0; j < sizeof(array(names)); ++j)
							{
								if (scan[names[j]].Yes())
								{
									axes |= (1U << j);
								}
								else if (!scan[names[j]].No())
								{
									axes |= (1U << j) & DirectX::DirectInput::DEFAULT_AXES;
								}
							}

							directInput.SetScannerAxes( index, axes );
						}
					}
				}
			}

			Configuration::ConstSection keys( input["keys"] );
			HeapString keyName;

			for (uint i=0; i < Settings::NUM_TYPES; ++i)
			{
				for (uint j=0, n=Settings::NumTypeKeys(i); j < n; ++j)
				{
					const Settings::Mapping& mapping = Settings::GetMapping( i, j );
					keyName = keys[mapping.cfgType][mapping.cfgKey].Str();

					Settings::Key key;

					if (keyName.Length())
					{
						if (i < Settings::TYPE_COMMAND || !key.MapVirtualKey( keyName ))
							directInput.MapKey( key, keyName.Ptr(), joyGuids, maxGuids );
					}
					else
					{
						if (i < Settings::TYPE_COMMAND)
							directInput.MapKeyboard( key, mapping.key );
						else
							key.MapVirtualKey( mapping.Code(), mapping.Alt(), mapping.Ctrl(), mapping.Shift() );
					}

					if (!settings.Map( mapping.index, key ))
					{
						Io::Log() << "DirectInput: warning, key assigned to \""
                                  << mapping.cfgType
                                  << ':'
                                  << mapping.cfgKey
                                  << "\" is already in use!\r\n";
					}
				}
			}
		}

		void Input::Save(Configuration& cfg) const
		{
			Configuration::Section input( cfg["input"] );

			input["auto-fire-speed"].Int() = settings.autoFireSpeed;
			input["allow-simultaneous-axes"].YesNo() = settings.allowSimulAxes;

			if (const uint numJoysticks=directInput.NumJoysticks())
			{
				Configuration::Section joysticks( input["joysticks"] );

				for (uint i=0; i < numJoysticks; ++i)
				{
					Configuration::Section joystick( joysticks["joystick"][i] );

					joystick["device"].Str() = directInput.GetJoystickGuid(i).GetString();
					joystick["enabled"].YesNo() = directInput.JoystickScanEnabled(i);
					joystick["deadzone"].Int() = directInput.GetAxisDeadZone(i);

					static cstring const names[] =
					{
						"x","y","z","rx","ry","rz","z0","z1","p0","p1","p2","p3"
					};

					Configuration::Section scan( joystick["scan"] );

					for (uint j=0, axes=directInput.GetScannerAxes(i); j < sizeof(array(names)); ++j)
						scan[names[j]].YesNo() = (axes & (1U << j));
				}
			}

			Configuration::Section keys( input["keys"] );

			for (uint i=0; i < Settings::NUM_TYPES; ++i)
			{
				for (uint j=0, n=Settings::NumTypeKeys(i); j < n; ++j)
				{
					const Settings::Mapping& mapping = Settings::GetMapping( i, j );
					keys[mapping.cfgType][mapping.cfgKey].Str() = directInput.GetKeyName( settings.GetKey(mapping.index) );
				}
			}
		}

		ibool Input::OnInitDialog(Param&)
		{
			{
				const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_DEVICES) );

				listBox.Reserve( Settings::NUM_TYPES );

				for (uint i=0; i < Settings::NUM_TYPES; ++i)
					listBox.Add( Resource::String(Settings::GetType(i).name) );

				listBox[0].Select();
			}

			if (const uint numJoysticks = directInput.NumJoysticks())
			{
				const Control::ComboBox comboBox( dialog.ComboBox(IDC_INPUT_JOYSTICKS) );

				for (uint i=0; i < numJoysticks; ++i)
					comboBox.Add( directInput.GetJoystickName(i).Ptr() );

				comboBox[0].Select();

				dialog.Slider(IDC_INPUT_JOYSTICKS_DEADZONE).SetRange( 0, DirectX::DirectInput::DEADZONE_MAX );

				UpdateJoysticks( 0 );
			}
			else for (uint i=IDC_INPUT_JOYSTICKS; i <= IDC_INPUT_JOYSTICKS_DEFAULT; ++i)
			{
				dialog.Control( i ).Disable();
			}

			dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).SetRange( 0, Settings::AUTOFIRE_MAX_SPEED );
			dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position() = settings.autoFireSpeed;

			dialog.CheckBox( IDC_INPUT_ALLOW_SIMUL_AXES ).Check( settings.allowSimulAxes );

			UpdateKeyNames( 0 );
			UpdateKeyMap( 0 );

			return true;
		}

		ibool Input::OnCmdDblClk(Param& param)
		{
			if (HIWORD(param.wParam) == LBN_DBLCLK)
			{
				if (ScanKeys() == SCAN_NEXT)
					SelectNextMapKey();

				return true;
			}

			return false;
		}

		ibool Input::OnHScroll(Param& param)
		{
			if (param.Slider().GetId() == IDC_INPUT_JOYSTICKS_DEADZONE)
			{
				const uint deadZone = param.Slider().Scroll();

				if (directInput.SetAxisDeadZone( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex(), deadZone ))
					dialog.Edit( IDC_INPUT_JOYSTICKS_DEADZONE_NUM ) << deadZone;
			}

			return true;
		}

		void Input::UpdateKeyNames(const uint type) const
		{
			NST_ASSERT( type < Settings::NUM_TYPES );

			const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_KEYS) );

			listBox.Clear();
			listBox.Reserve( Settings::NumTypeKeys(type) );

			for (uint i=0, n=Settings::NumTypeKeys(type); i < n; ++i)
				listBox.Add( Resource::String(Settings::GetMapping(type,i).dlgName) );
		}

		void Input::UpdateKeyMap(const uint type) const
		{
			const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_MAP) );

			listBox.Clear();
			listBox.Reserve( Settings::NumTypeKeys(type) );

			for (uint i=0, n=Settings::NumTypeKeys(type); i < n; ++i)
				listBox.Add( directInput.GetKeyName( settings.GetKey(Settings::GetMapping(type,i).index) ).Ptr() );

			listBox[0].Select();
		}

		void Input::ResetJoysticks()
		{
			if (uint numJoysticks = directInput.NumJoysticks())
			{
				do
				{
					directInput.ScanEnableJoystick( --numJoysticks, true );
					directInput.SetAxisDeadZone( numJoysticks, DirectX::DirectInput::DEFAULT_DEADZONE );
					directInput.SetScannerAxes( numJoysticks, DirectX::DirectInput::DEFAULT_AXES );
				}
				while (numJoysticks);

				UpdateJoysticks( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex() );
			}
		}

		void Input::UpdateJoysticks(const uint type) const
		{
			NST_ASSERT( directInput.NumJoysticks() );

			const bool enabled = directInput.JoystickScanEnabled( type );
			dialog.CheckBox(IDC_INPUT_JOYSTICKS_ENABLE).Check( enabled );

			{
				const uint deadZone = directInput.GetAxisDeadZone( type );

				dialog.Slider( IDC_INPUT_JOYSTICKS_DEADZONE ).Position() = deadZone;
				dialog.Slider( IDC_INPUT_JOYSTICKS_DEADZONE ).Enable( enabled );
				dialog.Slider( IDC_INPUT_JOYSTICKS_DEADZONE_TEXT ).Enable( enabled );

				dialog.Edit( IDC_INPUT_JOYSTICKS_DEADZONE_NUM ) << deadZone;
				dialog.Edit( IDC_INPUT_JOYSTICKS_DEADZONE_NUM ).Enable( enabled );
			}

			{
				const uint axes[] =
				{
					directInput.GetAvailableAxes( type ),
					directInput.GetScannerAxes( type )
				};

				for (uint i=0; i <= (IDC_INPUT_JOYSTICKS_POV3-IDC_INPUT_JOYSTICKS_X); ++i)
				{
					const Control::CheckBox box( dialog.CheckBox(IDC_INPUT_JOYSTICKS_X + i) );
					box.Enable( enabled && (axes[0] & (1U << i)) );
					box.Check( axes[1] & (1U << i) );
				}
			}
		}

		ibool Input::OnCmdDevice(Param& param)
		{
			if (param.ListBox().SelectionChanged())
			{
				const uint type = dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex();

				UpdateKeyNames( type );
				UpdateKeyMap( type );
			}

			return true;
		}

		ibool Input::OnCmdJoysticks(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
				UpdateJoysticks( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex() );

			return true;
		}

		ibool Input::OnCmdJoystickEnable(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint type = dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex();

				directInput.ScanEnableJoystick( type, dialog.CheckBox(param.Button().GetId()).Checked() );
				UpdateJoysticks( type  );
			}

			return true;
		}

		ibool Input::OnCmdJoystickAxis(Param& param)
		{
			if (param.Button().Clicked())
			{
				directInput.SetScannerAxes
				(
					dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex(),
					1U << (param.Button().GetId() - IDC_INPUT_JOYSTICKS_X),
					dialog.CheckBox(param.Button().GetId()).Checked()
				);
			}

			return true;
		}

		ibool Input::OnCmdSet(Param& param)
		{
			if (param.Button().Clicked() && ScanKeys() == SCAN_NEXT)
				SelectNextMapKey();

			return true;
		}

		ibool Input::OnCmdSetAll(Param& param)
		{
			if (param.Button().Clicked())
			{
				const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_MAP) );
				listBox[0].Select();

				for (uint count=listBox.Size(); count; --count)
				{
					if (ScanKeys() == SCAN_ABORT)
						break;

					SelectNextMapKey();
				}
			}

			return true;
		}

		ibool Input::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
			{
				const Control::ListBox keyBox( dialog.ListBox(IDC_INPUT_MAP) );
				const uint index = Settings::GetMapping( dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex(), keyBox.Selection().GetIndex() ).index;

				settings.Unmap( index );

				keyBox.Selection().Text() << directInput.GetKeyName( settings.GetKey(index) ).Ptr();
				SelectNextMapKey();
			}

			return true;
		}

		ibool Input::OnCmdClearAll(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint type = dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex();

				settings.Clear( type );

				UpdateKeyMap( type );
			}

			return true;
		}

		ibool Input::OnCmdDefaultCategory(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint type = dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex();

				settings.Clear( type );
				settings.Reset( directInput, type );

				UpdateKeyMap( type );
			}

			return true;
		}

		ibool Input::OnCmdAutoFireDefault(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position() = Settings::AUTOFIRE_DEFAULT_SPEED;

			return true;
		}

		ibool Input::OnCmdCalibrate(Param& param)
		{
			if (param.Button().Clicked())
				directInput.Calibrate( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex() );

			return true;
		}

		ibool Input::OnCmdJoysticksDefault(Param& param)
		{
			if (param.Button().Clicked())
				ResetJoysticks();

			return true;
		}

		ibool Input::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.Clear();
				settings.Reset( directInput );

				dialog.CheckBox( IDC_INPUT_ALLOW_SIMUL_AXES ).Uncheck();
				dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position() = Settings::AUTOFIRE_DEFAULT_SPEED;

				ResetJoysticks();

				UpdateKeyMap( dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() );
			}

			return true;
		}

		ibool Input::OnDestroy(Param&)
		{
			settings.autoFireSpeed = dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position();
			settings.allowSimulAxes = dialog.CheckBox( IDC_INPUT_ALLOW_SIMUL_AXES ).Checked();

			return true;
		}

		class Input::KeyPressWindow : Dynamic
		{
		public:

			enum Result
			{
				RESULT_OK,
				RESULT_ABORT,
				RESULT_DUPLICATE,
				RESULT_INVALID
			};

		private:

			struct Timer
			{
				enum
				{
					RATE = 50,
					CLOCK = 1000,
					START = 5000,
					SEC   = 1000
				};

				int remaining;
				int clock;

				Timer()
				: remaining(START), clock(START) {}
			};

			Input& base;
			Timer timer;
			Result result;

			void Close(Result r)
			{
				result = r;
				base.dialog.Enable( true );
				Destroy();
			}

			uint OnTimer()
			{
				timer.remaining -= Timer::RATE;

				if (timer.remaining <= 0 || *this != ::GetForegroundWindow())
				{
					Close( RESULT_ABORT );
					return false;
				}

				if (timer.remaining <= timer.clock)
				{
					uint msgId;

					if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() >= Settings::TYPE_COMMAND)
						msgId = IDS_DIALOG_INPUT_PRESS_ANY_KEY_MENU;
					else
						msgId = IDS_DIALOG_INPUT_PRESS_ANY_KEY_EMU;

					base.dialog.Control(IDC_INPUT_KEYPRESS_TEXT).Text() << Resource::String(msgId).Invoke( wchar_t('0' + (timer.clock / Timer::SEC)) );

					timer.clock -= Timer::CLOCK;
				}

				DirectX::DirectInput::ScanMode scanMode;

				if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() < Settings::TYPE_COMMAND)
					scanMode = DirectX::DirectInput::SCAN_MODE_ALL;
				else
					scanMode = DirectX::DirectInput::SCAN_MODE_JOY;

				Settings::Key key;

				switch (base.directInput.ScanKey( key, scanMode ))
				{
					case DirectX::DirectInput::SCAN_GOOD_KEY:

						for (uint i=10; i && base.directInput.AnyPressed(); --i)
							::Sleep( 100 );

						if (base.MapSelectedKey( key ))
						{
							Close( RESULT_OK );
							return false;
						}
						else
						{
							Close( RESULT_DUPLICATE );
							return false;
						}
						break;

					case DirectX::DirectInput::SCAN_INVALID_KEY:

						Close( RESULT_INVALID );
						return false;
				}

				return true;
			}

			ibool OnCreate(Param& param)
			{
				base.dialog.Enable( false );
				base.directInput.BeginScanMode( param.hWnd );

				StartTimer( this, &KeyPressWindow::OnTimer, Timer::RATE );
				::SetWindowPos( param.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW );

				return false;
			}

			ibool OnDestroy(Param&)
			{
				StopTimer( this, &KeyPressWindow::OnTimer );

				base.directInput.EndScanMode();
				base.dialog.Control(IDC_INPUT_KEYPRESS_TEXT).Text().Clear();

				return false;
			}

			ibool OnKeyDown(Param& param)
			{
				if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() >= Settings::TYPE_COMMAND)
				{
					if (param.wParam != VK_SHIFT && param.wParam != VK_CONTROL && param.wParam != VK_MENU)
					{
						const uint vKeys[3] =
						{
							(::GetAsyncKeyState( VK_SHIFT   ) & 0x8000U) ? VK_SHIFT   : 0,
							(::GetAsyncKeyState( VK_CONTROL ) & 0x8000U) ? VK_CONTROL : 0,
							(::GetAsyncKeyState( VK_MENU    ) & 0x8000U) ? VK_MENU    : 0
						};

						Settings::Key key;

						if (!key.MapVirtualKey( param.wParam, vKeys[0], vKeys[1], vKeys[2] ))
						{
							Close( RESULT_INVALID );
						}
						else if (!base.MapSelectedKey( key ))
						{
							Close( RESULT_DUPLICATE );
						}
						else
						{
							Close( RESULT_OK );
						}
					}
				}

				return false;
			}

		public:

			explicit KeyPressWindow(Input& b)
			: base(b), result(RESULT_ABORT)
			{
				static const MsgHandler::Entry<KeyPressWindow> messages[] =
				{
					{ WM_CREATE,     &KeyPressWindow::OnCreate  },
					{ WM_DESTROY,    &KeyPressWindow::OnDestroy },
					{ WM_KEYDOWN,    &KeyPressWindow::OnKeyDown },
					{ WM_SYSKEYDOWN, &KeyPressWindow::OnKeyDown }
				};

				Messages().Set( this, messages );

				Context context;

				context.className = L"Poll Key";
				context.windowName = context.className;
				context.winStyle = WS_POPUP;
				context.hParent = base.dialog;

				Create( context );
			}

			Result Poll() const
			{
				for (MSG msg; *this && ::GetMessage( &msg, NULL, 0, 0 ) > 0; )
				{
					::TranslateMessage( &msg );
					::DispatchMessage( &msg );
				}

				return result;
			}
		};

		INT_PTR Input::ScanKeys()
		{
			switch (KeyPressWindow(*this).Poll())
			{
				case KeyPressWindow::RESULT_INVALID:

					User::Warn( IDS_DIALOG_INPUT_PRESS_ANY_KEY_INVALID, IDS_TITLE_ERROR );
					return SCAN_ABORT;

				case KeyPressWindow::RESULT_DUPLICATE:

					User::Warn( IDS_DIALOG_INPUT_DUPLICATE_KEYS, IDS_TITLE_ERROR );
					return SCAN_ABORT;

				case KeyPressWindow::RESULT_ABORT:
					return SCAN_ABORT;
			}

			return SCAN_NEXT;
		}

		void Input::SelectNextMapKey()
		{
			const Control::ListBox keyBox( dialog.ListBox(IDC_INPUT_MAP) );

			const uint index = keyBox.Selection().GetIndex() + 1;
			keyBox[index < keyBox.Size() ? index : 0].Select();
		}

		bool Input::MapSelectedKey(const Settings::Key& key)
		{
			const uint index = Settings::GetMapping( dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex(), dialog.ListBox(IDC_INPUT_MAP).Selection().GetIndex() ).index;

			if (settings.Map( index, key ))
			{
				dialog.ListBox(IDC_INPUT_MAP).Selection().Text() << directInput.GetKeyName( settings.GetKey(index) ).Ptr();
				return true;
			}

			return false;
		}
	}
}
