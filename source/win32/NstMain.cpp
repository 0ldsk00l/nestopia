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

#include <new>
#include "NstApplicationException.hpp"
#include "NstApplicationMain.hpp"

#if NST_MSVC

 #ifndef _UNICODE
 #error compile with _UNICODE!
 #endif

 #ifdef _DEBUG
 #pragma comment(lib,"emucoredebug")
 #else
 #pragma comment(lib,"emucore")
 #endif

 #ifdef NST_DEBUG
 #define CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
 #endif

 #if NST_MSVC >= 1400 && !NST_ICC

  #ifdef _M_IX86
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
  #else
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
  #endif

 #endif

#endif

int WINAPI WinMain(HINSTANCE,HINSTANCE,char*,int cmdShow)
{
#if NST_MSVC && defined(NST_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	try
	{
		return Nestopia::Application::Main( cmdShow ).Run();
	}
	catch (const Nestopia::Application::Exception& exception)
	{
		return exception.Issue();
	}
	catch (const std::bad_alloc&)
	{
		return Nestopia::Application::Exception( IDS_ERR_OUT_OF_MEMORY ).Issue();
	}
#ifndef NST_DEBUG
	catch (...)
	{
		return Nestopia::Application::Exception( IDS_ERR_GENERIC ).Issue();
	}
#endif
}
