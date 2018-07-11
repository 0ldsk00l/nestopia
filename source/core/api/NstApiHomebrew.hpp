////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2018-2018 Phil Smith
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

#ifndef NST_API_HOMEBREW_H
#define NST_API_HOMEBREW_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		/**
		* Homebrew interface.
		*/
		class Homebrew : public Base
		{
		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Homebrew(T& instance)
			: Base(instance) {}

			/**
			* Sets the exit port.
			*
			* @param address exit port address,
			*   any previous exit port will be removed
			* @return result code
			*/
			Result SetExitPort(ushort address) throw();

			/**
			* Removes the exit port.
			*
			* @return result code
			*/
			Result ClearExitPort() throw();

			/**
			* Sets the standard out port.
			*
			* @param address standard out port address,
			*   any previous standard out port will be removed
			* @return result code
			*/
			Result SetStdOutPort(ushort address) throw();

			/**
			* Removes the standard out port.
			*
			* @return result code
			*/
			Result ClearStdOutPort() throw();

			/**
			* Sets the standard error port.
			*
			* @param address standard error port address,
			*   any previous standard error port will be removed
			* @return result code
			*/
			Result SetStdErrPort(ushort address) throw();

			/**
			* Removes the standard error port.
			*
			* @return result code
			*/
			Result ClearStdErrPort() throw();

			/**
			* Returns current number of ports.
			*
			* @return number
			*/
			ulong NumPorts() const throw();

			/**
			* Removes all existing ports.
			*
			* @return result code
			*/
			Result ClearPorts() throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
