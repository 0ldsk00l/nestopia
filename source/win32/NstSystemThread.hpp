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

#ifndef NST_SYSTEM_THREAD_H
#define NST_SYSTEM_THREAD_H

#pragma once

#include "NstObjectDelegate.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace System
	{
		class Thread
		{
			HANDLE hEnter;
			HANDLE hExit;
			HANDLE hAbort;

		public:

			Thread();
			~Thread();

			class Terminator : public ImplicitBool<Terminator>
			{
				friend class Thread;

				HANDLE const hAbort;

				inline explicit Terminator(HANDLE=NULL);

			public:

				bool operator ! () const
				{
					return ::WaitForSingleObject( hAbort, 0 ) != WAIT_OBJECT_0;
				}
			};

			typedef Object::Delegate<void,Terminator> Callback;

			void Start(const Callback&,int=0);
			void Stop();

			bool Idle() const
			{
				return !hEnter;
			}
		};
	}
}

#endif
