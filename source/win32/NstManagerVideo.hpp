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

#ifndef NST_MANAGER_VIDEO_H
#define NST_MANAGER_VIDEO_H

#pragma once

#include "NstWindowStatusBar.hpp"
#include "NstObjectHeap.hpp"
#include "NstDirect2D.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Video;
	}

	namespace Managers
	{
		class Paths;

		class Video : Manager
		{
		public:

			typedef Window::Point Point;
			typedef Window::Rect Rect;

			Video(Window::Custom&,Window::Menu&,Emulator&,const Paths&,const Configuration&);
			~Video();

			void StartEmulation();
			void StopEmulation();
			void SwitchScreen();
			void Save(Configuration&,const Rect&) const;
			uint GetMaxMessageLength() const;
			const Rect& GetInputRect() const;
			bool MustClearFrameScreen() const;

			Point GetDisplayMode() const;

		private:

			typedef Application::Instance Instance;
			typedef DirectX::Direct2D::Adapter::Modes::const_iterator Mode;

			enum
			{
				SCALE_TOLERANCE = 16,
				STATUSBAR_WIDTH = 11,
				SCREEN_TEXT_DURATION = 2250
			};

			bool SwitchFullscreen(Mode);
			void ToggleFps(bool);
			void UpdateScreen();
			void UpdateDialogBoxMode();
			void UpdateMenuScreenSizes(const Point) const;
			void UpdateFieldMergingState() const;
			void ResetScreenRect(uint);
			uint CalculateWindowScale(const Rect&) const;
			uint CalculateWindowScale() const;
			uint CalculateFullscreenScale() const;
			bool WindowMatched(const Rect&) const;
			bool WindowMatched() const;

			NST_NO_INLINE void RepairScreen();

			ibool OnPaint         (Window::Param&);
			ibool OnNcPaint       (Window::Param&);
			ibool OnEraseBkGnd    (Window::Param&);
			ibool OnDisplayChange (Window::Param&);
			void  OnEnterSizeMove (Window::Param&);
			void  OnExitSizeMove  (Window::Param&);

			void OnCmdFileScreenShot          (uint);
			void OnCmdMachineUnlimitedSprites (uint);
			void OnCmdViewScreenSize          (uint);
			void OnCmdViewTvAspect            (uint);
			void OnCmdViewFps                 (uint);
			void OnCmdViewStatusBar           (uint);
			void OnCmdOptionsVideo            (uint);

			void OnEmuEvent (Emulator::Event,Emulator::Data);
			void OnAppEvent (Instance::Event,const void*);
			void OnMenuScreenSizes  (const Window::Menu::PopupHandler::Param&);
			void OnMenuUnlimSprites (const Window::Menu::PopupHandler::Param&);
			void OnScreenText (const GenericString&,uint);
			uint OnTimerFps();
			uint OnTimerText();

			struct Callbacks;

			struct Fps
			{
				inline Fps();

				enum
				{
					UPDATE_INTERVAL = 2000
				};

				uint frame;
			};

			struct Nsf
			{
				inline Nsf();

				void Load (Nes::Nsf);
				void Update (Nes::Nsf);

				HeapString text;
				uint songTextOffset;
			};

			Window::Custom& window;
			Fps fps;
			Window::StatusBar statusBar;
			DirectX::Direct2D direct2d;
			Object::Heap<Window::Video> dialog;
			Nes::Video::Output nesOutput;
			Nsf nsf;
			bool sizingMoving;
			const Paths& paths;
			const uint childWindowSwitchCount;

		public:

			Nes::Video::Output* GetOutput()
			{
				return direct2d.ValidScreen() ? &nesOutput : NULL;
			}

			bool Windowed() const
			{
				return direct2d.Windowed();
			}

			bool Fullscreen() const
			{
				return !Windowed();
			}

			bool ThrottleRequired(uint speed) const
			{
				return direct2d.ThrottleRequired( speed );
			}

			const Rect GetScreenRect() const
			{
				Rect rect( direct2d.GetScreenRect() );

				if (direct2d.Windowed())
					rect.ScreenTransform( window );

				return rect;
			}

			void ClearScreen()
			{
				if (!direct2d.ClearScreen())
					RepairScreen();
			}

			void PresentScreen()
			{
				if (!direct2d.PresentScreen())
					RepairScreen();
			}

			bool ModernGPU() const
			{
				return direct2d.ModernGPU();
			}
		};
	}
}

#endif
