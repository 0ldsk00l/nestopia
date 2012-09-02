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

#ifndef NST_WINDOW_MAIN_H
#define NST_WINDOW_MAIN_H

#pragma once

#include "NstWindowDynamic.hpp"
#include "NstManagerVideo.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerInput.hpp"
#include "NstManagerFrameClock.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Main : Managers::Manager
		{
		public:

			Main
			(
				Managers::Emulator&,
				const Configuration&,
				Menu&,
				const Managers::Paths&,
				const Managers::Preferences&,
				int
			);

			~Main();

			int  Run();
			void Save(Configuration&) const;
			uint GetMaxMessageLength() const;

		private:

			struct MainWindow : Dynamic
			{
				MainWindow(const Configuration&,const Menu&);

				enum
				{
					CLASS_STYLE =
					(
						CS_HREDRAW |
						CS_VREDRAW
					),

					WIN_STYLE =
					(
						WS_OVERLAPPED |
						WS_CAPTION |
						WS_SYSMENU |
						WS_MINIMIZEBOX |
						WS_MAXIMIZEBOX |
						WS_THICKFRAME |
						WS_CLIPCHILDREN
					),

					WIN_EXSTYLE =
					(
						WS_EX_ACCEPTFILES |
						WS_EX_CLIENTEDGE
					)
				};

				bool menu;
				bool maximized;
				Rect rect;

				static const wchar_t name[];
			};

			inline bool Fullscreen() const;
			inline bool Windowed() const;

			bool CanRunInBackground() const;
			bool ToggleMenu() const;

			bool OnStartEmulation();
			void OnStopEmulation();

			void OnReturnInputScreen(Rect&);
			void OnReturnOutputScreen(Rect&);

			ibool OnSysKeyDown        (Param&);
			ibool OnCommand           (Param&);
			ibool OnEnable            (Param&);
			ibool OnEnterSizeMoveMenu (Param&);
			ibool OnActivate          (Param&);
			ibool OnSysCommand        (Param&);
			ibool OnNclButton         (Param&);
			ibool OnNcrButton         (Param&);
			ibool OnPowerBroadCast    (Param&);
			ibool OnCommandResume     (Param&);

			void OnCmdViewSwitchScreen  (uint);
			void OnCmdViewShowOnTop     (uint);
			void OnCmdViewShowMenu      (uint);

			void OnEmuEvent(Managers::Emulator::Event,Managers::Emulator::Data);
			void OnAppEvent(Application::Instance::Event,const void*);

			const Managers::Preferences& preferences;
			MainWindow window;
			Managers::Video video;
			Managers::Sound sound;
			Managers::Input input;
			Managers::FrameClock frameClock;

		public:

			Custom& Get()
			{
				return window;
			}

			const Custom& Get() const
			{
				return window;
			}
		};
	}
}

#endif
