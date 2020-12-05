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

#include <process.h>
#include "language/resource.h"
#include "NstApplicationException.hpp"
#include "NstSystemThread.hpp"

namespace Nestopia
{
	namespace System
	{
		Thread::Thread()
		: hEnter(NULL), hExit(NULL), hAbort(NULL) {}

		Thread::~Thread()
		{
			Stop();
		}

		inline Thread::Terminator::Terminator(HANDLE handle)
		: hAbort(handle) {}

		void Thread::Start(const Callback& callback,const int priority)
		{
			Stop();

			if
			(
				NULL == (hEnter = ::CreateEvent( NULL, false, false, NULL )) ||
				NULL == (hExit  = ::CreateEvent( NULL, false, false, NULL )) ||
				NULL == (hAbort = ::CreateEvent( NULL, false, false, NULL ))
			)
				throw Application::Exception( IDS_ERR_FAILED, L"CreateEvent()" );

			class Entry
			{
				const Callback callback;
				const int priority;
				HANDLE const hEnter;
				HANDLE const hExit;
				const Terminator terminator;

			public:

				Entry(const Callback& c,int p,HANDLE e,HANDLE x,Terminator t)
				: callback(c), priority(p), hEnter(e), hExit(x), terminator(t) {}

				static void WINAPIV Run(void* data)
				{
					const Entry local( *static_cast<Entry*>(data) );

					NST_ASSERT( local.hEnter && local.hExit );

					if (local.priority)
						::SetThreadPriority( ::GetCurrentThread(), local.priority );

					::SetEvent( local.hEnter );

					try
					{
						local.callback( local.terminator );
					}
					catch (...)
					{
					}

					::SetEvent( local.hExit );
				}
			};

			Entry entry( callback, priority, hEnter, hExit, Terminator(hAbort) );

			if (!::_beginthread( Entry::Run, 0, &entry ))
				throw Application::Exception( IDS_ERR_FAILED, L"_beginthread()" );

			::WaitForSingleObject( hEnter, INFINITE );
		}

		void Thread::Stop()
		{
			if (hAbort)
				::SetEvent( hAbort );

			if (HANDLE const handle = hExit)
			{
				hExit = NULL;
				::WaitForSingleObject( handle, INFINITE );
				::CloseHandle( handle );
			}

			if (HANDLE const handle = hAbort)
			{
				hAbort = NULL;
				::CloseHandle( handle );
			}

			if (HANDLE const handle = hEnter)
			{
				hEnter = NULL;
				::CloseHandle( handle );
			}
		}
	}
}
