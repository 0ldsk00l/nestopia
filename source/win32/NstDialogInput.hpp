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

#ifndef NST_DIALOG_INPUT_H
#define NST_DIALOG_INPUT_H

#pragma once

#include "NstWindowDialog.hpp"
#include "NstDirectInput.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Input
		{
		public:

			Input(DirectX::DirectInput&,Managers::Emulator&,const Configuration&);

			void Save(Configuration&) const;

		private:

			struct Handlers;
			class KeyPressWindow;

			enum
			{
				SCAN_ABORT,
				SCAN_NEXT
			};

			void ResetJoysticks();
			void ResetKeys();
			int  ScanKeys();

			ibool OnInitDialog          (Param&);
			ibool OnDestroy             (Param&);
			ibool OnHScroll             (Param&);
			ibool OnCmdDblClk           (Param&);
			ibool OnCmdDevice           (Param&);
			ibool OnCmdSet              (Param&);
			ibool OnCmdSetAll           (Param&);
			ibool OnCmdClear            (Param&);
			ibool OnCmdClearAll         (Param&);
			ibool OnCmdJoystickAxis     (Param&);
			ibool OnCmdDefaultCategory  (Param&);
			ibool OnCmdAutoFireDefault  (Param&);
			ibool OnCmdJoysticks        (Param&);
			ibool OnCmdJoystickEnable   (Param&);
			ibool OnCmdCalibrate        (Param&);
			ibool OnCmdJoysticksDefault (Param&);
			ibool OnCmdDefault          (Param&);

			void UpdateKeyMap    (uint) const;
			void UpdateKeyNames  (uint) const;
			void UpdateJoysticks (uint) const;

		public:

			class Settings
			{
				friend class Input;

			public:

				enum
				{
					TYPE_PAD1,
					TYPE_PAD2,
					TYPE_PAD3,
					TYPE_PAD4,
					TYPE_POWERPAD,
					TYPE_POWERGLOVE,
					TYPE_HORITRACK,
					TYPE_PACHINKO,
					TYPE_CRAZYCLIMBER,
					TYPE_MAHJONG,
					TYPE_EXCITINGBOXING,
					TYPE_POKKUNMOGURAA,
					TYPE_PARTYTAP,
					TYPE_KARAOKESTUDIO,
					TYPE_EMULATION,
					TYPE_FILE,
					TYPE_MACHINE,
					TYPE_NSF,
					TYPE_VIEW,
					TYPE_HELP,
					NUM_TYPES,
					TYPE_COMMAND = TYPE_FILE
				};

				enum
				{
					PAD_KEYS = 0,
					PAD_KEY_A = 0,
					PAD_KEY_B,
					PAD_KEY_SELECT,
					PAD_KEY_START,
					PAD_KEY_UP,
					PAD_KEY_DOWN,
					PAD_KEY_LEFT,
					PAD_KEY_RIGHT,
					PAD_KEY_AUTOFIRE_A,
					PAD_KEY_AUTOFIRE_B,
					PAD_KEY_MIC,
					PAD_NUM_KEYS,

					PAD1_KEYS = PAD_KEYS,
					PAD2_KEYS = PAD1_KEYS + PAD_NUM_KEYS,
					PAD3_KEYS = PAD2_KEYS + PAD_NUM_KEYS,
					PAD4_KEYS = PAD3_KEYS + PAD_NUM_KEYS,

					POWERPAD_KEYS = PAD4_KEYS + PAD_NUM_KEYS,
					POWERPAD_KEY_SIDE_A_1 = 0,
					POWERPAD_KEY_SIDE_A_2,
					POWERPAD_KEY_SIDE_A_3,
					POWERPAD_KEY_SIDE_A_4,
					POWERPAD_KEY_SIDE_A_5,
					POWERPAD_KEY_SIDE_A_6,
					POWERPAD_KEY_SIDE_A_7,
					POWERPAD_KEY_SIDE_A_8,
					POWERPAD_KEY_SIDE_A_9,
					POWERPAD_KEY_SIDE_A_10,
					POWERPAD_KEY_SIDE_A_11,
					POWERPAD_KEY_SIDE_A_12,
					POWERPAD_KEY_SIDE_B_3,
					POWERPAD_KEY_SIDE_B_2,
					POWERPAD_KEY_SIDE_B_8,
					POWERPAD_KEY_SIDE_B_7,
					POWERPAD_KEY_SIDE_B_6,
					POWERPAD_KEY_SIDE_B_5,
					POWERPAD_KEY_SIDE_B_11,
					POWERPAD_KEY_SIDE_B_10,
					POWERPAD_NUM_SIDE_A_KEYS = Nes::Input::Controllers::PowerPad::NUM_SIDE_A_BUTTONS,
					POWERPAD_NUM_SIDE_B_KEYS = Nes::Input::Controllers::PowerPad::NUM_SIDE_B_BUTTONS,
					POWERPAD_NUM_KEYS = POWERPAD_NUM_SIDE_A_KEYS + POWERPAD_NUM_SIDE_B_KEYS,

					POWERGLOVE_KEYS = POWERPAD_KEYS + POWERPAD_NUM_KEYS,
					POWERGLOVE_KEY_SELECT = 0,
					POWERGLOVE_KEY_START,
					POWERGLOVE_KEY_MOVE_IN,
					POWERGLOVE_KEY_MOVE_OUT,
					POWERGLOVE_KEY_ROLL_LEFT,
					POWERGLOVE_KEY_ROLL_RIGHT,
					POWERGLOVE_NUM_KEYS,

					HORITRACK_KEYS = POWERGLOVE_KEYS + POWERGLOVE_NUM_KEYS,
					HORITRACK_KEY_A = 0,
					HORITRACK_KEY_B,
					HORITRACK_KEY_SELECT,
					HORITRACK_KEY_START,
					HORITRACK_KEY_UP,
					HORITRACK_KEY_DOWN,
					HORITRACK_KEY_LEFT,
					HORITRACK_KEY_RIGHT,
					HORITRACK_KEY_SPEED,
					HORITRACK_KEY_ORIENTATION,
					HORITRACK_NUM_KEYS,

					PACHINKO_KEYS = HORITRACK_KEYS + HORITRACK_NUM_KEYS,
					PACHINKO_KEY_A = 0,
					PACHINKO_KEY_B,
					PACHINKO_KEY_SELECT,
					PACHINKO_KEY_START,
					PACHINKO_KEY_UP,
					PACHINKO_KEY_DOWN,
					PACHINKO_KEY_LEFT,
					PACHINKO_KEY_RIGHT,
					PACHINKO_NUM_KEYS,

					CRAZYCLIMBER_KEYS = PACHINKO_KEYS + PACHINKO_NUM_KEYS,
					CRAZYCLIMBER_KEY_LEFT_UP = 0,
					CRAZYCLIMBER_KEY_LEFT_RIGHT,
					CRAZYCLIMBER_KEY_LEFT_DOWN,
					CRAZYCLIMBER_KEY_LEFT_LEFT,
					CRAZYCLIMBER_KEY_RIGHT_UP,
					CRAZYCLIMBER_KEY_RIGHT_RIGHT,
					CRAZYCLIMBER_KEY_RIGHT_DOWN,
					CRAZYCLIMBER_KEY_RIGHT_LEFT,
					CRAZYCLIMBER_NUM_KEYS,

					MAHJONG_KEYS = CRAZYCLIMBER_KEYS + CRAZYCLIMBER_NUM_KEYS,
					MAHJONG_KEY_A = 0,
					MAHJONG_KEY_B,
					MAHJONG_KEY_C,
					MAHJONG_KEY_D,
					MAHJONG_KEY_E,
					MAHJONG_KEY_F,
					MAHJONG_KEY_G,
					MAHJONG_KEY_H,
					MAHJONG_KEY_I,
					MAHJONG_KEY_J,
					MAHJONG_KEY_K,
					MAHJONG_KEY_L,
					MAHJONG_KEY_M,
					MAHJONG_KEY_N,
					MAHJONG_KEY_START,
					MAHJONG_KEY_SELECT,
					MAHJONG_KEY_KAN,
					MAHJONG_KEY_PON,
					MAHJONG_KEY_CHI,
					MAHJONG_KEY_REACH,
					MAHJONG_KEY_RON,
					MAHJONG_NUM_KEYS,

					EXCITINGBOXING_KEYS = MAHJONG_KEYS + MAHJONG_NUM_KEYS,
					EXCITINGBOXING_KEY_LEFT_HOOK = 0,
					EXCITINGBOXING_KEY_RIGHT_HOOK,
					EXCITINGBOXING_KEY_LEFT_JAB,
					EXCITINGBOXING_KEY_RIGHT_JAB,
					EXCITINGBOXING_KEY_STRAIGHT,
					EXCITINGBOXING_KEY_BODY,
					EXCITINGBOXING_KEY_LEFT_MOVE,
					EXCITINGBOXING_KEY_RIGHT_MOVE,
					EXCITINGBOXING_NUM_KEYS,

					POKKUNMOGURAA_KEYS = EXCITINGBOXING_KEYS + EXCITINGBOXING_NUM_KEYS,
					POKKUNMOGURAA_KEY_ROW_1_1 = 0,
					POKKUNMOGURAA_KEY_ROW_1_2,
					POKKUNMOGURAA_KEY_ROW_1_3,
					POKKUNMOGURAA_KEY_ROW_1_4,
					POKKUNMOGURAA_KEY_ROW_2_1,
					POKKUNMOGURAA_KEY_ROW_2_2,
					POKKUNMOGURAA_KEY_ROW_2_3,
					POKKUNMOGURAA_KEY_ROW_2_4,
					POKKUNMOGURAA_KEY_ROW_3_1,
					POKKUNMOGURAA_KEY_ROW_3_2,
					POKKUNMOGURAA_KEY_ROW_3_3,
					POKKUNMOGURAA_KEY_ROW_3_4,
					POKKUNMOGURAA_NUM_KEYS,

					PARTYTAP_KEYS = POKKUNMOGURAA_KEYS + POKKUNMOGURAA_NUM_KEYS,
					PARTYTAP_UNIT_1 = 0,
					PARTYTAP_UNIT_2,
					PARTYTAP_UNIT_3,
					PARTYTAP_UNIT_4,
					PARTYTAP_UNIT_5,
					PARTYTAP_UNIT_6,
					PARTYTAP_NUM_KEYS,

					KARAOKESTUDIO_KEYS = PARTYTAP_KEYS + PARTYTAP_NUM_KEYS,
					KARAOKESTUDIO_MIC = 0,
					KARAOKESTUDIO_A,
					KARAOKESTUDIO_B,
					KARAOKESTUDIO_NUM_KEYS,

					EMULATION_KEYS = KARAOKESTUDIO_KEYS + KARAOKESTUDIO_NUM_KEYS,
					EMULATION_KEY_INSERT_COIN_1 = 0,
					EMULATION_KEY_INSERT_COIN_2,
					EMULATION_KEY_ALT_SPEED,
					EMULATION_KEY_REWIND,
					EMULATION_NUM_KEYS,

					POLLING_KEYS = EMULATION_KEYS + EMULATION_KEY_ALT_SPEED,
					COMMAND_KEYS = EMULATION_KEYS + EMULATION_NUM_KEYS,

					FILE_KEYS = COMMAND_KEYS,
					FILE_KEY_OPEN = 0,
					FILE_KEY_SAVE_STATE,
					FILE_KEY_LOAD_STATE,
					FILE_KEY_QUICK_LOAD_STATE_1,
					FILE_KEY_QUICK_LOAD_STATE_2,
					FILE_KEY_QUICK_LOAD_STATE_3,
					FILE_KEY_QUICK_LOAD_STATE_4,
					FILE_KEY_QUICK_LOAD_STATE_5,
					FILE_KEY_QUICK_LOAD_STATE_6,
					FILE_KEY_QUICK_LOAD_STATE_7,
					FILE_KEY_QUICK_LOAD_STATE_8,
					FILE_KEY_QUICK_LOAD_STATE_9,
					FILE_KEY_QUICK_LOAD_LAST_STATE,
					FILE_KEY_QUICK_SAVE_STATE_1,
					FILE_KEY_QUICK_SAVE_STATE_2,
					FILE_KEY_QUICK_SAVE_STATE_3,
					FILE_KEY_QUICK_SAVE_STATE_4,
					FILE_KEY_QUICK_SAVE_STATE_5,
					FILE_KEY_QUICK_SAVE_STATE_6,
					FILE_KEY_QUICK_SAVE_STATE_7,
					FILE_KEY_QUICK_SAVE_STATE_8,
					FILE_KEY_QUICK_SAVE_STATE_9,
					FILE_KEY_QUICK_SAVE_NEXT_STATE,
					FILE_KEY_SAVE_SCREENSHOT,
					FILE_KEY_LAUNCHER,
					FILE_KEY_EXIT,
					FILE_NUM_KEYS,

					MACHINE_KEYS = FILE_KEYS + FILE_NUM_KEYS,
					MACHINE_KEY_POWER = 0,
					MACHINE_KEY_RESET_SOFT,
					MACHINE_KEY_RESET_HARD,
					MACHINE_KEY_PAUSE,
					MACHINE_KEY_UNLIMITED_SPRITES,
					MACHINE_KEY_CHANGE_DISK_SIDE,
					MACHINE_NUM_KEYS,

					NSF_KEYS = MACHINE_KEYS + MACHINE_NUM_KEYS,
					NSF_KEY_PLAY = 0,
					NSF_KEY_STOP,
					NSF_KEY_NEXT,
					NSF_KEY_PREV,
					NSF_NUM_KEYS,

					VIEW_KEYS = NSF_KEYS + NSF_NUM_KEYS,
					VIEW_KEY_SCREENSIZE_1X = 0,
					VIEW_KEY_SCREENSIZE_2X,
					VIEW_KEY_SCREENSIZE_3X,
					VIEW_KEY_SCREENSIZE_4X,
					VIEW_KEY_SCREENSIZE_5X,
					VIEW_KEY_SCREENSIZE_6X,
					VIEW_KEY_SCREENSIZE_7X,
					VIEW_KEY_SCREENSIZE_8X,
					VIEW_KEY_SCREENSIZE_9X,
					VIEW_KEY_SCREENSIZE_MAX,
					VIEW_KEY_SHOW_MENU,
					VIEW_KEY_SHOW_STATUSBAR,
					VIEW_KEY_SHOW_ONTOP,
					VIEW_KEY_SHOW_FPS,
					VIEW_KEY_FULLSCREEN,
					VIEW_NUM_KEYS,

					HELP_KEYS = VIEW_KEYS + VIEW_NUM_KEYS,
					HELP_KEY_HELP = 0,
					HELP_NUM_KEYS,

					NUM_KEYS = HELP_KEYS + HELP_NUM_KEYS,
					NUM_COMMAND_KEYS = NUM_KEYS - COMMAND_KEYS
				};

				enum
				{
					AUTOFIRE_MAX_SPEED = 6,
					AUTOFIRE_NUM_SPEEDS = 7,
					AUTOFIRE_DEFAULT_SPEED = 3
				};

				typedef DirectX::DirectInput::Key Key;

			private:

				Settings();

				void Reset(const DirectX::DirectInput&);
				void Reset(const DirectX::DirectInput&,uint);
				void Clear();
				void Clear(uint);

			#pragma pack(push,1)

				struct Type
				{
					uchar offset;
					ushort name;
				};

				struct Mapping
				{
					inline uint Code() const;
					inline uint Alt() const;
					inline uint Shift() const;
					inline uint Ctrl() const;

					uchar index;
					ushort key;
					ushort dlgName;
					cstring cfgType;
					cstring cfgKey;
				};

			#pragma pack(pop)

				enum
				{
					NO_KEY = 0,
					OFFSET_COUNT = NUM_TYPES + 1,
					ALT = 0x100,
					SHIFT = 0x200,
					CTRL = 0x400
				};

				bool Map(uint,const Key&);
				inline void Unmap(uint);

				static inline const Mapping& GetMapping(uint,uint);
				static inline const Type& GetType(uint);

				Key keys[NUM_KEYS];
				uint autoFireSpeed;
				bool allowSimulAxes;

				static const Type types[OFFSET_COUNT];
				static const Mapping map[NUM_KEYS];

				static uint NumTypeKeys(uint index)
				{
					NST_ASSERT( index < NUM_TYPES );
					return types[index+1].offset - types[index].offset;
				}

			public:

				const Key& GetKey(uint type,uint index) const
				{
					NST_ASSERT( index < NumTypeKeys(type) );
					return keys[types[type].offset + index];
				}

				const Key& GetKey(uint index) const
				{
					NST_ASSERT( index < NUM_KEYS );
					return keys[index];
				}

				const Key* GetKeys(uint index=0) const
				{
					NST_ASSERT( index < NUM_KEYS );
					return keys + index;
				}

				const Key* GetKeys(uint type,uint index) const
				{
					return &GetKey( type, index );
				}

				uint AutoFireSpeed() const
				{
					return autoFireSpeed;
				}

				bool AllowSimulAxes() const
				{
					return allowSimulAxes;
				}
			};

		private:

			void SelectNextMapKey();
			bool MapSelectedKey(const Settings::Key&);

			Nes::Input nes;
			DirectX::DirectInput& directInput;
			Settings settings;
			Dialog dialog;

		public:

			void Open()
			{
				dialog.Open();
			}

			const Settings& GetSettings() const
			{
				return settings;
			}
		};
	}
}

#endif
