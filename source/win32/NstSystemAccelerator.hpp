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

#ifndef NST_SYSTEM_ACCELERATOR_H
#define NST_SYSTEM_ACCELERATOR_H

#pragma once

#include "NstString.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace System
	{
		class Accelerator
		{
		public:

			typedef Collection::Vector<ACCEL> Entries;

			uint Size() const;
			void Set(const ACCEL*,uint);
			bool Get(Entries&) const;
			void Clear();

			static const HeapString GetKeyName(const ACCEL&);

			ACCEL operator [] (uint) const;

		private:

			struct CopyTable;

			ACCEL* Copy(CopyTable&) const;

			HACCEL handle;

		public:

			Accelerator()
			: handle(NULL) {}

			~Accelerator()
			{
				if (handle)
					::DestroyAcceleratorTable( handle );
			}

			bool Enabled() const
			{
				return handle;
			}

			bool Translate(MSG& msg) const
			{
				NST_ASSERT( handle );
				return ::TranslateAccelerator( msg.hwnd, handle, &msg );
			}

			void Set(Entries& entries)
			{
				Set( entries.Ptr(), entries.Size() );
			}
		};
	}
}

#endif
