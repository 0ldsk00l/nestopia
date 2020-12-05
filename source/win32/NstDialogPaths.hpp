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

#ifndef NST_DIALOG_PATHS_H
#define NST_DIALOG_PATHS_H

#pragma once

#include "NstCollectionBitSet.hpp"
#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Paths
		{
		public:

			enum Type
			{
				DIR_IMAGE,
				DIR_SAVE,
				DIR_STATE,
				DIR_SAMPLES,
				DIR_CHEATS,
				DIR_PATCHES,
				DIR_SCREENSHOT
			};

			enum ScreenShotFormat
			{
				SCREENSHOT_PNG,
				SCREENSHOT_JPEG,
				SCREENSHOT_BMP
			};

			enum
			{
				NUM_DIRS = DIR_SCREENSHOT + 1,
				NUM_SCREENSHOTS = SCREENSHOT_BMP + 1
			};

			enum
			{
				USE_LAST_IMAGE_DIR,
				READONLY_CARTRIDGE,
				AUTO_IMPORT_STATE_SLOTS,
				AUTO_EXPORT_STATE_SLOTS,
				CHEATS_AUTO_LOAD,
				CHEATS_AUTO_SAVE,
				PATCH_AUTO_APPLY,
				PATCH_BYPASS_VALIDATION,
				COMPRESS_STATES,
				NUM_FLAGS
			};

			explicit Paths(const Configuration&);
			~Paths();

			void Save(Configuration&) const;
			const GenericString GetScreenShotExtension() const;
			const Path GetDirectory(Type) const;

		private:

			struct Handlers;

			struct Settings
			{
				Settings();

				struct Flags : Collection::BitSet
				{
					inline Flags();
				};

				Flags flags;
				Path dirs[NUM_DIRS];
				ScreenShotFormat screenShotFormat;
			};

			void Update(bool) const;
			void UpdateDirectory(uint);
			void UpdateLastVisited() const;

			ibool OnInitDialog     (Param&);
			ibool OnCmdBrowse      (Param&);
			ibool OnCmdLastVisited (Param&);
			ibool OnCmdDefault     (Param&);
			ibool OnCmdOk          (Param&);

			Settings settings;
			Dialog dialog;

			struct Lut;

		public:

			void Open()
			{
				dialog.Open();
			}

			bool GetSetting(uint flag) const
			{
				return settings.flags[flag];
			}

			ScreenShotFormat GetScreenShotFormat() const
			{
				return settings.screenShotFormat;
			}
		};
	}
}

#endif
