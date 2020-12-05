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

#include "NstObjectHeap.hpp"
#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"
#include "NstResourceString.hpp"
#include "NstManager.hpp"
#include "NstDialogFrameClock.hpp"
#include "NstManagerFrameClock.hpp"
#include "../core/api/NstApiSound.hpp"
#include "../core/api/NstApiRewinder.hpp"

namespace Nestopia
{
	namespace Managers
	{
		FrameClock::FrameClock(Window::Menu& m,Emulator& e,const Configuration& cfg,bool modernGPU)
		:
		Manager ( e, m, this, &FrameClock::OnEmuEvent, IDM_OPTIONS_TIMING, &FrameClock::OnMenuOptionsTiming ),
		dialog  ( new Window::FrameClock(cfg,modernGPU) )
		{
			UpdateSettings();

			Io::Log log;
			log << "Timer: performance counter ";

			if (System::Timer::HasPerformanceCounter())
				log << "present (" << uint(System::Timer::GetPerformanceCounterFrequency()) << " hz)\r\n";
			else
				log << "not present\r\n";
		}

		FrameClock::~FrameClock()
		{
		}

		void FrameClock::OnMenuOptionsTiming(uint)
		{
			dialog->Open();
			UpdateSettings();
		}

		void FrameClock::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void FrameClock::UpdateSettings()
		{
			UpdateRewinderState();

			settings.autoFrameSkip = dialog->UseAutoFrameSkip();
			settings.maxFrameSkips = dialog->GetMaxFrameSkips();

			emulator.ResetSpeed
			(
				dialog->UseDefaultSpeed() ? Emulator::DEFAULT_SPEED : dialog->GetSpeed(),
				dialog->UseVSync(),
				dialog->UseTrippleBuffering()
			);

			ResetTimer();
		}

		void FrameClock::UpdateRewinderState(bool force) const
		{
			if (NES_SUCCEEDED(Nes::Rewinder(emulator).Enable( force && dialog->UseRewinder() )))
				Nes::Rewinder(emulator).EnableSound( !dialog->NoRewindSound() );
		}

		void FrameClock::ResetTimer()
		{
			timer.Reset( dialog->UsePerformanceCounter() ? System::Timer::PERFORMANCE : System::Timer::MULTIMEDIA );

			counter = 0;
			clkMul = timer.GetFrequency();
			clkDiv = settings.refreshRate;

			if (Nes::Machine(emulator).Is(Nes::Machine::NTSC))
			{
				if (settings.refreshRate == Nes::Machine::CLK_NTSC_DOT/Nes::Machine::CLK_NTSC_VSYNC)
				{
					clkMul *= uint(Nes::Machine::CLK_NTSC_VSYNC);
					clkDiv = Nes::Machine::CLK_NTSC_DOT;
				}
			}
			else
			{
				if (settings.refreshRate == Nes::Machine::CLK_PAL_DOT/Nes::Machine::CLK_PAL_VSYNC)
				{
					clkMul *= uint(Nes::Machine::CLK_PAL_VSYNC);
					clkDiv = Nes::Machine::CLK_PAL_DOT;
				}
			}
		}

		void FrameClock::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			typedef Nes::Rewinder Rewinder;

			switch (event)
			{
				case Emulator::EVENT_SPEEDING_ON:

					settings.autoFrameSkip = (dialog->UseAutoFrameSkip() || dialog->GetAltSpeed() > emulator.GetDefaultSpeed());
					emulator.SetSpeed( dialog->GetAltSpeed() );
					Nes::Sound(emulator).Mute( dialog->NoAltSpeedSound() );
					break;

				case Emulator::EVENT_SPEEDING_OFF:

					Nes::Sound(emulator).Mute( false );

					if (dialog->UseDefaultRewindSpeed() || Rewinder(emulator).GetDirection() == Rewinder::FORWARD)
					{
						settings.autoFrameSkip = dialog->UseAutoFrameSkip();
						emulator.SetSpeed( Emulator::DEFAULT_SPEED );
					}
					else
					{
						settings.autoFrameSkip = (dialog->UseAutoFrameSkip() || dialog->GetRewindSpeed() > emulator.GetDefaultSpeed());
						emulator.SetSpeed( dialog->GetRewindSpeed() );
					}
					break;

				case Emulator::EVENT_REWINDING_ON:

					if (dialog->UseRewinder())
					{
						if (NES_FAILED(Rewinder(emulator).SetDirection( Rewinder::BACKWARD )))
							Io::Screen() << Resource::String( IDS_EMU_ERR_CANT_REWIND );
					}
					break;

				case Emulator::EVENT_REWINDING_OFF:

					Rewinder(emulator).SetDirection( Rewinder::FORWARD );
					break;

				case Emulator::EVENT_SPEED:

					settings.refreshRate = emulator.GetSpeed();
					ResetTimer();
					break;

				case Emulator::EVENT_REWINDING_START:

					if (!dialog->UseDefaultRewindSpeed() && !emulator.Speeding())
					{
						settings.autoFrameSkip = (dialog->UseAutoFrameSkip() || dialog->GetRewindSpeed() > emulator.GetDefaultSpeed());
						emulator.SetSpeed( dialog->GetRewindSpeed() );
					}

					ResetTimer();
					break;

				case Emulator::EVENT_REWINDING_STOP:

					if (!emulator.Speeding())
					{
						settings.autoFrameSkip = dialog->UseAutoFrameSkip();
						emulator.SetSpeed( Emulator::DEFAULT_SPEED );
					}

					ResetTimer();
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					UpdateRewinderState( !data );
					menu[IDM_OPTIONS_TIMING].Enable( !data );
					break;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		uint FrameClock::Synchronize(const bool throttle,uint skips)
		{
			const System::Timer::Value current( timer.Elapsed() );
			const System::Timer::Value next( clkMul * ++counter / clkDiv );

			if (current > next)
			{
				const uint frames = current * clkDiv / clkMul;

				if (skips & settings.autoFrameSkip)
				{
					skips = frames + 1 - counter;

					if (skips > settings.maxFrameSkips)
						skips = settings.maxFrameSkips;

					counter += skips;
				}
				else
				{
					skips = 0;
				}

				if (counter < frames)
					counter = frames;

				return skips;
			}
			else if (throttle)
			{
				if (!timer.Wait( current, next ))
					ResetTimer();
			}

			return 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
