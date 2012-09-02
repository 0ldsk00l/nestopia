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

#ifndef NST_DIALOG_FIND_H
#define NST_DIALOG_FIND_H

#pragma once

#include "NstObjectPod.hpp"
#include "NstObjectDelegate.hpp"
#include "NstWindowGeneric.hpp"
#include "NstString.hpp"
#include <CommDlg.h>

namespace Nestopia
{
	namespace Window
	{
		class Custom;
		struct Param;

		class Finder
		{
		public:

			explicit Finder(Custom&);
			~Finder();

			enum
			{
				DOWN = FR_DOWN,
				WHOLEWORD = FR_WHOLEWORD,
				MATCHCASE = FR_MATCHCASE
			};

			void Close();

		private:

			typedef Object::Delegate<void,GenericString,uint> Callback;

			void Open(const Callback&,uint);

			ibool OnMsg(Param&);

			enum {BUFFER_SIZE = 512-1};

			Window::Generic window;
			const Window::Custom& parent;
			Object::Pod<FINDREPLACE> findReplace;
			Callback callback;

		public:

			template<typename Data,typename Code>
			void Open(Data* data,Code code,uint dir=DOWN)
			{
				Open( Callback(data,code), dir );
			}
		};
	}
}

#endif
