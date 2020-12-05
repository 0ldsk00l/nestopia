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

#include "NstSystemTimer.hpp"
#include "NstSystemInfo.hpp"
#include "NstWindowParam.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstDialogFrameClock.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct FrameClock::Handlers
		{
			static const MsgHandler::Entry<FrameClock> messages[];
			static const MsgHandler::Entry<FrameClock> commands[];
		};

		const MsgHandler::Entry<FrameClock> FrameClock::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &FrameClock::OnInitDialog },
			{ WM_HSCROLL,    &FrameClock::OnHScroll    }
		};

		const MsgHandler::Entry<FrameClock> FrameClock::Handlers::commands[] =
		{
			{ IDC_TIMING_SYNC_REFRESH,           &FrameClock::OnCmdRefresh      },
			{ IDC_TIMING_AUTO_FRAME_SKIP,        &FrameClock::OnCmdRefresh      },
			{ IDC_TIMING_REWINDER,               &FrameClock::OnCmdRewinder     },
			{ IDC_TIMING_DEFAULT_SPEED,          &FrameClock::OnCmdDefaultSpeed },
			{ IDC_TIMING_REWINDER_DEFAULT_SPEED, &FrameClock::OnCmdRewinder     },
			{ IDC_TIMING_DEFAULT,                &FrameClock::OnCmdDefault      },
			{ IDOK,                              &FrameClock::OnCmdOk           }
		};

		FrameClock::FrameClock(const Configuration& cfg,bool mGPU)
		: dialog(IDD_TIMING,this,Handlers::messages,Handlers::commands), modernGPU(mGPU)
		{
			Configuration::ConstSection timing( cfg["timing"] );

			{
				Configuration::ConstSection autoFrameSkipping( timing["auto-frame-skipping"] );

				if (System::Info::GetCpuSpeed() && System::Info::GetCpuSpeed() <= MAX_MHZ_AUTO_FRAME_SKIP_ENABLE)
					settings.autoFrameSkip = !autoFrameSkipping["enabled"].No();
				else
					settings.autoFrameSkip = autoFrameSkipping["enabled"].Yes();

				settings.maxFrameSkips = autoFrameSkipping["max-frames"].Int();

				settings.maxFrameSkips =
				(
					settings.maxFrameSkips ? NST_CLAMP(settings.maxFrameSkips,MIN_FRAME_SKIPS,MAX_FRAME_SKIPS) :
                                             DEFAULT_FRAME_SKIPS
				);
			}

			{
				Configuration::ConstSection performance( timing["performance"] );

				if (!modernGPU || System::Info::GetCpuSpeed() <= MAX_MHZ_TRIPLE_BUFFERING_ENABLE)
					settings.tripleBuffering = !performance["triple-buffering"].No();
				else
					settings.tripleBuffering = performance["triple-buffering"].Yes();

				if (!System::Timer::HasPerformanceCounter())
				{
					settings.pfCounter = false;
				}
				else if (System::Info::GetCpuCount() == 1)
				{
					settings.pfCounter = !performance["high-precision-timer"].No();
				}
				else
				{
					settings.pfCounter = performance["high-precision-timer"].Yes();
				}

				settings.vsync = performance["vsync"].Yes();
			}

			{
				Configuration::ConstSection rewinder( timing["rewinder"] );

				settings.rewinder = rewinder["enabled"].Yes();
				settings.noRewindSound = rewinder["sound"].No();
				settings.useDefaultRewindSpeed = !rewinder["use-native-speed"].No();

				settings.rewindSpeed = rewinder["speed"].Int();

				settings.rewindSpeed =
				(
					settings.rewindSpeed ? NST_CLAMP(settings.rewindSpeed,MIN_SPEED,MAX_SPEED) :
                                           DEFAULT_REWIND_SPEED
				);
			}

			{
				Configuration::ConstSection speed( timing["speed"] );

				settings.speed = speed["default-speed"].Int();

				settings.speed =
				(
					settings.speed ? NST_CLAMP(settings.speed,MIN_SPEED,MAX_SPEED) :
                                     DEFAULT_SPEED
				);

				settings.altSpeed = speed["alt-speed"].Int();

				settings.altSpeed =
				(
					settings.altSpeed ? NST_CLAMP(settings.altSpeed,MIN_SPEED,MAX_SPEED) :
										DEFAULT_ALT_SPEED
				);

				settings.noAltSpeedSound = speed["alt-speed-sound"].No();

				settings.useDefaultSpeed = !speed["use-native-speed"].No();
			}
		}

		void FrameClock::Save(Configuration& cfg) const
		{
			Configuration::Section timing( cfg["timing"] );

			{
				Configuration::Section rewinder( timing["rewinder"] );

				rewinder[ "enabled"          ].YesNo() = settings.rewinder;
				rewinder[ "use-native-speed" ].YesNo() = settings.useDefaultRewindSpeed;
				rewinder[ "speed"            ].Int() = settings.rewindSpeed;
				rewinder[ "sound"            ].YesNo() = !settings.noRewindSound;
			}

			{
				Configuration::Section autoFrameSkipping( timing["auto-frame-skipping"] );

				autoFrameSkipping[ "max-frames" ].Int() = settings.maxFrameSkips;
				autoFrameSkipping[ "enabled"    ].YesNo() = settings.autoFrameSkip;
			}

			{
				Configuration::Section speed( timing["speed"] );

				speed[ "default-speed"    ].Int() = settings.speed;
				speed[ "alt-speed"        ].Int() = settings.altSpeed;
				speed[ "alt-speed-sound"  ].YesNo() = !settings.noAltSpeedSound;
				speed[ "use-native-speed" ].YesNo() = settings.useDefaultSpeed;
			}

			{
				Configuration::Section performance( timing["performance"] );

				performance[ "vsync"                ].YesNo() = settings.vsync;
				performance[ "triple-buffering"     ].YesNo() = settings.tripleBuffering;
				performance[ "high-precision-timer" ].YesNo() = settings.pfCounter;
			}
		}

		void FrameClock::UpdateRewinderEnable() const
		{
			const bool enabled = dialog.CheckBox( IDC_TIMING_REWINDER ).Checked();
			const bool defaultSpeed = dialog.CheckBox( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Checked();

			dialog.Control( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Enable( enabled );
			dialog.Control( IDC_TIMING_REWINDER_NOSOUND       ).Enable( enabled );
			dialog.Control( IDC_TIMING_REWINDER_SPEED         ).Enable( enabled && !defaultSpeed );
			dialog.Control( IDC_TIMING_REWINDER_SPEED_NUM     ).Enable( enabled && !defaultSpeed );
		}

		ibool FrameClock::OnInitDialog(Param&)
		{
			dialog.Slider      ( IDC_TIMING_SPEED                  ).SetRange     ( MIN_SPEED, MAX_SPEED                   );
			dialog.Slider      ( IDC_TIMING_SPEED                  ).Position() = ( settings.speed                         );
			dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).SetRange     ( MIN_SPEED, MAX_SPEED                   );
			dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).Position() = ( settings.altSpeed                      );
			dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).SetRange     ( MIN_SPEED, MAX_SPEED                   );
			dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).Position() = ( settings.rewindSpeed                   );
			dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).SetRange     ( MIN_FRAME_SKIPS, MAX_FRAME_SKIPS       );
			dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).Position() = ( settings.maxFrameSkips                 );
			dialog.RadioButton ( IDC_TIMING_SYNC_REFRESH           ).Check        ( !settings.autoFrameSkip                );
			dialog.RadioButton ( IDC_TIMING_AUTO_FRAME_SKIP        ).Check        ( settings.autoFrameSkip                 );
			dialog.CheckBox    ( IDC_TIMING_VSYNC                  ).Check        ( settings.vsync                         );
			dialog.CheckBox    ( IDC_TIMING_TRIPLE_BUFFERING       ).Check        ( settings.tripleBuffering               );
			dialog.CheckBox    ( IDC_TIMING_ALT_SPEED_NOSOUND      ).Check        ( settings.noAltSpeedSound               );
			dialog.CheckBox    ( IDC_TIMING_REWINDER               ).Check        ( settings.rewinder                      );
			dialog.CheckBox    ( IDC_TIMING_DEFAULT_SPEED          ).Check        ( settings.useDefaultSpeed               );
			dialog.CheckBox    ( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Check        ( settings.useDefaultRewindSpeed         );
			dialog.CheckBox    ( IDC_TIMING_REWINDER_NOSOUND       ).Check        ( settings.noRewindSound                 );
			dialog.CheckBox    ( IDC_TIMING_PFC                    ).Check        ( settings.pfCounter                     );
			dialog.Control     ( IDC_TIMING_PFC                    ).Enable       ( System::Timer::HasPerformanceCounter() );
			dialog.Control     ( IDC_TIMING_FRAME_SKIPS            ).Enable       ( settings.autoFrameSkip                 );
			dialog.Control     ( IDC_TIMING_FRAME_SKIPS_TEXT       ).Enable       ( settings.autoFrameSkip                 );
			dialog.Control     ( IDC_TIMING_FRAME_SKIPS_NUM        ).Enable       ( settings.autoFrameSkip                 );
			dialog.Control     ( IDC_TIMING_SPEED                  ).Enable       ( !settings.useDefaultSpeed              );
			dialog.Control     ( IDC_TIMING_SPEED_NUM              ).Enable       ( !settings.useDefaultSpeed              );

			dialog.Edit( IDC_TIMING_SPEED_NUM          ) << uint( settings.speed );
			dialog.Edit( IDC_TIMING_ALT_SPEED_NUM      ) << uint( settings.altSpeed );
			dialog.Edit( IDC_TIMING_REWINDER_SPEED_NUM ) << uint( settings.rewindSpeed );
			dialog.Edit( IDC_TIMING_FRAME_SKIPS_NUM    ) << uint( settings.maxFrameSkips );

			UpdateRewinderEnable();

			return true;
		}

		ibool FrameClock::OnHScroll(Param& param)
		{
			NST_COMPILE_ASSERT
			(
				IDC_TIMING_SPEED          == IDC_TIMING_SPEED_NUM-1          &&
				IDC_TIMING_FRAME_SKIPS    == IDC_TIMING_FRAME_SKIPS_NUM-1    &&
				IDC_TIMING_ALT_SPEED      == IDC_TIMING_ALT_SPEED_NUM-1      &&
				IDC_TIMING_REWINDER_SPEED == IDC_TIMING_REWINDER_SPEED_NUM-1
			);

			switch (const uint id = param.Slider().GetId())
			{
				case IDC_TIMING_SPEED:
				case IDC_TIMING_ALT_SPEED:
				case IDC_TIMING_REWINDER_SPEED:
				case IDC_TIMING_FRAME_SKIPS:

					dialog.Edit( id+1 ) << param.Slider().Scroll();
					break;
			}

			return true;
		}

		ibool FrameClock::OnCmdRefresh(Param& param)
		{
			if (param.Button().Clicked())
			{
				const bool autoFrameSkip = (param.Button().GetId() == IDC_TIMING_AUTO_FRAME_SKIP);

				dialog.Control( IDC_TIMING_FRAME_SKIPS      ).Enable( autoFrameSkip );
				dialog.Control( IDC_TIMING_FRAME_SKIPS_TEXT ).Enable( autoFrameSkip );
				dialog.Control( IDC_TIMING_FRAME_SKIPS_NUM  ).Enable( autoFrameSkip );
			}

			return true;
		}

		ibool FrameClock::OnCmdDefaultSpeed(Param& param)
		{
			if (param.Button().Clicked())
			{
				const bool speed = dialog.CheckBox( IDC_TIMING_DEFAULT_SPEED ).Unchecked();

				dialog.Control( IDC_TIMING_SPEED     ).Enable( speed );
				dialog.Control( IDC_TIMING_SPEED_NUM ).Enable( speed );
			}

			return true;
		}

		ibool FrameClock::OnCmdRewinder(Param& param)
		{
			if (param.Button().Clicked())
				UpdateRewinderEnable();

			return true;
		}

		ibool FrameClock::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Slider      ( IDC_TIMING_SPEED                  ).Position() = DEFAULT_SPEED;
				dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).Position() = DEFAULT_ALT_SPEED;
				dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).Position() = DEFAULT_REWIND_SPEED;
				dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).Position() = DEFAULT_FRAME_SKIPS;
				dialog.RadioButton ( IDC_TIMING_SYNC_REFRESH           ).Check  ( true  );
				dialog.RadioButton ( IDC_TIMING_AUTO_FRAME_SKIP        ).Check  ( System::Info::GetCpuSpeed() && System::Info::GetCpuSpeed() <= MAX_MHZ_AUTO_FRAME_SKIP_ENABLE );
				dialog.CheckBox    ( IDC_TIMING_VSYNC                  ).Check  ( false  );
				dialog.CheckBox    ( IDC_TIMING_TRIPLE_BUFFERING       ).Check  ( !modernGPU || System::Info::GetCpuSpeed() <= MAX_MHZ_TRIPLE_BUFFERING_ENABLE );
				dialog.CheckBox    ( IDC_TIMING_DEFAULT_SPEED          ).Check  ( true  );
				dialog.CheckBox    ( IDC_TIMING_ALT_SPEED_NOSOUND      ).Check  ( false );
				dialog.CheckBox    ( IDC_TIMING_REWINDER               ).Check  ( false );
				dialog.CheckBox    ( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Check  ( true  );
				dialog.CheckBox    ( IDC_TIMING_REWINDER_NOSOUND       ).Check  ( false );
				dialog.CheckBox    ( IDC_TIMING_PFC                    ).Check  ( System::Timer::HasPerformanceCounter() && System::Info::GetCpuCount() == 1 );
				dialog.Control     ( IDC_TIMING_PFC                    ).Enable ( System::Timer::HasPerformanceCounter() );
				dialog.Control     ( IDC_TIMING_FRAME_SKIPS            ).Enable ( false );
				dialog.Control     ( IDC_TIMING_FRAME_SKIPS_TEXT       ).Enable ( false );
				dialog.Control     ( IDC_TIMING_FRAME_SKIPS_NUM        ).Enable ( false );
				dialog.Control     ( IDC_TIMING_SPEED                  ).Enable ( false );
				dialog.Control     ( IDC_TIMING_SPEED_NUM              ).Enable ( false );
				dialog.Edit        ( IDC_TIMING_SPEED_NUM              ) << uint(DEFAULT_SPEED);
				dialog.Edit        ( IDC_TIMING_ALT_SPEED_NUM          ) << uint(DEFAULT_ALT_SPEED);
				dialog.Edit        ( IDC_TIMING_REWINDER_SPEED_NUM     ) << uint(DEFAULT_REWIND_SPEED);
				dialog.Edit        ( IDC_TIMING_FRAME_SKIPS_NUM        ) << uint(DEFAULT_FRAME_SKIPS);

				UpdateRewinderEnable();
			}

			return true;
		}

		ibool FrameClock::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.autoFrameSkip         = dialog.RadioButton ( IDC_TIMING_AUTO_FRAME_SKIP        ).Checked();
				settings.vsync                 = dialog.CheckBox    ( IDC_TIMING_VSYNC                  ).Checked();
				settings.tripleBuffering       = dialog.CheckBox    ( IDC_TIMING_TRIPLE_BUFFERING       ).Checked();
				settings.rewinder              = dialog.CheckBox    ( IDC_TIMING_REWINDER               ).Checked();
				settings.useDefaultSpeed       = dialog.CheckBox    ( IDC_TIMING_DEFAULT_SPEED          ).Checked();
				settings.useDefaultRewindSpeed = dialog.CheckBox    ( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Checked();
				settings.noAltSpeedSound       = dialog.CheckBox    ( IDC_TIMING_ALT_SPEED_NOSOUND      ).Checked();
				settings.noRewindSound         = dialog.CheckBox    ( IDC_TIMING_REWINDER_NOSOUND       ).Checked();
				settings.pfCounter             = dialog.CheckBox    ( IDC_TIMING_PFC                    ).Checked();
				settings.speed                 = dialog.Slider      ( IDC_TIMING_SPEED                  ).Position();
				settings.altSpeed              = dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).Position();
				settings.rewindSpeed           = dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).Position();
				settings.maxFrameSkips         = dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).Position();

				dialog.Close();
			}

			return true;
		}
	}
}
