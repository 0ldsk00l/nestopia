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

#ifndef NST_MANAGER_INPUT_H
#define NST_MANAGER_INPUT_H

#pragma once

#include "NstDirectInput.hpp"
#include "NstResourceCursor.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Input;
	}

	namespace Managers
	{
		class Input : Manager
		{
		public:

			typedef Object::Delegate<void,Window::Rect&> Screening;

			Input
			(
				Window::Custom&,
				Window::Menu&,
				Emulator&,
				const Configuration&,
				const Screening&,
				const Screening&
			);

			~Input();

			void Save(Configuration&) const;
			void StartEmulation();
			void StopEmulation();

		private:

			inline void CheckPoll();
			void ForcePoll();

			bool SetAdapter(uint) const;
			void SyncControllers(uint,Nes::Input::Type,uint) const;

			void UpdateDevices();
			void UpdateSettings();
			void UpdateAdapterPort(Nes::Input::Adapter) const;

			void OnMenuPort1(const Window::Menu::PopupHandler::Param&);
			void OnMenuPort2(const Window::Menu::PopupHandler::Param&);
			void OnMenuPort3(const Window::Menu::PopupHandler::Param&);
			void OnMenuPort4(const Window::Menu::PopupHandler::Param&);
			void OnMenuPort5(const Window::Menu::PopupHandler::Param&);
			void OnEmuEvent(Emulator::Event,Emulator::Data);
			void OnCmdMachineAutoSelectController(uint);
			void OnCmdMachinePort(uint);
			void OnCmdMachineAdapter(uint);
			void OnCmdOptionsInput(uint);

			class Callbacks;

			class Cursor
			{
			public:

				Cursor(Window::Custom&,const Screening&,const Screening&);
				~Cursor();

				void Acquire(Emulator&);
				void Unacquire();
				void AutoHide();
				bool Poll(uint&,uint&,uint&,uint* = NULL) const;

			private:

				enum
				{
					TIME_OUT    = 3000,
					NO_DEADLINE = DWORD(~0UL),
					WHEEL_MIN   = -WHEEL_DELTA*30,
					WHEEL_MAX   = +WHEEL_DELTA*30,
					WHEEL_SCALE = 56
				};

				ibool OnNop            (Window::Param&);
				ibool OnSetCursor      (Window::Param&);
				ibool OnMouseMove      (Window::Param&);
				ibool OnLButtonDown    (Window::Param&);
				ibool OnRButtonDown    (Window::Param&);
				ibool OnRButtonDownNop (Window::Param&);
				ibool OnButtonUp       (Window::Param&);
				ibool OnWheel          (Window::Param&);

				HCURSOR hCursor;
				HCURSOR hCurrent;
				DWORD deadline;
				Window::Custom& window;
				int wheel;
				const Screening getInputRect;
				const Screening getOutputRect;
				const Resource::Cursor gun;
				const uint primaryButtonId;
				const uint secondaryButtonId;

			public:

				int GetWheel() const
				{
					return wheel / WHEEL_SCALE;
				}

				bool MustAutoHide() const
				{
					return deadline != NO_DEADLINE && deadline <= ::GetTickCount();
				}
			};

			class Commands
			{
				typedef DirectX::DirectInput::Key Key;

			public:

				Commands(Window::Custom&,DirectX::DirectInput&);
				~Commands();

				void BeginAdd();
				void Add(const Key&,uint);
				void EndAdd();
				void Acquire();
				void Unacquire();

			private:

				bool ForcePoll();
				void Update();
				uint OnTimer();
				void OnFocus(Window::Param&);

				enum
				{
					POLL_RAPID    = 50,
					POLL_REST     = 1000,
					CLOCK_DEFAULT = 1,
					CLOCK_STOP    = 0
				};

				struct CmdKey : Key
				{
					CmdKey(const Key&,uint);

					uint cmd;
					bool prev;
				};

				typedef Collection::Vector<CmdKey> Keys;

				bool acquired;
				Keys keys;
				uint clock;
				Window::Custom& window;
				DirectX::DirectInput& directInput;

				bool CanPoll() const
				{
					return !keys.Empty() && window.Focused();
				}

			public:

				void Poll()
				{
					if (CanPoll())
						ForcePoll();
				}
			};

			class Clipboard : Manager
			{
			public:

				Clipboard(Emulator&,Window::Menu&);

				enum Type
				{
					FAMILY,
					SUBOR
				};

				uint Query(const uchar* NST_RESTRICT,Type);
				void Clear();
				void operator ++ ();

			private:

				bool CanPaste() const;
				void OnCmdMachineKeyboardPaste(uint);
				void OnMenuKeyboard(const Window::Menu::PopupHandler::Param&);

				uint pos;
				uchar releasing;
				uchar hold;
				bool shifted;
				bool paste;
				String::Heap<wchar_t> buffer;

			public:

				bool Shifted() const
				{
					return shifted;
				}

				void Shift()
				{
					shifted = true;
				}

				uint operator * ()
				{
					return releasing ? (--releasing, UINT_MAX) : buffer[pos];
				}
			};

			class AutoFire : public ImplicitBool<AutoFire>
			{
			public:

				inline AutoFire();
				void operator = (uint);

			private:

				uint step;
				uint signal;

			public:

				bool operator ! () const
				{
					return step < signal;
				}

				void operator ++ ()
				{
					step = (step < signal) ? (step + 1) : 0;
				}
			};

			ibool polled;
			AutoFire autoFire;
			Cursor cursor;
			Nes::Input::Controllers nesControllers;
			DirectX::DirectInput directInput;
			Object::Heap<Window::Input> dialog;
			Commands commands;
			Clipboard clipboard;

		public:

			void Calibrate(bool full)
			{
				directInput.Calibrate( full );
			}

			Nes::Input::Controllers* GetOutput()
			{
				++autoFire;
				return &nesControllers;
			}

			void Poll()
			{
				polled ^= 1;

				if (polled)
					ForcePoll();

				if (cursor.MustAutoHide())
					cursor.AutoHide();
			}
		};
	}
}

#endif
