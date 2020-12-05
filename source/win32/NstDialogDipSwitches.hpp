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

#ifndef NST_DIALOG_DIPSWITCHES_H
#define NST_DIALOG_DIPSWITCHES_H

#pragma once

#include "NstWindowDialog.hpp"
#include "../core/api/NstApiDipSwitches.hpp"

namespace Nestopia
{
	namespace Window
	{
		class DipSwitches
		{
		public:

			explicit DipSwitches(Managers::Emulator&,bool);

		private:

			struct Handlers;

			#ifdef IDC_DIPSWITCHES_9
			#error Must update dipswitch dialog fields!
			#endif

			enum
			{
				MAX_DIPS = IDC_DIPSWITCHES_8+1-IDC_DIPSWITCHES_1
			};

			ibool OnInitDialog (Param&);
			ibool OnCmdOk      (Param&);

			Dialog dialog;
			Nes::DipSwitches dipSwitches;
			const bool userOpened;

		public:

			bool Open()
			{
				return dipSwitches.NumDips() ? dialog.Open() : false;
			}
		};
	}
}

#endif
