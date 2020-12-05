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

#ifndef NST_DIALOG_VIDEODECODER_H
#define NST_DIALOG_VIDEODECODER_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class VideoDecoder
		{
		public:

			explicit VideoDecoder(Nes::Video);
			~VideoDecoder();

			static void Load(const Configuration&,Nes::Video);
			static void Save(Configuration&,const Nes::Video);

		private:

			struct Handlers;

			void Update() const;

			ibool OnInitDialog     (Param&);
			ibool OnHScroll        (Param&);
			ibool OnCmdGain        (Param&);
			ibool OnCmdBoostYellow (Param&);
			ibool OnCmdPreset      (Param&);
			ibool OnCmdOk          (Param&);

			Dialog dialog;
			Nes::Video nes;
			Nes::Video::Decoder final;

		public:

			void Open()
			{
				dialog.Open();
			}
		};
	}
}

#endif
