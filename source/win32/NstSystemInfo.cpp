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

#include "NstSystemInfo.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace System
	{
		namespace Info
		{
			uint GetCpuCount()
			{
				struct Stored
				{
					uint count;

					Stored()
					{
						SYSTEM_INFO sysinfo;
						::GetSystemInfo( &sysinfo );
						count = NST_MIN(sysinfo.dwNumberOfProcessors,1);
					}
				};

				static const Stored stored;
				return stored.count;
			}

			uint GetCpuSpeed()
			{
				struct Stored
				{
					uint mHz;

					Stored()
					: mHz(0)
					{
						HKEY hKey;

						if
						(
							ERROR_SUCCESS == ::RegOpenKeyEx
							(
								HKEY_LOCAL_MACHINE,
								L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
								0,
								KEY_QUERY_VALUE,
								&hKey
							)
						)
						{
							DWORD data, size;

							if
							(
								ERROR_SUCCESS == ::RegQueryValueEx
								(
									hKey,
									L"~MHz",
									NULL,
									&data,
									NULL,
									&size
								)
								&& data == REG_DWORD
								&& size == sizeof(DWORD)
								&& ERROR_SUCCESS == ::RegQueryValueEx
								(
									hKey,
									L"~MHz",
									NULL,
									NULL,
									reinterpret_cast<BYTE*>(&data),
									&size
								)
							)
								mHz = data;

							::RegCloseKey( hKey );
						}
					}
				};

				static const Stored stored;
				return stored.mHz;
			}
		}
	}
}
