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

#include "NstWindowParam.hpp"
#include "NstWindowRect.hpp"
#include "NstWindowDropFiles.hpp"

namespace Nestopia
{
	namespace Window
	{
		DropFiles::DropFiles(const Param& param)
		: hDrop(reinterpret_cast<HDROP>(param.wParam)), hWnd(param.hWnd) {}

		DropFiles::~DropFiles()
		{
			::DragFinish( hDrop );
		}

		uint DropFiles::Size() const
		{
			return ::DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 );
		}

		Path DropFiles::operator [] (uint i) const
		{
			Path file;

			if (const uint length = ::DragQueryFile( hDrop, i, NULL, 0 ))
			{
				file.Resize( length );
				::DragQueryFile( hDrop, i, file.Ptr(), length + 1 );
			}

			return file;
		}

		bool DropFiles::Inside(HWND const hChild) const
		{
			Point point;
			::DragQueryPoint( hDrop, &point );
			return Rect::Window(hChild).Inside( point.ScreenTransform(hWnd) );
		}
	}
}
