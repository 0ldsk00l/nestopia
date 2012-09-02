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

#ifndef NST_DIALOG_INESHEADER_H
#define NST_DIALOG_INESHEADER_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class InesHeader
		{
		public:

			InesHeader(const Managers::Paths&);

			void Open(const Path&);

		private:

			struct Handlers;
			class CustomSize;

			enum SizeType
			{
				SIZETYPE_EXT = 0,
				SIZETYPE_STD_8K = 8,
				SIZETYPE_STD_16K = 16,
				SIZETYPE_STATE = 2
			};

			enum
			{
				OTHER_SIZE     = 0x40000000,
				OTHER_SIZE_DIV = SIZETYPE_STD_8K|SIZETYPE_STD_16K,
				HEADER_SIZE    = 16,
				HEADER_ID      = FourCC<'N','E','S',0x1A>::V
			};

			typedef uchar Header[HEADER_SIZE];

			static uint Import(const Path&,Collection::Buffer&);
			static uint Export(const Path&,const Collection::Buffer&);

			void UpdateHeader(const Nes::Cartridge::NesHeader&) const;
			void UpdateVersion() const;
			void UpdateSystem() const;
			void UpdateSizes(uint,SizeType,uint) const;

			bool SaveHeader(Header&) const;
			uint GetMaxSize(uint) const;
			bool OkToSave(uint) const;

			ibool OnInitDialog   (Param&);
			ibool OnCmdFileType  (Param&);
			ibool OnCmdSystem    (Param&);
			ibool OnCmdSizeOther (Param&);
			ibool OnCmdUndoAll   (Param&);
			ibool OnCmdSave      (Param&);

			Dialog dialog;
			Header header;
			const Path* path;
			const Managers::Paths& paths;
		};
	}
}

#endif
