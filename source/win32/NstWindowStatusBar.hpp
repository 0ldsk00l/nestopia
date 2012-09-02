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

#ifndef NST_WINDOW_STATUSBAR_H
#define NST_WINDOW_STATUSBAR_H

#pragma once

#include "NstWindowGeneric.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Custom;

		class StatusBar
		{
		public:

			StatusBar(Custom&,uint);
			~StatusBar();

			void Enable(bool=true,bool=true);
			void Show() const;
			uint Height() const;
			uint GetMaxMessageLength() const;

			enum
			{
				FIRST_FIELD,
				SECOND_FIELD
			};

		private:

			enum {CHILD_ID=1000};

			void Update() const;
			void OnSize(Param&);
			void OnDestroy(Param&);

			struct Width
			{
				enum
				{
					DEF_CHAR_WIDTH = 7,
					DEF_FIRST_WIDTH = 10
				};

				inline explicit Width(uint);

				void Calculate(HWND);

				const uint numChars;
				uint character;
				uint first;
			};

			Width width;
			Custom& parent;
			Generic window;

		public:

			bool Enabled() const
			{
				return window;
			}

			void Disable()
			{
				Enable( false );
			}

			class Stream
			{
				const Generic window;
				const uint field;

			public:

				Stream(Generic w,uint f)
				: window(w), field(f) {}

				void operator << (wcstring) const;
				void Clear() const;
			};

			Stream Text(uint field=0) const
			{
				return Stream( window, field );
			}
		};
	}
}

#endif
