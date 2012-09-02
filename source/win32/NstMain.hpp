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

#ifndef NST_MAIN_H
#define NST_MAIN_H

#pragma once

#include "../core/NstBase.hpp"

#if NST_MSVC >= 1200 || NST_ICC >= 800
#pragma warning( push )
#endif

#include "../core/NstCore.hpp"
#include "../core/NstAssert.hpp"

#if NST_MSVC >= 1200 || NST_ICC >= 800
#pragma warning( pop )
#endif

#if UINT_MAX < 0xFFFFFFFF || INT_MAX < 2147483647 || INT_MIN > -2147483647
#error Unsupported plattform!
#endif

#if NST_ICC

 #pragma warning( disable : 11 304 373 383 444 810 981 1572 1599 1786 )

#elif NST_MSVC

 #pragma warning( disable : 4018 4127 4244 4245 4355 4389 4512 4800 4996 )

 #if NST_MSVC >= 1400

  #pragma warning( default : 4263 4287 4289 4296 4350 4545 4546 4547 4549 4555 4557 4686 4836 4905 4906 4928 4946 )

  #if 0
  #pragma warning( default : 4820 ) // byte padding on structs
  #pragma warning( default : 4710 ) // function not inlined
  #pragma warning( default : 4711 ) // function auto inlined
  #endif

 #endif

#endif

// minimum operating system: Windows 98 2nd.edition

#define WINVER         0x0500
#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT   0x0500
#define _WIN32_IE      0x0401

#define _WIN32_DCOM

#define STRICT
#define WIN32_LEAN_AND_MEAN

// <windows.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define NOGDICAPMASKS
#define NOICONS
#define NOKEYSTATES
#define NORASTEROPS
#define NOATOM
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

// <commctrl.h>

#define NOUPDOWN
#define NOMENUHELP
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOHEADER
#define NOTABCONTROL
#define NOANIMATE
#define NOBUTTON
#define NOSTATIC
#define NOEDIT
#define NOLISTBOX
#define NOCOMBOBOX
#define NOSCROLLBAR

namespace Nestopia
{
	typedef signed char schar;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;
	typedef uint ibool;
	typedef const char* cstring;
	typedef const wchar_t* wcstring;
	typedef Nes::qword qword;

	#define NST_CLAMP(t_,x_,y_) ((t_) < (x_) ? (x_) : (t_) > (y_) ? (y_) : (t_))

	template<typename T,uint N>
	char(& array(T(&)[N]))[N];

	template<typename T>
	class ImplicitBool;

	template<>
	class ImplicitBool<void>
	{
	public:

		int type;
		typedef int ImplicitBool<void>::*Type;
	};

	template<typename T>
	class ImplicitBool
	{
		template<typename U> void operator == (const ImplicitBool<U>&) const;
		template<typename U> void operator != (const ImplicitBool<U>&) const;

	public:

		operator ImplicitBool<void>::Type () const
		{
			return !static_cast<const T&>(*this) ? 0 : &ImplicitBool<void>::type;
		}
	};

	template<uchar A=0,uchar B=0,uchar C=0,uchar D=0>
	struct FourCC
	{
		enum
		{
			V = uint(A) << 0 | uint(B) << 8 | uint(C) << 16 | uint(D) << 24
		};

		template<typename P>
		static uint T(const P*);
	};

	template<uchar A,uchar B,uchar C,uchar D> template<typename P>
	uint FourCC<A,B,C,D>::T(const P* p)
	{
		return (p[0] & 0xFFU) << 0 | (p[1] & 0xFFU) << 8 | (p[2] & 0xFFU) << 16 | (p[3] & 0xFFU) << 24;
	}

	namespace Collection
	{
		template<typename> class Vector;
		typedef Vector<char> Buffer;
	}

	namespace Application
	{
		class Configuration;
	}

	enum
	{
		IDM_POS_FILE = 0,
		IDM_POS_FILE_QUICKLOADSTATE = 6,
		IDM_POS_FILE_QUICKSAVESTATE = 7,
		IDM_POS_FILE_SOUNDRECORDER = 12,
		IDM_POS_FILE_MOVIE = 13,
		IDM_POS_FILE_RECENTFILES = 17,
		IDM_POS_FILE_RECENTDIRS = 18,
		IDM_POS_MACHINE = 1,
		IDM_POS_MACHINE_RESET = 1,
		IDM_POS_MACHINE_INPUT = 3,
		IDM_POS_MACHINE_INPUT_PORT1 = 2,
		IDM_POS_MACHINE_INPUT_PORT2 = 3,
		IDM_POS_MACHINE_INPUT_PORT3 = 4,
		IDM_POS_MACHINE_INPUT_PORT4 = 5,
		IDM_POS_MACHINE_INPUT_EXP = 6,
		IDM_POS_MACHINE_INPUT_ADAPTER = 8,
		IDM_POS_MACHINE_EXT = 4,
		IDM_POS_MACHINE_EXT_FDS = 0,
		IDM_POS_MACHINE_EXT_KEYBOARD = 1,
		IDM_POS_MACHINE_EXT_TAPE = 2,
		IDM_POS_MACHINE_EXT_FDS_INSERT = 0,
		IDM_POS_MACHINE_NSF = 5,
		IDM_POS_MACHINE_REGION = 6,
		IDM_POS_MACHINE_OPTIONS = 7,
		IDM_POS_VIEW = 3,
		IDM_POS_VIEW_SCREENSIZE = 3,
		IDM_POS_OPTIONS = 4,
		IDM_POS_OPTIONS_AUTOSAVER = 10,
		IDM_POS_OPTIONS_AUTOSAVER_OPTIONS = 0,
		IDM_POS_OPTIONS_AUTOSAVER_START
	};
}

#endif
