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

#include "resource/resource.h"
#include "NstResourceString.hpp"
#include "NstResourceIcon.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManager.hpp"
#include "NstManagerPreferences.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowMain.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <Pbt.h>

namespace Nestopia
{
	namespace Window
	{
		const wchar_t Main::MainWindow::name[] = L"Nestopia";

		Main::MainWindow::MainWindow(const Configuration& cfg,const Menu& menu)
		: menu(false), maximized(false)
		{
			Context context;

			context.className   = Application::Instance::GetClassName();
			context.classStyle  = CLASS_STYLE;
			context.hBackground = reinterpret_cast<HBRUSH>(::GetStockObject( NULL_BRUSH ));
			context.hIcon       = Resource::Icon( Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? IDI_PAD : IDI_PAD_J );
			context.windowName  = name;
			context.winStyle    = WIN_STYLE;
			context.exStyle     = WIN_EXSTYLE;

			{
				Configuration::ConstSection show( cfg["view"]["show"] );

				if (show["on-top"].Yes())
				{
					context.exStyle |= WS_EX_TOPMOST;
					menu[IDM_VIEW_ON_TOP].Check();
				}

				if (!show["window-menu"].No())
					context.hMenu = menu.GetHandle();
			}

			Create( context );
		}

		Main::Main
		(
			Managers::Emulator& e,
			const Configuration& cfg,
			Menu& m,
			const Managers::Paths& paths,
			const Managers::Preferences& p,
			const int cmdShow
		)
		:
		Manager     ( e, m, this, &Main::OnEmuEvent, &Main::OnAppEvent ),
		preferences ( p ),
		window      ( cfg, m ),
		video       ( window, m, e, paths, cfg ),
		sound       ( window, m, e, paths, p, cfg ),
		input       ( window, m, e, cfg, Managers::Input::Screening(this,&Main::OnReturnInputScreen), Managers::Input::Screening(this,&Main::OnReturnOutputScreen) ),
		frameClock  ( m, e, cfg, video.ModernGPU() )
		{
			menu.Hook( window );
			emulator.Hook( this, &Main::OnStartEmulation, &Main::OnStopEmulation );

			static const MsgHandler::Entry<Main> messages[] =
			{
				{ WM_SYSKEYDOWN,                                &Main::OnSysKeyDown        },
				{ WM_SYSCOMMAND,                                &Main::OnSysCommand        },
				{ WM_ENTERSIZEMOVE,                             &Main::OnEnterSizeMoveMenu },
				{ WM_ENTERMENULOOP,                             &Main::OnEnterSizeMoveMenu },
				{ WM_ENABLE,                                    &Main::OnEnable            },
				{ WM_ACTIVATE,                                  &Main::OnActivate          },
				{ WM_NCLBUTTONDOWN,                             &Main::OnNclButton         },
				{ WM_NCRBUTTONDOWN,                             &Main::OnNcrButton         },
				{ WM_POWERBROADCAST,                            &Main::OnPowerBroadCast    },
				{ Application::Instance::WM_NST_COMMAND_RESUME, &Main::OnCommandResume     }
			};

			window.Messages().Add( this, messages );

			static const Menu::CmdHandler::Entry<Main> commands[] =
			{
				{ IDM_VIEW_SWITCH_SCREEN, &Main::OnCmdViewSwitchScreen },
				{ IDM_VIEW_MENU,          &Main::OnCmdViewShowMenu     },
				{ IDM_VIEW_ON_TOP,        &Main::OnCmdViewShowOnTop    }
			};

			menu.Commands().Add( this, commands );

			menu[IDM_VIEW_MENU].Check();
			menu[IDM_VIEW_SWITCH_SCREEN].Text() << Resource::String(IDS_MENU_FULLSCREEN);

			if (preferences[Managers::Preferences::SAVE_WINDOWPOS])
			{
				Configuration::ConstSection pos( cfg["view"]["window-position"] );

				const Rect rect
				(
					pos[ "left"   ].Int(),
					pos[ "top"    ].Int(),
					pos[ "right"  ].Int(),
					pos[ "bottom" ].Int()
				);

				const Point mode( video.GetDisplayMode() );

				const bool winpos =
				(
					rect.left < rect.right && rect.top < rect.bottom &&
					rect.left < mode.x && rect.top < mode.y &&
					rect.Width() < mode.x * 2 && rect.Height() < mode.y * 2
				);

				if (winpos)
					window.SetPlacement( rect );
			}

			if (preferences[Managers::Preferences::START_IN_FULLSCREEN])
				window.PostCommand( IDM_VIEW_SWITCH_SCREEN );
			else
				::ShowWindow( window, cmdShow );
		}

		Main::~Main()
		{
			menu.Commands().Remove( this );
			window.Messages().Remove( this );
			emulator.Unhook();
			menu.Unhook();
		}

		inline bool Main::Fullscreen() const
		{
			return video.Fullscreen();
		}

		inline bool Main::Windowed() const
		{
			return video.Windowed();
		}

		void Main::Save(Configuration& cfg) const
		{
			{
				Configuration::Section show( cfg["view"]["show"] );

				show[ "on-top"      ].YesNo() = menu[IDM_VIEW_ON_TOP].Checked();
				show[ "window-menu" ].YesNo() = (Windowed() ? menu.Visible() : window.menu);
			}

			Rect rect( video.Fullscreen() ? window.rect : window.GetPlacement() );
			rect.Position() += Point(rect.left < 0 ? -rect.left : 0, rect.top < 0 ? -rect.top : 0 );

			if (preferences[Managers::Preferences::SAVE_WINDOWPOS])
			{
				Configuration::Section pos( cfg["view"]["window-position"] );

				pos[ "left"   ].Int() = rect.left;
				pos[ "top"    ].Int() = rect.top;
				pos[ "right"  ].Int() = rect.right;
				pos[ "bottom" ].Int() = rect.bottom;
			}

			video.Save( cfg, rect );
			sound.Save( cfg );
			input.Save( cfg );
			frameClock.Save( cfg );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		int Main::Run()
		{
			MSG msg;

			while (const BOOL ret = ::GetMessage( &msg, NULL, 0, 0 ))
			{
				if (ret == -1)
					return EXIT_FAILURE;

				if (!Dialog::ProcessMessage( msg ) && !Menu::TransAccelerator( msg ))
				{
					::TranslateMessage( &msg );
					::DispatchMessage( &msg );
				}

				if (!emulator.Resume())
					continue;

				for (;;)
				{
					if (video.MustClearFrameScreen() && emulator.IsGame())
						video.ClearScreen();

					while (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
					{
						if (!Dialog::ProcessMessage( msg ) && !Menu::TransAccelerator( msg ))
						{
							::TranslateMessage( &msg );
							::DispatchMessage( &msg );
						}

						if (msg.message == WM_QUIT)
							return msg.wParam;
					}

					if (emulator.Resume())
					{
						if (emulator.IsGame())
						{
							for (uint skips=frameClock.GameSynchronize( video.ThrottleRequired(frameClock.GetRefreshRate()) ); skips; --skips)
								emulator.Execute( NULL, sound.GetOutput(), input.GetOutput() );

							emulator.Execute( video.GetOutput(), sound.GetOutput(), input.GetOutput() );
							video.PresentScreen();
							input.Poll();
						}
						else
						{
							NST_ASSERT( emulator.IsNsf() );
							emulator.Execute( NULL, sound.GetOutput(), NULL );
							frameClock.SoundSynchronize();
						}
					}
					else
					{
						break;
					}
				}
			}

			return msg.wParam;
		}

		void Main::OnReturnInputScreen(Rect& rect)
		{
			rect = video.GetInputRect();
		}

		void Main::OnReturnOutputScreen(Rect& rect)
		{
			rect = video.GetScreenRect();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		uint Main::GetMaxMessageLength() const
		{
			return video.GetMaxMessageLength();
		}

		bool Main::OnStartEmulation()
		{
			if (window.Enabled() && (CanRunInBackground() || (window.Active() && !window.Minimized() && (Windowed() || !menu.Visible()))))
			{
				int priority;

				switch (preferences.GetPriority())
				{
					case Managers::Preferences::PRIORITY_HIGH:

						if (emulator.IsGame() && !CanRunInBackground())
						{
							priority = THREAD_PRIORITY_HIGHEST;
							break;
						}

					case Managers::Preferences::PRIORITY_ABOVE_NORMAL:

						priority = THREAD_PRIORITY_ABOVE_NORMAL;
						break;

					default:

						priority = THREAD_PRIORITY_NORMAL;
						break;
				}

				::SetThreadPriority( ::GetCurrentThread(), priority );

				frameClock.StartEmulation();
				video.StartEmulation();
				input.StartEmulation();
				sound.StartEmulation();

				return true;
			}

			return false;
		}

		void Main::OnStopEmulation()
		{
			::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_NORMAL );

			sound.StopEmulation();
			video.StopEmulation();
			input.StopEmulation();
			frameClock.StopEmulation();
		}

		bool Main::ToggleMenu() const
		{
			bool visible = menu.Visible();

			if (Fullscreen() || !window.Restored())
			{
				if (Fullscreen() && !visible && !CanRunInBackground())
					emulator.Stop();

				visible = menu.Toggle();
			}
			else
			{
				Point size;

				if (visible)
				{
					size.y = menu.Height();
					menu.Hide();
					window.Size() -= size;
					visible = false;
				}
				else
				{
					menu.Show();
					size.y = menu.Height();
					window.Size() += size;
					visible = true;
				}
			}

			return visible;
		}

		bool Main::CanRunInBackground() const
		{
			if (emulator.IsNsf())
				return menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Checked();

			return preferences[Managers::Preferences::RUN_IN_BACKGROUND];
		}

		ibool Main::OnSysKeyDown(Param& param)
		{
			return (param.wParam == VK_MENU && !menu.Visible());
		}

		ibool Main::OnSysCommand(Param& param)
		{
			switch (param.wParam & 0xFFF0)
			{
				case SC_MOVE:
				case SC_SIZE:

					return Fullscreen();

				case SC_MONITORPOWER:
				case SC_SCREENSAVE:

					return Fullscreen() || emulator.IsOn();

				case SC_MAXIMIZE:

					if (Fullscreen())
						return true;

				case SC_MINIMIZE:
				case SC_RESTORE:

					if (Windowed())
						emulator.Stop();

					break;
			}

			return false;
		}

		ibool Main::OnEnterSizeMoveMenu(Param&)
		{
			emulator.Stop();
			return true;
		}

		ibool Main::OnEnable(Param& param)
		{
			if (!param.wParam)
				emulator.Stop();

			return true;
		}

		ibool Main::OnActivate(Param& param)
		{
			if (param.Activator().Entering())
			{
				if (Fullscreen() && param.Activator().Minimized())
					emulator.Stop();
			}
			else
			{
				if (!CanRunInBackground() || (Fullscreen() && param.Activator().OutsideApplication()))
					emulator.Stop();
			}

			return false;
		}

		ibool Main::OnNclButton(Param& param)
		{
			switch (param.wParam)
			{
				case HTCAPTION:
				case HTMINBUTTON:
				case HTMAXBUTTON:
				case HTCLOSE:

					emulator.Stop();
			}

			return false;
		}

		ibool Main::OnNcrButton(Param& param)
		{
			switch (param.wParam)
			{
				case HTCAPTION:
				case HTSYSMENU:
				case HTMINBUTTON:

					emulator.Stop();
			}

			return false;
		}

		ibool Main::OnPowerBroadCast(Param& param)
		{
			switch (param.wParam)
			{
				case PBT_APMQUERYSUSPEND:

					emulator.Stop();

				case PBT_APMRESUMESUSPEND:

					return true;
			}

			return false;

		}

		ibool Main::OnCommandResume(Param& param)
		{
			if (HIWORD(param.wParam) == 0 && Fullscreen() && emulator.IsOn() && menu.Visible())
				window.PostCommand( IDM_VIEW_MENU ); // Hide menu and resume emulation

			return true;
		}

		void Main::OnCmdViewSwitchScreen(uint)
		{
			Application::Instance::Waiter wait;

			emulator.Stop();

			if (Windowed())
			{
				window.menu = menu.Visible();
				window.maximized = window.Maximized();
				window.rect = window.GetPlacement();

				menu.Hide();

				menu[ IDM_VIEW_ON_TOP ].Disable();
				menu[ IDM_VIEW_SWITCH_SCREEN ].Text() << Resource::String(IDS_MENU_DESKTOP);

				window.MakeTopMost( false );
				window.MakeFullscreen();

				Application::Instance::Events::Signal( Application::Instance::EVENT_FULLSCREEN );

				video.SwitchScreen();

				if (!emulator.IsOn())
					menu.Show();
			}
			else
			{
				Application::Instance::Events::Signal( Application::Instance::EVENT_DESKTOP );

				menu.Hide();

				video.SwitchScreen();

				window.Show( false );
				window.MakeTopMost( menu[IDM_VIEW_ON_TOP].Checked() );
				window.MakeWindowed( MainWindow::WIN_STYLE, MainWindow::WIN_EXSTYLE );

				menu[ IDM_VIEW_ON_TOP ].Enable();
				menu[ IDM_VIEW_SWITCH_SCREEN ].Text() << Resource::String(IDS_MENU_FULLSCREEN);

				if (window.menu)
					menu.Show();

				if (window.maximized)
					window.Maximize();

				window.SetPlacement( window.rect );
				window.Show( true );

				Application::Instance::ShowChildWindows();
			}

			::Sleep( 500 );
		}

		void Main::OnCmdViewShowOnTop(uint)
		{
			NST_ASSERT( Windowed() );

			window.MakeTopMost( menu[IDM_VIEW_ON_TOP].ToggleCheck() );
		}

		void Main::OnCmdViewShowMenu(uint)
		{
			const bool visible = ToggleMenu();

			if (Fullscreen())
				Application::Instance::ShowChildWindows( visible );
		}

		void Main::OnEmuEvent(const Managers::Emulator::Event event,const Managers::Emulator::Data data)
		{
			switch (event)
			{
				case Managers::Emulator::EVENT_POWER_ON:
				case Managers::Emulator::EVENT_POWER_OFF:

					if (emulator.NetPlayers())
					{
						menu.ToggleModeless( event == Managers::Emulator::EVENT_POWER_ON );
					}
					else if (Fullscreen())
					{
						const bool show = (event == Managers::Emulator::EVENT_POWER_OFF);
						menu.Show( show );
						Application::Instance::ShowChildWindows( show );
					}
					break;

				case Managers::Emulator::EVENT_LOAD:
				{
					Path name;

					if (emulator.IsCart())
					{
						name = Nes::Cartridge(emulator).GetProfile()->game.title.c_str();
					}
					else if (emulator.IsNsf())
					{
						name.Import( Nes::Nsf(emulator).GetName() );
					}

					if (name.Empty())
					{
						name = emulator.GetImagePath().Target().File();
						name.Extension().Clear();
					}

					if (name.Length())
						window.Text() << (name << " - " << MainWindow::name).Ptr();

					break;
				}

				case Managers::Emulator::EVENT_UNLOAD:

					window.Text() << MainWindow::name;
					break;

				case Managers::Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_VIEW_SWITCH_SCREEN].Enable( !data );
					break;
			}
		}

		void Main::OnAppEvent(Application::Instance::Event event,const void*)
		{
			if (event == Application::Instance::EVENT_SYSTEM_BUSY)
				emulator.Stop();
		}
	}
}
