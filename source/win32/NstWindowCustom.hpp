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

#ifndef NST_WINDOW_CUSTOM_H
#define NST_WINDOW_CUSTOM_H

#pragma once

#include "NstCollectionRouter.hpp"
#include "NstWindowGeneric.hpp"

namespace Nestopia
{
	namespace Window
	{
		using Application::Configuration;

		struct Param;
		typedef Collection::Router<ibool,Param&,uint> MsgHandler;

		class Custom : public Generic
		{
		protected:

			MsgHandler msgHandler;

			Custom() {}

			template<typename Owner,typename MsgArray>
			Custom(Owner* owner,MsgArray& array)
			: msgHandler( owner, array ) {}

			template<typename Owner,typename Msg>
			Custom(uint mid,Owner* owner,Msg msg)
			: msgHandler( mid, owner, msg ) {}

		private:

			typedef Object::Delegate<uint> TimerCallback;

			struct Timer : TimerCallback
			{
				Timer(const TimerCallback&);

				bool active;
			};

			typedef Collection::Vector<Timer> Timers;

			void StartTimer(TimerCallback,uint) const;
			bool StopTimer(TimerCallback) const;

			static void CALLBACK TimerProc(HWND,uint,UINT_PTR,DWORD);

			static Timers timers;

		public:

			MsgHandler& Messages()
			{
				return msgHandler;
			}

			template<typename Data,typename Code>
			void StartTimer(Data* data,Code code,uint duration) const
			{
				StartTimer( TimerCallback(data,code), duration );
			}

			template<typename Data,typename Code>
			bool StopTimer(Data* data,Code code) const
			{
				return StopTimer( TimerCallback(data,code) );
			}
		};
	}
}

#endif
