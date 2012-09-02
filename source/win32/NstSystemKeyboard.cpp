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

#include "NstSystemKeyboard.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace System
	{
		Keyboard::KeyName Keyboard::keyNames[NUM_KEYS];

		void Keyboard::KeyName::Set(uint key)
		{
			uint flags = 0x200;

			switch (key)
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':

					operator << (wchar_t(key));

				case 0:
					break;

				case VK_INSERT:
				case VK_DELETE:
				case VK_HOME:
				case VK_END:
				case VK_NEXT:
				case VK_PRIOR:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:

					flags = 0x100|0x200;

				default:

					key = ::MapVirtualKey( key, 0 );

					if (key)
					{
						key = (key | flags) << 16;

						uint length;

						do
						{
							Reserve( Capacity() + 19 );
							length = ::GetKeyNameText( key, Ptr(), Capacity() + 1 );
						}
						while (length == Capacity());

						if (length)
						{
							ShrinkTo( length );

							::CharUpperBuff( Ptr(), 1 );

							if (length > 1)
								::CharLowerBuff( Ptr() + 1, length - 1 );
						}
					}
					break;
			}
		}

		const GenericString Keyboard::GetName(uint key)
		{
			NST_ASSERT( key < NUM_KEYS );

			if (keyNames[key].Empty())
				keyNames[key].Set( key );

			return keyNames[key];
		}

		uint Keyboard::GetKey(const GenericString name)
		{
			for (uint key=0; key < NUM_KEYS; ++key)
			{
				if (GetName(key) == name)
					return key;
			}

			return NONE;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool Keyboard::ToggleIndicator(const Indicator indicator,const bool on)
		{
			uint key = indicator;

			switch (indicator)
			{
				case CAPS_LOCK:   key = VK_CAPITAL; break;
				case NUM_LOCK:    key = VK_NUMLOCK; break;
				case SCROLL_LOCK: key = VK_SCROLL;  break;
			}

			if (on != (::GetKeyState( key ) & 0x1U))
			{
				INPUT input[2];
				std::memset( input, 0, sizeof(input) );

				input[1].type = input[0].type = INPUT_KEYBOARD;
				input[1].ki.wVk = input[0].ki.wVk = key;

				input[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
				::SendInput( 2, input, sizeof(INPUT) );

				input[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP;
				::SendInput( 2, input, sizeof(INPUT) );

				return true;
			}

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
