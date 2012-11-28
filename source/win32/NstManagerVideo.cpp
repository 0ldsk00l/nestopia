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

#include "NstManagerPaths.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogVideo.hpp"
#include "NstManagerVideo.hpp"
#include "NstIoScreen.hpp"

namespace Nestopia
{
	namespace Managers
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		struct Video::Callbacks
		{
			static bool NST_CALLBACK ScreenLock(Nes::Video::UserData data,Nes::Video::Output& output)
			{
				NST_ASSERT( data );

				return static_cast<DirectX::Direct2D*>(data)->LockScreen( output.pixels, output.pitch );
			}

			static void NST_CALLBACK ScreenUnlock(Nes::Video::UserData data,Nes::Video::Output&)
			{
				NST_ASSERT( data );

				static_cast<DirectX::Direct2D*>(data)->UnlockScreen();
				static_cast<DirectX::Direct2D*>(data)->RenderScreen( DirectX::Direct2D::RENDER_PICTURE|DirectX::Direct2D::RENDER_FPS|DirectX::Direct2D::RENDER_MSG );
			}
		};

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline Video::Fps::Fps()
		: frame(0) {}

		inline Video::Nsf::Nsf()
		: songTextOffset(0) {}

		void Video::Nsf::Load(const Nes::Nsf emulator)
		{
			struct Info
			{
				static cstring GoodName(cstring text)
				{
					if
					(
						text && *text &&
						std::strcmp( text, "<?>"   ) &&
						std::strcmp( text, "< ? >" ) &&
						std::strcmp( text, "?"     )
					)
						return text;

					return NULL;
				}

				static cstring GetMode(Nes::Nsf::TuneMode mode)
				{
					return
					(
						mode == Nes::Nsf::TUNE_MODE_PAL  ? " (PAL)" :
						mode == Nes::Nsf::TUNE_MODE_NTSC ? " (NTSC)" :
                                                           " (NTSC/PAL)"
					);
				}
			};

			text.Clear();

			if (cstring name = Info::GoodName( emulator.GetName() ))
				text.Import( name );
			else
				text = "noname";

			text << Info::GetMode( emulator.GetMode() );

			cstring const artist = Info::GoodName( emulator.GetArtist() );
			cstring const copyright = Info::GoodName( emulator.GetCopyright() );

			if (artist)
				(text << "\r\n").Import( artist );

			if (copyright)
				(text << "\r\n").Import( copyright );

			if (const uint chips = emulator.GetChips())
			{
				text << (copyright || artist ? ", " : "\r\n");

				if ( chips & Nes::Nsf::CHIP_MMC5 ) text << "MMC5 ";
				if ( chips & Nes::Nsf::CHIP_FDS  ) text << "FDS ";
				if ( chips & Nes::Nsf::CHIP_VRC6 ) text << "VRC6 ";
				if ( chips & Nes::Nsf::CHIP_VRC7 ) text << "VRC7 ";
				if ( chips & Nes::Nsf::CHIP_N163 ) text << "N163 ";
				if ( chips & Nes::Nsf::CHIP_S5B  ) text << "Sunsoft5B ";

				text << ((chips & (chips-1)) ? "chips" : "chip");
			}

			text << "\r\nSong: ";
			songTextOffset = text.Length();

			Update( emulator );
		}

		void Video::Nsf::Update(const Nes::Nsf emulator)
		{
			text.ShrinkTo( songTextOffset );
			text << (emulator.GetCurrentSong() + 1) << '/' << emulator.GetNumSongs();
		}

		Video::Video
		(
			Window::Custom& w,
			Window::Menu& m,
			Emulator& e,
			const Paths& p,
			const Configuration& cfg
		)
		:
		Manager                ( e, m, this, &Video::OnEmuEvent, &Video::OnAppEvent ),
		window                 ( w ),
		statusBar              ( w, STATUSBAR_WIDTH ),
		direct2d               ( w ),
		dialog                 ( new Window::Video( e, direct2d.GetAdapters(), p, cfg ) ),
		sizingMoving           ( false ),
		paths                  ( p ),
		childWindowSwitchCount ( Application::Instance::NumChildWindows() + 1 )
		{
			static const Window::MsgHandler::Entry<Video> messages[] =
			{
				{ WM_PAINT,          &Video::OnPaint         },
				{ WM_NCPAINT,        &Video::OnNcPaint       },
				{ WM_ERASEBKGND,     &Video::OnEraseBkGnd    },
				{ WM_DISPLAYCHANGE,  &Video::OnDisplayChange }
			};

			static const Window::MsgHandler::HookEntry<Video> hooks[] =
			{
				{ WM_ENTERSIZEMOVE, &Video::OnEnterSizeMove },
				{ WM_EXITSIZEMOVE,  &Video::OnExitSizeMove  }
			};

			window.Messages().Add( this, messages, hooks );

			static const Window::Menu::CmdHandler::Entry<Video> commands[] =
			{
				{ IDM_OPTIONS_VIDEO,                    &Video::OnCmdOptionsVideo            },
				{ IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES, &Video::OnCmdMachineUnlimitedSprites },
				{ IDM_FILE_SAVE_SCREENSHOT,             &Video::OnCmdFileScreenShot          },
				{ IDM_VIEW_WINDOWSIZE_1X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_2X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_3X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_4X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_5X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_6X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_7X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_8X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_9X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_MAX,              &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_TVASPECT,         &Video::OnCmdViewTvAspect            },
				{ IDM_VIEW_STATUSBAR,                   &Video::OnCmdViewStatusBar           },
				{ IDM_VIEW_FPS,                         &Video::OnCmdViewFps                 }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Video> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_VIEW,IDM_POS_VIEW_SCREENSIZE>::ID, &Video::OnMenuScreenSizes },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_OPTIONS>::ID, &Video::OnMenuUnlimSprites }
			};

			menu.Popups().Add( this, popups );

			Io::Screen::SetCallback( this, &Video::OnScreenText );

			Nes::Video::Output::lockCallback.Set( &Callbacks::ScreenLock, &direct2d );
			Nes::Video::Output::unlockCallback.Set( &Callbacks::ScreenUnlock, &direct2d );

			direct2d.EnableAutoFrequency( dialog->UseAutoFrequency() );
			direct2d.SelectAdapter( dialog->GetAdapter() );

			menu[IDM_FILE_SAVE_SCREENSHOT].Disable();

			if (cfg["machine"]["no-sprite-limit"].Yes())
				Nes::Video(emulator).EnableUnlimSprites( true );

			{
				Configuration::ConstSection view( cfg["view"] );

				if (view["show"]["status-bar"].No())
				{
					menu[IDM_VIEW_FPS].Disable();
				}
				else
				{
					menu[IDM_VIEW_STATUSBAR].Check();
					statusBar.Enable();
				}

				if (view["show"]["fps"].Yes())
					menu[IDM_VIEW_FPS].Check();

				const uint size = view["size"]["window"].Int();
				ResetScreenRect( size >= 2 && size <= 9 ? size-1 : 0 );
			}

			menu[IDM_VIEW_WINDOWSIZE_TVASPECT].Check( dialog->TvAspect() );

			UpdateMenuScreenSizes( GetDisplayMode() );
		}

		Video::~Video()
		{
			Io::Screen::UnsetCallback();

			Nes::Video::Output::lockCallback.Unset();
			Nes::Video::Output::unlockCallback.Unset();

			window.Messages().RemoveAll( this );
			window.StopTimer( this, &Video::OnTimerText );
		}

		void Video::Save(Configuration& cfg,const Rect& client) const
		{
			cfg[ "machine" ][ "no-sprite-limit" ].YesNo() = Nes::Video(emulator).AreUnlimSpritesEnabled();

			{
				Configuration::Section view( cfg["view"] );

				const uint size = Point(dialog->GetNesRect()).ScaleToFit( client, Point::SCALE_NEAREST );

				view[ "size" ][ "window"     ].Int() = (size <= 8 ? size+1 : 1);
				view[ "show" ][ "status-bar" ].YesNo() = menu[ IDM_VIEW_STATUSBAR ].Checked();
				view[ "show" ][ "fps"        ].YesNo() = menu[ IDM_VIEW_FPS       ].Checked();
			}

			dialog->Save( cfg );
		}

		Video::Point Video::GetDisplayMode() const
		{
			return Fullscreen() ? direct2d.GetFullscreenDisplayMode() : Point( ::GetSystemMetrics( SM_CXSCREEN ), ::GetSystemMetrics( SM_CYSCREEN ) );
		}

		uint Video::GetMaxMessageLength() const
		{
			return Fullscreen() ? direct2d.GetMaxMessageLength() : statusBar.Enabled() ? statusBar.GetMaxMessageLength() : 0;
		}

		/**
		 * Is called when the Options Video dialog has been closed. Updates the window according to the video settings. For example
		 * changing video adapter, resolution etc.
		 */
		void Video::OnCmdOptionsVideo(uint)
		{
			const Rect oldRect( dialog->GetNesRect() );

			dialog->Open();

			direct2d.EnableAutoFrequency( dialog->UseAutoFrequency() );
			direct2d.SelectAdapter( dialog->GetAdapter() );

			if (Windowed() || !SwitchFullscreen( dialog->GetMode() ))
			{
				if (oldRect != dialog->GetNesRect())
				{
					UpdateMenuScreenSizes( GetDisplayMode() );

					if (Windowed() && window.Restored())
						ResetScreenRect( CalculateWindowScale() );
				}

				window.Redraw();
			}
		}

		void Video::OnCmdViewScreenSize(uint id)
		{
			ResetScreenRect( id == IDM_VIEW_WINDOWSIZE_MAX ? Window::Video::SCREEN_STRETCHED : id - IDM_VIEW_WINDOWSIZE_1X );
			Resume();
		}

		void Video::OnCmdViewTvAspect(uint)
		{
			menu[IDM_VIEW_WINDOWSIZE_TVASPECT].Check( dialog->ToggleTvAspect() );

			if (Windowed())
				window.PostCommand( IDM_VIEW_WINDOWSIZE_1X + CalculateWindowScale() );
			else
				window.Redraw();

			Resume();
		}

		void Video::OnCmdViewStatusBar(uint)
		{
			NST_VERIFY( Windowed() );

			if (Windowed())
			{
				const bool enable = menu[IDM_VIEW_STATUSBAR].ToggleCheck();
				menu[IDM_VIEW_FPS].Enable( enable );

				Point size;

				if (enable)
				{
					statusBar.Enable( true, false );
					size.y = statusBar.Height();
				}
				else
				{
					size.y = statusBar.Height();
					statusBar.Disable();
				}

				if (window.Restored())
				{
					if (enable)
						window.Size() += size;
					else
						window.Size() -= size;
				}
				else if (!enable)
				{
					window.Redraw();
				}

				if (enable)
					statusBar.Show();

				if (emulator.Running() && emulator.IsGame() && menu[IDM_VIEW_FPS].Checked())
					ToggleFps( enable );
			}
		}

		void Video::OnCmdViewFps(uint)
		{
			NST_VERIFY( Fullscreen() || statusBar.Enabled() );

			if (Fullscreen() || statusBar.Enabled())
			{
				const bool enable = menu[IDM_VIEW_FPS].ToggleCheck();

				if (emulator.Running() && emulator.IsGame())
					ToggleFps( enable );
			}
		}

		void Video::OnCmdMachineUnlimitedSprites(uint)
		{
			const bool enable = !Nes::Video(emulator).AreUnlimSpritesEnabled();

			if (NES_SUCCEEDED(Nes::Video(emulator).EnableUnlimSprites( enable )))
				Io::Screen() << Resource::String(enable ? IDS_SCREEN_NOSPRITELIMIT_ON : IDS_SCREEN_NOSPRITELIMIT_OFF );
		}

		void Video::OnCmdFileScreenShot(uint)
		{
			if (emulator.IsGameOn())
			{
				const Path path( paths.GetScreenShotPath() );
				const DirectX::Direct2D::ScreenShotResult result = direct2d.SaveScreenShot( path.Ptr(), path.Extension().Id() );

				if (result == DirectX::Direct2D::SCREENSHOT_OK)
				{
					const uint length = GetMaxMessageLength();

					if (length > 22)
						Io::Screen() << Resource::String(IDS_SCREEN_SCREENSHOT_SAVED_TO).Invoke( Path::Compact( path, length - 20 ) );
				}
				else
				{
					const uint ids = (DirectX::Direct2D::SCREENSHOT_UNSUPPORTED ? IDS_SCREENSHOT_UNSUPPORTED_FORMAT : IDS_SCREENSHOT_SAVE_FAILED);

					if (emulator.NetPlayers())
						Io::Screen() << Resource::String( ids );
					else
						Window::User::Fail( ids );
				}
			}
		}

		uint Video::CalculateWindowScale(const Rect& rect) const
		{
			return Point(rect.Size()).ScaleToFit( window.PictureCoordinates(), Point::SCALE_NEAREST );
		}

		uint Video::CalculateWindowScale() const
		{
			return CalculateWindowScale( dialog->GetNesRect() );
		}

		uint Video::CalculateFullscreenScale() const
		{
			const Point screen( GetDisplayMode() );
			return Point(dialog->GetNesRect()).ScaleToFit( screen + (screen / SCALE_TOLERANCE), Point::SCALE_BELOW, dialog->GetFullscreenScale() );
		}

		void Video::OnMenuScreenSizes(const Window::Menu::PopupHandler::Param& param)
		{
			uint scale;
			bool check;

			if (param.show)
			{
				if (Fullscreen())
				{
					if (dialog->GetFullscreenScale() == Window::Video::SCREEN_STRETCHED)
						scale = Window::Video::SCREEN_STRETCHED;
					else
						scale = CalculateFullscreenScale();
				}
				else
				{
					if (window.Maximized())
					{
						scale = Window::Video::SCREEN_STRETCHED;
					}
					else if (WindowMatched())
					{
						scale = CalculateWindowScale();
					}
					else
					{
						scale = 0xBEDBABE;
					}
				}

				if (scale == Window::Video::SCREEN_STRETCHED)
				{
					scale = IDM_VIEW_WINDOWSIZE_MAX;
					check = true;
				}
				else if (scale < IDM_VIEW_WINDOWSIZE_MAX-IDM_VIEW_WINDOWSIZE_1X)
				{
					scale += IDM_VIEW_WINDOWSIZE_1X;
					check = true;
				}
				else
				{
					scale = IDM_VIEW_WINDOWSIZE_1X;
					check = false;
				}
			}
			else
			{
				scale = IDM_VIEW_WINDOWSIZE_1X;
				check = false;
			}

			param.menu[scale].Check( IDM_VIEW_WINDOWSIZE_1X, IDM_VIEW_WINDOWSIZE_MAX, check );
		}

		void Video::OnMenuUnlimSprites(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].Check( param.show && Nes::Video(emulator).AreUnlimSpritesEnabled() );
		}

		bool Video::WindowMatched(const Rect& nesRect) const
		{
			const Point output( window.PictureCoordinates() );
			const Point input( nesRect );

			return
			(
				output.x && (output.x % input.x) == 0 &&
				output.y && (output.y % input.y) == 0
			);
		}

		bool Video::WindowMatched() const
		{
			return WindowMatched( dialog->GetNesRect() );
		}

		void Video::ResetScreenRect(const uint scale)
		{
			if (Fullscreen())
			{
				if (dialog->GetFullscreenScale() != scale)
				{
					dialog->SetFullscreenScale( scale );
					window.Redraw();
				}
			}
			else if (scale == Window::Video::SCREEN_STRETCHED)
			{
				window.Maximize();
			}
			else
			{
				if (!window.Restored())
					window.Restore();

				Point size( dialog->GetNesRect() );
				size.ScaleToFit( GetDisplayMode(), Point::SCALE_BELOW, scale );

				window.Size() = size + window.NonClientCoordinates();

				if (window.PictureCoordinates() != size)
					window.Size() = size + window.NonClientCoordinates();
			}
		}

		void Video::UpdateMenuScreenSizes(Point screen) const
		{
			for (uint i=IDM_VIEW_WINDOWSIZE_1X; i < IDM_VIEW_WINDOWSIZE_MAX; ++i)
				menu[i].Remove();

			const Point original( dialog->GetNesRect() );
			Point nes( original );

			if (Fullscreen())
				screen += screen / SCALE_TOLERANCE;

			for (uint i=0; i < (IDM_VIEW_WINDOWSIZE_MAX-IDM_VIEW_WINDOWSIZE_1X); ++i)
			{
				menu.Insert( menu[IDM_VIEW_WINDOWSIZE_MAX], IDM_VIEW_WINDOWSIZE_1X + i, Resource::String(IDS_MENU_X).Invoke( wchar_t('1'+i) ) );
				nes = original * (i+2);

				if (nes.x > screen.x || nes.y > screen.y)
					break;
			}
		}

		ibool Video::OnDisplayChange(Window::Param& param)
		{
			UpdateMenuScreenSizes( Point(LOWORD(param.lParam),HIWORD(param.lParam)) );
			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool Video::MustClearFrameScreen() const
		{
			return Fullscreen() && (dialog->GetFullscreenScale() != Window::Video::SCREEN_STRETCHED || dialog->TvAspect());
		}

		ibool Video::OnPaint(Window::Param&)
		{
			if (emulator.IsOn())
			{
				if (!sizingMoving)
					UpdateScreen();

				if (emulator.IsGame())
				{
					if (MustClearFrameScreen())
						ClearScreen();

					if (!sizingMoving && emulator.Running())
					{
						window.Redraw( false );
						return true;
					}

					Nes::Video( emulator ).Blit( nesOutput );
				}
				else
				{
					ClearScreen();
					direct2d.RenderScreen( DirectX::Direct2D::RENDER_MSG|DirectX::Direct2D::RENDER_NFO );
				}
			}
			else
			{
				ClearScreen();
				direct2d.RenderScreen( DirectX::Direct2D::RENDER_MSG );
			}

			window.Redraw( false );
			PresentScreen();

			return true;
		}

		void Video::UpdateDialogBoxMode()
		{
			direct2d.EnableDialogBoxMode
			(
				!emulator.IsOn() ||
				emulator.IsNsf() ||
				menu.Visible()
			);
		}

		ibool Video::OnNcPaint(Window::Param&)
		{
			UpdateDialogBoxMode();
			return false;
		}

		ibool Video::OnEraseBkGnd(Window::Param& param)
		{
			param.lResult = true;
			return true;
		}

		void Video::OnEnterSizeMove(Window::Param&)
		{
			sizingMoving = Windowed();
		}

		void Video::OnExitSizeMove(Window::Param&)
		{
			sizingMoving = false;
			window.Redraw();
		}

		uint Video::OnTimerText()
		{
			if (Fullscreen())
			{
				direct2d.ClearMsg();
				window.Redraw();
			}
			else if (statusBar.Enabled())
			{
				statusBar.Text(Window::StatusBar::FIRST_FIELD).Clear();
			}

			return false;
		}

		uint Video::OnTimerFps()
		{
			if (emulator.IsGameOn())
			{
				class FpsString : HeapString
				{
					const uint offset;

				public:

					FpsString()
					:
					HeapString (Resource::String(IDS_TEXT_FPS) << ": xxx.x"),
					offset     (Length() - 5)
					{
					}

					void Update(uint fps)
					{
						const char dec[] = {'.',fps % (Fps::UPDATE_INTERVAL/1000) ? '5' : '0','\0'};
						fps /= (Fps::UPDATE_INTERVAL/1000);

						(*this)(offset) = NST_MIN(fps,999);
						Append( dec, 2 );
					}

					GenericString Number() const
					{
						return (*this)(offset);
					}

					wcstring Full() const
					{
						return Ptr();
					}
				};

				static FpsString fpsString;

				const uint prev = fps.frame;
				fps.frame = emulator.Frame();

				fpsString.Update( fps.frame - prev );

				if (Fullscreen())
				{
					direct2d.DrawFps( fpsString.Number() );
					return true;
				}
				else if (statusBar.Enabled())
				{
					statusBar.Text(Window::StatusBar::SECOND_FIELD) << fpsString.Full();
					return true;
				}
			}

			return false;
		}

		void Video::OnScreenText(const GenericString& text,const uint time)
		{
			if (Fullscreen())
			{
				direct2d.DrawMsg( text );
				window.Redraw();
			}
			else if (statusBar.Enabled())
			{
				statusBar.Text(Window::StatusBar::FIRST_FIELD) << text.Ptr();
			}
			else
			{
				return;
			}

			window.StartTimer( this, &Video::OnTimerText, time ? time : SCREEN_TEXT_DURATION );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Video::RepairScreen()
		{
			if (direct2d.Repair())
			{
				if (emulator.IsOn())
					UpdateScreen();

				direct2d.ClearScreen();
			}
		}

		/**
		 * Switches to a fullscreen mode.
		 * 
		 * @param mode The video mode to set to fullscreen.
		 * @return True if switched to fullscreen. False if the mode is already set in fullscreen.
		 */
		bool Video::SwitchFullscreen(Mode mode)
		{
			const bool prevFullscreen = Fullscreen();
			const bool toggleMenu = menu.Visible() && direct2d.CanSwitchFullscreen( mode );

			if (toggleMenu)
				menu.Hide();

			const bool switched = direct2d.SwitchFullscreen( mode );

			if (switched && prevFullscreen && dialog->GetFullscreenScale() != Window::Video::SCREEN_STRETCHED)
				dialog->SetFullscreenScale( Window::Video::SCREEN_MATCHED );

			if (toggleMenu)
				menu.Show();

			window.Redraw();

			return switched;
		}

		/**
		 * Switches between window and full screen mode.
		 */
		void Video::SwitchScreen()
		{
			menu[IDM_VIEW_STATUSBAR].Enable( Fullscreen() );

			if (Windowed())
			{
				if (statusBar.Enabled())
					statusBar.Disable();
				else
					menu[IDM_VIEW_FPS].Enable();

				SwitchFullscreen( dialog->GetMode() );
			}
			else
			{
				direct2d.SwitchWindowed();

				if (menu[IDM_VIEW_STATUSBAR].Checked())
					statusBar.Enable();
				else
					menu[IDM_VIEW_FPS].Disable();
			}
		}

		void Video::OnAppEvent(Instance::Event event,const void* param)
		{
			if (Fullscreen())
			{
				switch (event)
				{
					case Instance::EVENT_WINDOW_CREATE:

						if (Instance::NumChildWindows() == childWindowSwitchCount)
						{
							const Instance::Events::WindowCreateParam& info =
							(
								*static_cast<const Instance::Events::WindowCreateParam*>(param)
							);

							const Point mode( GetDisplayMode() );

							if
							(
								(mode.x < Window::Video::MIN_DIALOG_WIDTH || mode.y < Window::Video::MIN_DIALOG_HEIGHT) &&
								(mode.x < int(info.x) || mode.y < int(info.y))
							)
								SwitchFullscreen( dialog->GetDialogMode() );
						}

						menu.Show();
						break;

					case Instance::EVENT_WINDOW_DESTROY:

						if (Instance::NumChildWindows() == childWindowSwitchCount)
						{
							SwitchFullscreen( dialog->GetMode() );	//switches to fullscreen if necessary
							UpdateDialogBoxMode();
						}
						break;
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		const Video::Rect& Video::GetInputRect() const
		{
			return dialog->GetInputRect();
		}

		void Video::UpdateScreen()
		{
			NST_ASSERT( emulator.IsOn() );

			if (emulator.IsGame())
			{
				Rect picture;

				if (Windowed())
				{
					picture = window.PictureCoordinates();
				}
				else
				{
					const Point screen( GetDisplayMode() );

					if (dialog->GetFullscreenScale() == Window::Video::SCREEN_STRETCHED && !dialog->TvAspect())
					{
						picture = screen;
					}
					else
					{
						Point nesPoint( dialog->GetNesRect() );
						nesPoint.ScaleToFit( screen + (screen / SCALE_TOLERANCE), Point::SCALE_BELOW, dialog->GetFullscreenScale() );
						picture = nesPoint;

						if (dialog->GetFullscreenScale() == Window::Video::SCREEN_STRETCHED)
						{
							picture.right += screen.y - picture.bottom;
							picture.top = 0;
							picture.bottom = screen.y;
						}

						picture.Center() = screen.Center();

						if (nesPoint.x > screen.x + (screen.x / SCALE_TOLERANCE))
						{
							picture.left = 0;
							picture.right = screen.x;
						}

						if (nesPoint.y > screen.y + (screen.y / SCALE_TOLERANCE))
						{
							picture.top = 0;
							picture.bottom = screen.y;
						}
					}
				}

				Nes::Video::RenderState renderState;

				renderState.bits.count = direct2d.GetBitsPerPixel();
				NST_ASSERT( renderState.bits.count == 16 || renderState.bits.count == 32 );

				direct2d.GetBitMask( renderState.bits.mask.r, renderState.bits.mask.g, renderState.bits.mask.b );
				NST_ASSERT( renderState.bits.mask.r && renderState.bits.mask.g && renderState.bits.mask.b );

				const Rect nesScreen( dialog->GetRenderState( renderState, picture.Size() ) );
				NST_ASSERT( direct2d.GetAdapter().maxScreenSize.x >= renderState.width && direct2d.GetAdapter().maxScreenSize.y >= renderState.height );

				Nes::Video(emulator).SetRenderState( renderState );

				if (Windowed())
				{
					direct2d.UpdateWindowView
					(
						Point(renderState.width,renderState.height),
						nesScreen,
						dialog->GetScanlines(),
						dialog->GetScreenCurvature(),
						dialog->GetTextureFilter(),
						dialog->PutTextureInVideoMemory()
					);
				}
				else
				{
					direct2d.UpdateFullscreenView
					(
						picture,
						Point(renderState.width,renderState.height),
						nesScreen,
						dialog->GetScanlines(),
						dialog->GetScreenCurvature(),
						dialog->GetTextureFilter(),
						dialog->PutTextureInVideoMemory()
					);
				}

				UpdateFieldMergingState();
			}
			else if (Windowed())
			{
				direct2d.UpdateWindowView();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Video::UpdateFieldMergingState() const
		{
			Nes::Video(emulator).EnableFieldMerging
			(
				dialog->EnableFieldMerging() ||
				(dialog->UseAutoFieldMerging() && !direct2d.SmoothFrameRate())
			);
		}

		void Video::ToggleFps(const bool enable)
		{
			if (enable)
			{
				NST_ASSERT( Fullscreen() || statusBar.Enabled() );

				fps.frame = emulator.Frame();

				if (statusBar.Enabled())
					statusBar.Text(Window::StatusBar::SECOND_FIELD) << (Resource::String(IDS_TEXT_FPS) << ": ").Ptr();
				else
					direct2d.DrawFps( L"0.0" );

				window.StartTimer( this, &Video::OnTimerFps, Fps::UPDATE_INTERVAL );
			}
			else
			{
				fps.frame = 0;

				if (window.StopTimer( this, &Video::OnTimerFps ))
				{
					if (statusBar.Enabled())
						statusBar.Text(Window::StatusBar::SECOND_FIELD).Clear();
					else
						direct2d.ClearFps();
				}
			}
		}

		void Video::StartEmulation()
		{
			if (emulator.IsGame() && menu[IDM_VIEW_FPS].Checked() && (Fullscreen() || statusBar.Enabled()))
				ToggleFps( true );
		}

		void Video::StopEmulation()
		{
			ToggleFps( false );
		}

		void Video::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_BASE_SPEED:

					direct2d.UpdateFrameRate
					(
						emulator.GetBaseSpeed(),
						emulator.SyncFrameRate(),
						emulator.UseTripleBuffering()
					);

					UpdateFieldMergingState();
					break;

				case Emulator::EVENT_NSF_SELECT:

					nsf.Update( Nes::Nsf(emulator) );
					direct2d.DrawNfo( nsf.text );
					window.Redraw();
					break;

				case Emulator::EVENT_POWER_ON:

					UpdateScreen();

				case Emulator::EVENT_POWER_OFF:

					menu[IDM_FILE_SAVE_SCREENSHOT].Enable( event == Emulator::EVENT_POWER_ON && emulator.IsGame() );
					window.Redraw();
					break;

				case Emulator::EVENT_MODE_NTSC:
				case Emulator::EVENT_MODE_PAL:
				{
					const Rect& currentRect = dialog->GetNesRect
					(
						event == Emulator::EVENT_MODE_NTSC ? Nes::Machine::PAL :
                                                             Nes::Machine::NTSC
					);

					if (Windowed() && WindowMatched( currentRect ))
						ResetScreenRect( CalculateWindowScale( currentRect ) );
					else
						window.Redraw();

					break;
				}

				case Emulator::EVENT_LOAD:

					if (emulator.IsGame())
					{
						dialog->UpdateAutoModes();
					}
					else if (emulator.IsNsf())
					{
						nsf.Load( Nes::Nsf(emulator) );
						direct2d.DrawNfo( nsf.text );
					}
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_OPTIONS_VIDEO].Enable( !data );
					menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].Enable( !data );
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_OPTIONS].Enable( !data );
					break;
			}
		}
	}
}
