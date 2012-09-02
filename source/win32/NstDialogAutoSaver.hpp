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

#ifndef NST_DIALOG_AUTOSAVER_H
#define NST_DIALOG_AUTOSAVER_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class AutoSaver
		{
		public:

			explicit AutoSaver(const Managers::Paths&);
			~AutoSaver();

			enum
			{
				DEFAULT_SAVE_SLOT = 0
			};

			const Path GetStateFile() const;

		private:

			struct Handlers;

			struct Settings
			{
				Settings();

				uint interval;
				bool notify;
				Path stateFile;
			};

			ibool OnInitDialog (Param&);
			ibool OnCmdBrowse  (Param&);
			ibool OnCmdClear   (Param&);
			ibool OnCmdOk      (Param&);

			Dialog dialog;
			const Managers::Paths& paths;
			Settings settings;

		public:

			void Open()
			{
				dialog.Open();
			}

			uint GetInterval() const
			{
				return settings.interval * 60 * 1000;
			}

			bool ShouldNotify() const
			{
				return settings.notify;
			}
		};
	}
}

#endif
