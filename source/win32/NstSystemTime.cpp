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

#include <cstdio>
#include "NstSystemTime.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace System
	{
		Time::Time()
		{
			Clear();
		}

		void Time::SetLocal(const void* const ptr)
		{
			const SYSTEMTIME& local = *static_cast<const SYSTEMTIME*>(ptr);

			year = local.wYear;
			month = local.wMonth;
			day = local.wDay;
			hour = local.wHour;
			minute = local.wMinute;
			second = local.wSecond;
			milli = local.wMilliseconds;
		}

		void Time::Set()
		{
			SYSTEMTIME local;
			::GetLocalTime( &local );
			SetLocal( &local );
		}

		bool Time::Set(wcstring const path)
		{
			NST_ASSERT( path && *path );

			WIN32_FILE_ATTRIBUTE_DATA data;
			SYSTEMTIME system, local;

			if
			(
				!::GetFileAttributesEx( path, GetFileExInfoStandard, &data ) ||
				!::FileTimeToSystemTime( &data.ftLastWriteTime, &system ) ||
				!::SystemTimeToTzSpecificLocalTime( NULL, &system, &local )
			)
				return false;

			SetLocal( &local );

			return true;
		}

		void Time::Clear()
		{
			std::memset( this, 0, sizeof(*this) );
		}

		bool Time::operator < (const Time& time) const
		{
			if ( year < time.year     ) return true;
			if ( year > time.year     ) return false;
			if ( month < time.month   ) return true;
			if ( month > time.month   ) return false;
			if ( day < time.day       ) return true;
			if ( day > time.day       ) return false;
			if ( hour < time.hour     ) return true;
			if ( hour > time.hour     ) return false;
			if ( minute < time.minute ) return true;
			if ( minute > time.minute ) return false;
			if ( second < time.second ) return true;
			if ( second > time.second ) return false;
			if ( milli < time.milli   ) return true;
			if ( milli > time.milli   ) return false;

			return false;
		}

		bool Time::operator == (const Time& time) const
		{
			return
			(
				milli == time.milli &&
				second == time.second &&
				minute == time.minute &&
				hour == time.hour &&
				day == time.day &&
				month == time.month &&
				year == time.year
			);
		}

		bool Time::Almost(const Time& time) const
		{
			return
			(
				minute == time.minute &&
				hour == time.hour &&
				day == time.day &&
				month == time.month &&
				year == time.year
			);
		}

		HeapString Time::ToString(const bool useSeconds) const
		{
			char buffer[64];
			buffer[0] = '\0';

			std::sprintf
			(
				buffer,
				"%02u/%02u/%u  %02u:%02u:%02u",
				uint( month  ),
				uint( day    ),
				uint( year   ),
				uint( hour   ),
				uint( minute ),
				uint( second )
			);

			if (!useSeconds)
				buffer[std::strlen(buffer) - 3] = '\0';

			return buffer;
		}
	}
}
