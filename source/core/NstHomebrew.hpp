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

#ifndef NST_HOMEBREW_H
#define NST_HOMEBREW_H

#ifndef NST_VECTOR_H
#include "NstVector.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Homebrew
		{
		public:

			explicit Homebrew(Cpu&);
			~Homebrew();

			void Reset();
			void ClearPorts();

			Result SetExitPort (word,bool);
			Result ClearExitPort ();

			Result SetStdOutPort (word,bool);
			Result ClearStdOutPort ();

			Result SetStdErrPort (word,bool);
			Result ClearStdErrPort ();

			dword NumPorts () const;

		private:

			NES_DECL_PEEK( Exit );
			NES_DECL_POKE( Exit );

			NES_DECL_PEEK( StdOut );
			NES_DECL_POKE( StdOut );

			NES_DECL_PEEK( StdErr );
			NES_DECL_POKE( StdErr );

			Result ActivateExitPort ();
			Result ActivateStdOutPort ();
			Result ActivateStdErrPort ();

			Cpu& cpu;

			word exitAddress;
			ibool exitSet;
			const Io::Port* exitPort;

			word stdOutAddress;
			ibool stdOutSet;
			const Io::Port* stdOutPort;

			word stdErrAddress;
			ibool stdErrSet;
			const Io::Port* stdErrPort;
		};
	}
}

#endif
