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

#ifndef NST_WINDOW_DYNAMIC_H
#define NST_WINDOW_DYNAMIC_H

#pragma once

#include "NstWindowCustom.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Dynamic : public Custom
		{
			HeapString className;

		public:

			struct Context
			{
				wcstring className;
				DWORD classStyle;
				HCURSOR hCursor;
				HICON hIcon;
				HBRUSH hBackground;
				wcstring windowName;
				DWORD winStyle;
				DWORD exStyle;
				int x;
				int y;
				int width;
				int height;
				HWND hParent;
				HMENU hMenu;

				Context()
				:
				className   (NULL),
				classStyle  (0),
				hCursor     (NULL),
				hIcon       (NULL),
				hBackground (NULL),
				windowName  (NULL),
				winStyle    (0),
				exStyle     (0),
				x           (CW_USEDEFAULT),
				y           (CW_USEDEFAULT),
				width       (CW_USEDEFAULT),
				height      (CW_USEDEFAULT),
				hParent     (NULL),
				hMenu       (NULL)
				{}
			};

			Dynamic();

			template<typename Owner,typename MsgArray>
			Dynamic(Owner*,MsgArray&);

			~Dynamic();

			void Create(Context&);
			void Destroy();

		private:

			void Initialize();

			typedef Collection::Vector<Dynamic*> Instances;

			void OnCreate(HWND);
			ibool OnNcDestroy(Param&);

			static LRESULT CALLBACK WndProcSingle (HWND,uint,WPARAM,LPARAM);
			static LRESULT CALLBACK WndProcMulti  (HWND,uint,WPARAM,LPARAM);
			static LRESULT CALLBACK WndProcCreate (HWND,uint,WPARAM,LPARAM);

			static Instances instances;
		};

		template<typename Owner,typename MsgArray>
		Dynamic::Dynamic(Owner* owner,MsgArray& array)
		: Custom( owner, array )
		{
			Initialize();
		}
	}
}

#endif
