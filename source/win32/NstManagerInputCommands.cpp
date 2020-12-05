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

#include "NstManager.hpp"
#include "NstManagerInput.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Input::Commands::CmdKey::CmdKey(const Key& key,uint c)
		: Key(key), cmd(c), prev(false) {}

		Input::Commands::Commands(Window::Custom& w,DirectX::DirectInput& d)
		: acquired(false), clock(CLOCK_DEFAULT), window(w), directInput(d)
		{
			window.Messages().Hooks().Add( WM_SETFOCUS, this, &Commands::OnFocus );
		}

		Input::Commands::~Commands()
		{
			window.StopTimer( this, &Commands::OnTimer );
			window.Messages().Hooks().Remove( this );
		}

		void Input::Commands::Update()
		{
			if (!acquired && CanPoll())
			{
				clock = CLOCK_DEFAULT;
				window.StartTimer( this, &Commands::OnTimer, POLL_RAPID );
			}
		}

		void Input::Commands::Add(const Key& key,uint cmd)
		{
			if (key.Assigned() && !key.IsVirtualKey())
				keys.PushBack( CmdKey(key,cmd) );
		}

		void Input::Commands::BeginAdd()
		{
			keys.Clear();
		}

		void Input::Commands::EndAdd()
		{
			keys.Defrag();
			Update();
		}

		void Input::Commands::Acquire()
		{
			acquired = true;
		}

		void Input::Commands::Unacquire()
		{
			acquired = false;
			Update();
		}

		void Input::Commands::OnFocus(Window::Param&)
		{
			Update();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool Input::Commands::ForcePoll()
		{
			NST_ASSERT( keys.Size() );

			Keys::Iterator it = keys.Begin();
			Keys::ConstIterator const end = keys.End();

			do
			{
				if (it->GetToggle( it->prev ))
				{
					window.PostCommand( it->cmd );
					return true;
				}
			}
			while (++it != end);

			return false;
		}

		uint Input::Commands::OnTimer()
		{
			if (!acquired)
			{
				if (CanPoll())
				{
					directInput.Poll();

					if (ForcePoll())
					{
						clock = POLL_RAPID;
						return POLL_REST;
					}
					else
					{
						uint next = clock;
						clock = CLOCK_DEFAULT;
						return next;
					}
				}
				else
				{
					directInput.Unacquire();
				}
			}

			return CLOCK_STOP;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
