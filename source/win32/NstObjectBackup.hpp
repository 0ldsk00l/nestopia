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

#ifndef NST_OBJECT_BACKUP_H
#define NST_OBJECT_BACKUP_H

#pragma once

#include <cstring>
#include "NstObjectHeap.hpp"

namespace Nestopia
{
	namespace Object
	{
		class Backup
		{
			const uint size;
			void* const data;
			Heap<void> backup;

		public:

			Backup(void* p,uint s)
			: size(s), data(p), backup(s)
			{
				std::memcpy( backup, p, s );
			}

			void Restore() const
			{
				std::memcpy( data, backup, size );
			}
		};
	}
}

#endif
