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

#include <cstring>
#include "NstCpu.hpp"
#include "NstState.hpp"
#include "api/NstApiSound.hpp"
#include "NstSoundRenderer.inl"

// CPU cycles between a write to $4015 that enables the DMC and the enable
// reaching the DMA unit. The ROM states this outright in its own test:
// "The Delta Modulation Channel will be enabled in 3 CPU cycles."
#ifndef DMC_ENABLE_DELAY
#define DMC_ENABLE_DELAY 3
#endif

namespace Nes
{
	namespace Core
	{
		const dword Apu::Cycles::frameClocks[3][4] =
		{
			{
				CPU_RP2A03_CC * 29830UL,
				CPU_RP2A03_CC,
				CPU_RP2A03_CC,
				CPU_RP2A03_CC * (29830UL - 2),
			},
			{
				CPU_RP2A07_CC * 33254UL,
				CPU_RP2A07_CC,
				CPU_RP2A07_CC,
				CPU_RP2A07_CC * (33254UL - 2)
			},
			{
				CPU_DENDY_CC * 29830UL,
				CPU_DENDY_CC,
				CPU_DENDY_CC,
				CPU_DENDY_CC * (29830UL - 2),
			}
		};

		const dword Apu::Cycles::oscillatorClocks[3][2][4] =
		{
			{
				{
					CPU_RP2A03_CC * (7459UL - 1),
					CPU_RP2A03_CC * 7456UL,
					CPU_RP2A03_CC * 7458UL,
					CPU_RP2A03_CC * 7458UL
				},
				{
					CPU_RP2A03_CC * 7458UL,
					CPU_RP2A03_CC * 7456UL,
					CPU_RP2A03_CC * 7458UL,
					CPU_RP2A03_CC * (7458UL + 7452)
				}
			},
			{
				{
					CPU_RP2A07_CC * (8315UL - 1),
					CPU_RP2A07_CC * 8314UL,
					CPU_RP2A07_CC * 8312UL,
					CPU_RP2A07_CC * 8314UL
				},
				{
					CPU_RP2A07_CC * 8314UL,
					CPU_RP2A07_CC * 8314UL,
					CPU_RP2A07_CC * 8312UL,
					CPU_RP2A07_CC * (8314UL + 8312)
				}
			},
			{
				{
					CPU_DENDY_CC * (7459UL - 1),
					CPU_DENDY_CC * 7456UL,
					CPU_DENDY_CC * 7458UL,
					CPU_DENDY_CC * 7458UL
				},
				{
					CPU_DENDY_CC * 7458UL,
					CPU_DENDY_CC * 7456UL,
					CPU_DENDY_CC * 7458UL,
					CPU_DENDY_CC * (7458UL + 7452)
				}
			}
		};

		const byte Apu::Channel::LengthCounter::lut[32] =
		{
			0x0A, 0xFE, 0x14, 0x02,
			0x28, 0x04, 0x50, 0x06,
			0xA0, 0x08, 0x3C, 0x0A,
			0x0E, 0x0C, 0x1A, 0x0E,
			0x0C, 0x10, 0x18, 0x12,
			0x30, 0x14, 0x60, 0x16,
			0xC0, 0x18, 0x48, 0x1A,
			0x10, 0x1C, 0x20, 0x1E
		};

		const word Apu::Noise::lut[3][16] =
		{
			{
				0x004, 0x008, 0x010, 0x020,
				0x040, 0x060, 0x080, 0x0A0,
				0x0CA, 0x0FE, 0x17C, 0x1FC,
				0x2FA, 0x3F8, 0x7F2, 0xFE4
			},
			{
				0x004, 0x007, 0x00E, 0x01E,
				0x03C, 0x058, 0x076, 0x094,
				0x0BC, 0x0EC, 0x162, 0x1D8,
				0x2C4, 0x3B0, 0x762, 0xEC2
			},
			{
				0x004, 0x008, 0x010, 0x020,
				0x040, 0x060, 0x080, 0x0A0,
				0x0CA, 0x0FE, 0x17C, 0x1FC,
				0x2FA, 0x3F8, 0x7F2, 0xFE4
			}
		};

		const word Apu::Dmc::lut[3][16] =
		{
			{
				0x1AC * CPU_RP2A03_CC,
				0x17C * CPU_RP2A03_CC,
				0x154 * CPU_RP2A03_CC,
				0x140 * CPU_RP2A03_CC,
				0x11E * CPU_RP2A03_CC,
				0x0FE * CPU_RP2A03_CC,
				0x0E2 * CPU_RP2A03_CC,
				0x0D6 * CPU_RP2A03_CC,
				0x0BE * CPU_RP2A03_CC,
				0x0A0 * CPU_RP2A03_CC,
				0x08E * CPU_RP2A03_CC,
				0x080 * CPU_RP2A03_CC,
				0x06A * CPU_RP2A03_CC,
				0x054 * CPU_RP2A03_CC,
				0x048 * CPU_RP2A03_CC,
				0x036 * CPU_RP2A03_CC
			},
			{
				0x18E * CPU_RP2A07_CC,
				0x162 * CPU_RP2A07_CC,
				0x13C * CPU_RP2A07_CC,
				0x12A * CPU_RP2A07_CC,
				0x114 * CPU_RP2A07_CC,
				0x0EC * CPU_RP2A07_CC,
				0x0D2 * CPU_RP2A07_CC,
				0x0C6 * CPU_RP2A07_CC,
				0x0B0 * CPU_RP2A07_CC,
				0x094 * CPU_RP2A07_CC,
				0x084 * CPU_RP2A07_CC,
				0x076 * CPU_RP2A07_CC,
				0x062 * CPU_RP2A07_CC,
				0x04E * CPU_RP2A07_CC,
				0x042 * CPU_RP2A07_CC,
				0x032 * CPU_RP2A07_CC
			},
			{
				0x1AC * CPU_DENDY_CC,
				0x17C * CPU_DENDY_CC,
				0x154 * CPU_DENDY_CC,
				0x140 * CPU_DENDY_CC,
				0x11E * CPU_DENDY_CC,
				0x0FE * CPU_DENDY_CC,
				0x0E2 * CPU_DENDY_CC,
				0x0D6 * CPU_DENDY_CC,
				0x0BE * CPU_DENDY_CC,
				0x0A0 * CPU_DENDY_CC,
				0x08E * CPU_DENDY_CC,
				0x080 * CPU_DENDY_CC,
				0x06A * CPU_DENDY_CC,
				0x054 * CPU_DENDY_CC,
				0x048 * CPU_DENDY_CC,
				0x036 * CPU_DENDY_CC
			}
		};

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Apu(Cpu& c)
		:
		cpu        (c),
		extChannel (NULL),
		buffer     (16)
		{
			NST_COMPILE_ASSERT( CPU_RP2A03 == 0 && CPU_RP2A07 == 1 && CPU_DENDY == 2 );

			PowerOff();
		}

		void Apu::PowerOff()
		{
			Reset( false, true );
		}

		void Apu::Reset(bool hard)
		{
			Reset( true, hard );
		}

		void Apu::Reset(const bool on,const bool hard)
		{
			if (on)
				UpdateSettings();

			updater = &Apu::SyncOff;

			cycles.Reset( extChannel, cpu.GetModel() );
			synchronizer.Resync( settings.speed, cpu );

			for (uint i=0; i < 2; ++i)
				square[i].Reset();

			triangle.Reset();
			noise.Reset( cpu.GetModel() );
			dmc.Reset( cpu.GetModel() );

			dcBlocker.Reset();

			stream = NULL;

			buffer.Reset();

			if (on)
			{
				cpu.Map( 0x4000 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4000 );
				cpu.Map( 0x4001 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4001 );
				cpu.Map( 0x4002 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4002 );
				cpu.Map( 0x4003 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4003 );
				cpu.Map( 0x4004 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4000 );
				cpu.Map( 0x4005 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4001 );
				cpu.Map( 0x4006 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4002 );
				cpu.Map( 0x4007 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4003 );
				cpu.Map( 0x4008 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4008 );
				cpu.Map( 0x400A ).Set( this, &Apu::Peek_40xx, &Apu::Poke_400A );
				cpu.Map( 0x400B ).Set( this, &Apu::Peek_40xx, &Apu::Poke_400B );
				cpu.Map( 0x400C ).Set( this, &Apu::Peek_40xx, &Apu::Poke_400C );
				cpu.Map( 0x400E ).Set( this, &Apu::Peek_40xx, &Apu::Poke_400E );
				cpu.Map( 0x400F ).Set( this, &Apu::Peek_40xx, &Apu::Poke_400F );
				cpu.Map( 0x4010 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4010 );
				cpu.Map( 0x4011 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4011 );
				cpu.Map( 0x4012 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4012 );
				cpu.Map( 0x4013 ).Set( this, &Apu::Peek_40xx, &Apu::Poke_4013 );
				cpu.Map( 0x4015 ).Set( this, &Apu::Peek_4015, &Apu::Poke_4015 );

				if (cpu.GetApu().IsGenie())
				{
					NES_DO_POKE(4000,0x4000,0x30);
					NES_DO_POKE(4001,0x4001,0xF9);
					NES_DO_POKE(400C,0x400C,0x30);
					NES_DO_POKE(400E,0x400E,0x0E);
					NES_DO_POKE(400F,0x400F,0x04);
					NES_DO_POKE(4015,0x4015,0x09);
				}

				if (hard)
				{
					ctrl = STATUS_FRAME_IRQ_ENABLE;
				}

				if (ctrl == STATUS_FRAME_IRQ_ENABLE)
					cycles.frameIrqClock = (cycles.frameCounter / cycles.fixed) - cpu.GetClock();

				if (extChannel)
					extChannel->Reset();
			}
			else
			{
				ctrl = STATUS_FRAME_IRQ_ENABLE;
			}
		}

		Result Apu::SetSampleRate(const dword rate)
		{
			if (settings.rate == rate)
				return RESULT_NOP;

			if (!rate)
				return RESULT_ERR_INVALID_PARAM;

			/*if (rate < 44100 || rate > 96000)
				return RESULT_ERR_UNSUPPORTED;*/

			settings.rate = rate;
			UpdateSettings();

			return RESULT_OK;
		}

		Result Apu::SetVolume(const uint channels,const uint volume)
		{
			if (volume > 100)
				return RESULT_ERR_INVALID_PARAM;

			bool updated = false;

			for (uint i=0; i < MAX_CHANNELS; ++i)
			{
				if (channels & (1U << i))
				{
					if (settings.volumes[i] != volume)
					{
						settings.volumes[i] = volume;
						updated = true;
					}
				}
			}

			if (!updated)
				return RESULT_NOP;

			UpdateSettings();

			return RESULT_OK;
		}

		uint Apu::GetVolume(const uint channel) const
		{
			for (uint i=0; i < MAX_CHANNELS; ++i)
			{
				if (channel & (1U << i))
					return settings.volumes[i];
			}

			return 0;
		}

		uint Apu::GetCtrl()
		{
			return ctrl;
		}

		Result Apu::SetSpeed(const uint speed)
		{
			if (settings.speed == speed)
				return RESULT_NOP;

			if ((speed > 0 && speed < 30) || speed > 240)
				return RESULT_ERR_UNSUPPORTED;

			settings.speed = speed;
			UpdateSettings();

			return RESULT_OK;
		}

		void Apu::Mute(const bool mute)
		{
			if (settings.muted != mute)
			{
				settings.muted = mute;
				UpdateSettings();
			}
		}

		void Apu::SetAutoTranspose(const bool transpose)
		{
			if (settings.transpose != transpose)
			{
				settings.transpose = transpose;
				UpdateSettings();
			}
		}

		void Apu::SetGenie(const bool genie)
		{
			if (settings.genie != genie)
			{
				settings.genie = genie;
				UpdateSettings();
			}
		}

		void Apu::EnableStereo(const bool enable)
		{
			if (settings.stereo != enable)
			{
				settings.stereo = enable;
				UpdateSettings();
			}
		}

		void Apu::UpdateSettings()
		{
			cycles.Update( settings.rate, settings.speed, cpu );
			synchronizer.Reset( settings.speed, settings.rate, cpu );
			dcBlocker.Reset();
			buffer.Reset();

			Cycle rate; uint fixed;
			CalculateOscillatorClock( rate, fixed );

			/* The DAC index is a sum of channel levels, so a channel has to
			 * contribute a whole number of them. Volume is mute or nothing.
			*/
			#define NST_APU_VOL(c_) ((settings.muted || !settings.volumes[ Channel::c_ ]) ? 0U : uint(Channel::DEFAULT_VOLUME))

			square[0].UpdateSettings ( NST_APU_VOL( APU_SQUARE1  ), rate, fixed );
			square[1].UpdateSettings ( NST_APU_VOL( APU_SQUARE2  ), rate, fixed );
			triangle.UpdateSettings  ( NST_APU_VOL( APU_TRIANGLE ), rate, fixed );
			noise.UpdateSettings     ( NST_APU_VOL( APU_NOISE    ), rate, fixed );
			dmc.UpdateSettings       ( NST_APU_VOL( APU_DPCM     ) );

			#undef NST_APU_VOL

			UpdateMixLut();
			UpdateVolumes();
		}

		void Apu::UpdateVolumes()
		{
			settings.audible = (extChannel && extChannel->UpdateSettings()) ||
			(
				uint(settings.volumes[ Channel::APU_SQUARE1  ]) |
				uint(settings.volumes[ Channel::APU_SQUARE2  ]) |
				uint(settings.volumes[ Channel::APU_TRIANGLE ]) |
				uint(settings.volumes[ Channel::APU_NOISE    ]) |
				uint(settings.volumes[ Channel::APU_DPCM     ])
			);
		}

		void Apu::Resync(const dword rate)
		{
			cycles.Update( rate, settings.speed, cpu );
			ClearBuffers( false );
		}

		void Apu::CalculateOscillatorClock(Cycle& rate,uint& fixed) const
		{
			dword sampleRate = settings.rate;

			if (settings.transpose && settings.speed)
				sampleRate = sampleRate * cpu.GetFps() / settings.speed;

			uint multiplier = 0;
			const qaword clockBase = cpu.GetClockBase();

			while (++multiplier < 0x1000 && clockBase * (multiplier+1) / sampleRate <= 0x7FFFF && clockBase * multiplier % sampleRate);

			rate = clockBase * multiplier / sampleRate;
			fixed = cpu.GetClockDivider() * cpu.GetClock() * multiplier;
		}

		void Apu::SaveState(State::Saver& state,const dword baseChunk) const
		{
			state.Begin( baseChunk );

			{
				Cycle clock = cycles.frameCounter / cycles.fixed;

				NST_VERIFY( clock >= cpu.GetCycles() );

				if (clock > cpu.GetCycles())
					clock = (clock - cpu.GetCycles()) / cpu.GetClock();
				else
					clock = 0;

				NST_VERIFY( cycles.frameCounter == (cpu.GetCycles() + clock * cpu.GetClock()) * cycles.fixed );

				const byte data[4] =
				{
					static_cast<byte>(ctrl),
					static_cast<byte>(clock & 0xFF),
					static_cast<byte>(clock >> 8),
					static_cast<byte>(cycles.frameDivider)
				};

				state.Begin( AsciiId<'F','R','M'>::V ).Write( data ).End();
			}

			if (cycles.frameIrqClock != Cpu::CYCLE_MAX)
			{
				Cycle clock = cycles.frameIrqClock;

				NST_VERIFY( clock >= cpu.GetCycles() );

				if (clock > cpu.GetCycles())
					clock = (clock - cpu.GetCycles()) / cpu.GetClock();
				else
					clock = 0;

				NST_VERIFY( cycles.frameIrqClock == cpu.GetCycles() + clock * cpu.GetClock() );

				const byte data[3] =
				{
					static_cast<byte>(clock & 0xFF),
					static_cast<byte>(clock >> 8),
					static_cast<byte>(cycles.frameIrqRepeat % 3)
				};

				state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
			}

			if (cycles.extCounter != Cpu::CYCLE_MAX)
			{
				Cycle clock = cycles.extCounter / cycles.fixed;

				NST_VERIFY( clock >= cpu.GetCycles() || clock == 0 );

				if (clock > cpu.GetCycles())
				{
					clock = (clock - cpu.GetCycles()) / cpu.GetClock();
					NST_VERIFY( cycles.extCounter == (cpu.GetCycles() + clock * cpu.GetClock()) * cycles.fixed );
				}
				else
				{
					clock = 0;
				}

				state.Begin( AsciiId<'E','X','T'>::V ).Write16( clock ).End();
			}

			{
				/* Both are rebased by EndFrame, so both can outlive the frame
				 * they were opened in and have to be carried.
				*/
				const byte data[6] =
				{
					static_cast<byte>(cycles.frameIrqHold & 0xFF),
					static_cast<byte>(cycles.frameIrqHold >> 8 & 0xFF),
					static_cast<byte>(cycles.frameIrqHold >> 16 & 0xFF),
					static_cast<byte>(cycles.frameIrqPhantom & 0xFF),
					static_cast<byte>(cycles.frameIrqPhantom >> 8 & 0xFF),
					static_cast<byte>(cycles.frameIrqPhantom >> 16 & 0xFF)
				};

				state.Begin( AsciiId<'F','I','W'>::V ).Write( data ).End();
			}

			square[0].SaveState( state, AsciiId<'S','Q','0'>::V );
			square[1].SaveState( state, AsciiId<'S','Q','1'>::V );
			triangle.SaveState( state, AsciiId<'T','R','I'>::V );
			noise.SaveState( state, AsciiId<'N','O','I'>::V );
			dmc.SaveState( state, AsciiId<'D','M','C'>::V, cpu, cycles.dmcClock );

			dcBlocker.SaveState( state, AsciiId<'D','C','B'>::V );

			{
				const byte data[4] =
				{
					static_cast<byte>(cycles.rateCounter & 0xFFU),
					static_cast<byte>(cycles.rateCounter >> 8),
					static_cast<byte>(cycles.rateCounter >> 16),
					static_cast<byte>(cycles.rateCounter >> 24),
				};

				state.Begin( AsciiId<'S','0','0'>::V ).Write( data ).End();
			}

			state.End();
		}

		void Apu::LoadState(State::Loader& state)
		{
			cycles.frameIrqHold = 0;
			cycles.frameIrqPhantom = 0;

			cycles.frameIrqClock = Cpu::CYCLE_MAX;
			cycles.frameIrqRepeat = 0;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'F','R','M'>::V:
					{
						State::Loader::Data<4> data( state );

						ctrl = data[0] & STATUS_BITS;

						cycles.rateCounter = cycles.fixed * cpu.GetCycles();

						cycles.frameCounter = cycles.fixed *
						(
							cpu.GetCycles() + (data[1] | data[2] << 8) * cpu.GetClock()
						);

						cycles.frameDivider = data[3] & 0x3;
						break;
					}

					case AsciiId<'I','R','Q'>::V:
					{
						State::Loader::Data<3> data( state );

						cycles.frameIrqClock = cpu.GetCycles() +
						(
							(data[0] | data[1] << 8) * cpu.GetClock()
						);

						cycles.frameIrqRepeat = (data[2] & 0x3) % 3;
						break;
					}

					case AsciiId<'F','I','W'>::V:
					{
						State::Loader::Data<6> data( state );

						cycles.frameIrqHold =
							data[0] | data[1] << 8 | dword(data[2]) << 16;
						cycles.frameIrqPhantom =
							data[3] | data[4] << 8 | dword(data[5]) << 16;

						break;
					}

					case AsciiId<'E','X','T'>::V:

						NST_VERIFY( cycles.extCounter != Cpu::CYCLE_MAX );

						if (cycles.extCounter != Cpu::CYCLE_MAX)
						{
							cycles.extCounter = cycles.fixed *
							(
								cpu.GetCycles() + state.Read16() * cpu.GetClock()
							);
						}
						break;

					case AsciiId<'S','Q','0'>::V:

						square[0].LoadState( state );
						break;

					case AsciiId<'S','Q','1'>::V:

						square[1].LoadState( state );
						break;

					case AsciiId<'T','R','I'>::V:

						triangle.LoadState( state );
						break;

					case AsciiId<'N','O','I'>::V:

						noise.LoadState( state, cpu.GetModel() );
						break;

					case AsciiId<'D','M','C'>::V:

						dmc.LoadState( state, cpu, cpu.GetModel(), cycles.dmcClock );
						break;

					case AsciiId<'D','C','B'>::V:

						dcBlocker.LoadState( state );
						break;

					case AsciiId<'S','0','0'>::V:
					{
						State::Loader::Data<4> data( state );

						cycles.rateCounter = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
						break;
					}
				}

				state.End();
			}

			if (ctrl != STATUS_FRAME_IRQ_ENABLE)
			{
				cycles.frameIrqClock = Cpu::CYCLE_MAX;
				cycles.frameIrqRepeat = 0;
			}
			else if (cycles.frameIrqClock == Cpu::CYCLE_MAX)
			{
				cycles.frameIrqClock = (cycles.frameCounter / cycles.fixed) + (3 - cycles.frameDivider) * (Cycles::frameClocks[cpu.GetModel()][0] / 4);
				cycles.frameIrqRepeat = 0;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void NST_FASTCALL Apu::SyncOn(const Cycle target)
		{
			NST_ASSERT( (stream && settings.audible) && (cycles.rate && cycles.fixed) && (cycles.extCounter == Cpu::CYCLE_MAX) );

			if (cycles.rateCounter < target)
			{
				Cycle rateCounter = cycles.rateCounter;
				const Cycle rate = cycles.rate;

				do
				{
					buffer << GetSample();

					if (cycles.frameCounter <= rateCounter)
						ClockFrameCounter();

					rateCounter += rate;
				}
				while (rateCounter < target);

				cycles.rateCounter = rateCounter;
			}

			if (cycles.frameCounter < target)
			{
				ClockFrameCounter();
				NST_ASSERT( cycles.frameCounter >= target );
			}
		}

		void NST_FASTCALL Apu::SyncOnExt(const Cycle target)
		{
			NST_ASSERT( (stream && settings.audible) && (cycles.rate && cycles.fixed) && extChannel );

			Cycle extCounter = cycles.extCounter;

			if (cycles.rateCounter < target)
			{
				Cycle rateCounter = cycles.rateCounter;

				do
				{
					buffer << GetSample();

					if (extCounter <= rateCounter)
						extCounter = extChannel->Clock( extCounter, cycles.fixed, rateCounter );

					if (cycles.frameCounter <= rateCounter)
						ClockFrameCounter();

					rateCounter += cycles.rate;
				}
				while (rateCounter < target);

				cycles.rateCounter = rateCounter;
			}

			if (extCounter <= target)
			{
				cycles.extCounter = extChannel->Clock( extCounter, cycles.fixed, target );
				NST_ASSERT( cycles.extCounter > target );
			}
			else
			{
				cycles.extCounter = extCounter;
			}

			if (cycles.frameCounter < target)
			{
				ClockFrameCounter();
				NST_ASSERT( cycles.frameCounter >= target );
			}
		}

		void NST_FASTCALL Apu::SyncOff(const Cycle target)
		{
			NST_ASSERT( !(stream && settings.audible) && cycles.fixed );

			cycles.rateCounter = target;

			while (cycles.frameCounter < target)
				ClockFrameCounter();

			NST_ASSERT( cycles.extCounter == Cpu::CYCLE_MAX || extChannel );

			if (cycles.extCounter <= target)
			{
				cycles.extCounter = extChannel->Clock( cycles.extCounter, cycles.fixed, target );
				NST_ASSERT( cycles.extCounter > target );
			}
		}

		void Apu::BeginFrame(Sound::Output* output)
		{
			stream = output;
			updater = (output && settings.audible ? (cycles.extCounter == Cpu::CYCLE_MAX ? &Apu::SyncOn : &Apu::SyncOnExt) : &Apu::SyncOff);
		}

		inline void Apu::Update(const Cycle target)
		{
			NST_ASSERT( cycles.fixed );
			(*this.*updater)( target * cycles.fixed );
		}

		void Apu::Update()
		{
			Update( cpu.Update() );
		}

		void Apu::UpdateLatency()
		{
			Update( cpu.Update() + 1 );
		}

		bool Apu::UpdateDelta()
		{
			const Cycle elapsed = cpu.Update();
			const bool delta = cycles.frameCounter != elapsed * cycles.fixed;
			Update( elapsed + 1 );
			return delta;
		}

		template<typename T,bool STEREO>
		void Apu::FlushSound()
		{
			NST_ASSERT( (stream && settings.audible) && (cycles.rate && cycles.fixed) );

			for (uint i=0; i < 2; ++i)
			{
				if (stream->length[i] && stream->samples[i])
				{
					Sound::Buffer::Block block( stream->length[i] );
					buffer >> block;

					Sound::Buffer::Renderer<T,STEREO> output( stream->samples[i], stream->length[i], buffer.history );

					if (output << block)
					{
						const Cycle target = cpu.GetCycles() * cycles.fixed;

						if (cycles.rateCounter < target)
						{
							Cycle rateCounter = cycles.rateCounter;

							do
							{
								output << GetSample();

								if (cycles.frameCounter <= rateCounter)
									ClockFrameCounter();

								if (cycles.extCounter <= rateCounter)
									cycles.extCounter = extChannel->Clock( cycles.extCounter, cycles.fixed, rateCounter );

								rateCounter += cycles.rate;
							}
							while (rateCounter < target && output);

							cycles.rateCounter = rateCounter;
						}

						if (output)
						{
							if (cycles.frameCounter < target)
								ClockFrameCounter();

							if (cycles.extCounter <= target)
								cycles.extCounter = extChannel->Clock( cycles.extCounter, cycles.fixed, target );

							do
							{
								output << GetSample();
							}
							while (output);
						}
					}
				}
			}
		}

		void Apu::EndFrame()
		{
			NST_ASSERT( (stream && settings.audible) == (updater != &Apu::SyncOff) );

			if (updater != &Apu::SyncOff)
			{
				dword streamed = 0;

				if (Sound::Output::lockCallback( *stream ))
				{
					streamed = stream->length[0] + stream->length[1];

					if (!settings.stereo)
						FlushSound<iword,false>();
					else
						FlushSound<iword,true>();

					Sound::Output::unlockCallback( *stream );
				}

				if (const dword rate = synchronizer.Clock( streamed, settings.rate, cpu ))
					Resync( rate );
			}

			Update( cpu.GetCycles() );

			Cycle frame = cpu.GetFrameCycles();

			/* The DMC halt is dispatched two cycles after the output clock,
			 * so a clock due in the last cycles of a frame is still pending
			 * here. Cycle is unsigned and the rebase below subtracts the
			 * frame length from it, so a pending clock wraps to just under
			 * 2^32 - and the invariant asserted below (NST_ASSUME in release
			 * builds) states that cannot happen. Retire it now instead: the
			 * CPU has stopped for the frame, so there is no in-flight read
			 * left for the DMA to collide with.
			*/
			while (cycles.dmcClock < frame)
				ClockDmc( cycles.dmcClock + cpu.GetClock(2) );

			/* Same for a load scheduled in the last cycles of a frame:
			 * RebaseFrame clamps at zero, which would silently discard it.
			*/
			if (dmc.HasPendingLoad() && dmc.GetLoadClock() < frame)
				ClockPendingLoad( dmc.GetLoadClock() + cpu.GetClock(2), 0 );

			NST_ASSERT
			(
				cycles.dmcClock >= frame &&
				cycles.frameIrqClock >= frame
			);

			cycles.dmcClock -= frame;
			dmc.RebaseFrame( frame );
			cycles.frameIrqHold -= NST_MIN( cycles.frameIrqHold, frame );
			cycles.frameIrqPhantom -= NST_MIN( cycles.frameIrqPhantom, frame );

			if (cycles.frameIrqClock != Cpu::CYCLE_MAX)
				cycles.frameIrqClock -= frame;

			frame *= cycles.fixed;

			NST_ASSERT
			(
				cycles.rateCounter >= frame &&
				cycles.frameCounter >= frame &&
				cycles.extCounter >= frame
			);

			cycles.rateCounter -= frame;
			cycles.frameCounter -= frame;

			if (cycles.extCounter != Cpu::CYCLE_MAX)
				cycles.extCounter -= frame;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Settings::Settings()
		: rate(44100), speed(0), muted(false), transpose(false), stereo(false), audible(true)
		{
			for (uint i=0; i < MAX_CHANNELS; ++i)
				volumes[i] = Channel::DEFAULT_VOLUME;
		}

		Apu::Cycles::Cycles()
		: fixed(1), rate(1) {}

		void Apu::Cycles::Reset(const bool extChannel,const CpuModel model)
		{
			rateCounter = 0;
			frameDivider = 0;
			frameIrqClock = Cpu::CYCLE_MAX;
			frameIrqHold = 0;
			frameIrqPhantom = 0;
			frameIrqRepeat = 0;
			dmcClock = Dmc::GetResetFrequency( model );
			frameCounter = frameClocks[model][0] * fixed;
			extCounter = (extChannel ? 0UL : Cpu::CYCLE_MAX);
		}

		void Apu::Cycles::Update(dword sampleRate,const uint speed,const Cpu& cpu)
		{
			frameCounter /= fixed;
			rateCounter /= fixed;

			if (extCounter != Cpu::CYCLE_MAX)
				extCounter /= fixed;

			if (speed)
				sampleRate = sampleRate * cpu.GetFps() / speed;

			uint multiplier = 0;
			const qaword clockBase = cpu.GetClockBase();

			while (++multiplier < 512 && clockBase * multiplier % sampleRate);

			rate = clockBase * multiplier / sampleRate;
			fixed = cpu.GetClockDivider() * multiplier;

			frameCounter *= fixed;
			rateCounter *= fixed;

			if (extCounter != Cpu::CYCLE_MAX)
				extCounter *= fixed;
		}

		Apu::Synchronizer::Synchronizer()
		: rate(0) {}

		void Apu::Synchronizer::Resync(uint speed,const Cpu& cpu)
		{
			duty = 0;
			streamed = 0;

			if (speed == 0 || speed == cpu.GetFps())
				sync = 4;
			else
				sync = 0;
		}

		void Apu::Synchronizer::Reset(uint speed,dword sampleRate,const Cpu& cpu)
		{
			rate = sampleRate;
			Resync( speed, cpu );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_SINGLE_CALL dword Apu::Synchronizer::Clock(const dword output,const dword sampleRate,const Cpu& cpu)
		{
			/*if (sync)
			{
				if (duty >= 60*4)
					streamed += output;

				if (duty < 60*12)
				{
					duty++;
				}
				else
				{
					duty = 60*4;

					dword actualRate = streamed / (60*8) * cpu.GetFps();
					const dword limit = sampleRate / 21;

					if (actualRate <= sampleRate-limit)
					{
						actualRate = sampleRate-limit;
						sync--;
					}
					else if (actualRate >= sampleRate+limit)
					{
						actualRate = sampleRate+limit;
						sync--;
					}
					else
					{
						sync = (sync > 2 ? sync - 2 : 0);
					}

					actualRate = actualRate * 9999 / 10000;
					streamed = 0;

					if (rate != actualRate)
					{
						rate = actualRate;
						return actualRate;
					}
				}
			}*/

			return 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Channel::LengthCounter::LengthCounter()
		{
			Reset();
		}

		void Apu::Channel::LengthCounter::Reset()
		{
			enabled = 0;
			count = 0;
		}

		void Apu::Channel::LengthCounter::LoadState(State::Loader& state)
		{
			const uint data = state.Read8();
			enabled = (data == 0xFF ? 0U : ~0U);
			count = data & enabled;
		}

		void Apu::Channel::LengthCounter::SaveState(State::Saver& state,const dword chunk) const
		{
			NST_VERIFY( count < 0xFF );
			state.Begin( chunk ).Write8( enabled ? count : 0xFF ).End();
		}

		Apu::Channel::Envelope::Envelope()
		: outputVolume(OUTPUT_MUL)
		{
			Reset();
		}

		void Apu::Channel::Envelope::Reset()
		{
			output = 0;
			regs[0] = 0x0;
			regs[1] = 0x10;
			count = 0;
			reset = false;
		}

		void Apu::Channel::Envelope::SetOutputVolume(uint v)
		{
			outputVolume = v;
			UpdateOutput();
		}

		void Apu::Channel::Envelope::SaveState(State::Saver& state,const dword chunk) const
		{
			const byte data[3] =
			{
				count,
				static_cast<byte>(regs[0] | (reset ? 0x80U : 0x00U)),
				regs[1]
			};

			state.Begin( chunk ).Write( data ).End();
		}

		void Apu::Channel::Envelope::LoadState(State::Loader& state)
		{
			State::Loader::Data<3> data( state );

			count   = data[0] & 0x0F;
			reset   = data[1] >> 7;
			regs[0] = data[1] & 0x0F;
			regs[1] = data[2];

			UpdateOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Apu::Channel::Envelope::UpdateOutput()
		{
			output = (regs[regs[1] >> 4 & 1U] & 0xFUL) * outputVolume;
		}

		void Apu::Channel::Envelope::Clock()
		{
			if (!reset)
			{
				if (count)
				{
					count--;
					return;
				}

				if (regs[0] | (regs[1] & 0x20U))
					regs[0] = (regs[0] - 1U) & 0xF;
			}
			else
			{
				reset = false;
				regs[0] = 0xF;
			}

			count = regs[1] & 0x0FU;
			UpdateOutput();
		}

		void Apu::Channel::Envelope::Write(const uint data)
		{
			regs[1] = data;
			UpdateOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Channel::DcBlocker::DcBlocker()
		{
			Reset();
		}

		void Apu::Channel::DcBlocker::Reset()
		{
			acc = 0;
			prev = 0;
			next = 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Apu::Channel::Sample Apu::Channel::DcBlocker::Apply(Sample sample)
		{
			acc  -= prev;
			prev  = signed_shl(sample,15);
			acc  += prev - next * POLE;
			next  = signed_shr(acc,15);
			return next;
		}

		void Apu::Channel::DcBlocker::SaveState(State::Saver& state,const dword chunk) const
		{
			state.Begin( chunk );

			{
				const byte data[12] =
				{
					static_cast<byte>(acc & 0xFFU),
					static_cast<byte>(acc >> 8),
					static_cast<byte>(acc >> 16),
					static_cast<byte>(acc >> 24),
					static_cast<byte>(prev & 0xFFU),
					static_cast<byte>(prev >> 8),
					static_cast<byte>(prev >> 16),
					static_cast<byte>(prev >> 24),
					static_cast<byte>(next & 0xFFU),
					static_cast<byte>(next >> 8),
					static_cast<byte>(next >> 16),
					static_cast<byte>(next >> 24),
				};

				state.Begin( AsciiId<'S','0','0'>::V ).Write( data ).End();
			}

			state.End();
		}

		void Apu::Channel::DcBlocker::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'S','0','0'>::V:
					{
						State::Loader::Data<12> data( state );

						acc = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
						prev = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
						next = data[8] | (data[9] << 8) | (data[10] << 16) | (data[11] << 24);
						break;
					}
				}

				state.End();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Channel::Channel(Apu& a)
		: apu(a) {}

		Apu::Channel::~Channel()
		{
			if (apu.extChannel == this)
			{
				apu.extChannel = NULL;
				apu.UpdateVolumes();
			}
		}

		void Apu::Channel::Connect(bool audible)
		{
			NST_ASSERT( apu.extChannel == NULL );

			if (audible)
				apu.settings.audible = true;
			else
				apu.UpdateVolumes();

			apu.extChannel = this;
		}

		void Apu::Channel::GetOscillatorClock(Cycle& rate,uint& fixed) const
		{
			apu.CalculateOscillatorClock( rate, fixed );
		}

		uint Apu::Channel::GetVolume(uint channel) const
		{
			NST_ASSERT( channel < MAX_CHANNELS );
			return apu.settings.volumes[channel];
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Cycle Apu::Channel::GetCpuClockBase() const
		{
			return apu.cpu.GetClockBase();
		}

		uint Apu::Channel::GetCpuClockDivider() const
		{
			return apu.cpu.GetClockDivider();
		}

		Cycle Apu::Channel::GetCpuClock(uint clock) const
		{
			return apu.cpu.GetClock(clock);
		}

		dword Apu::Channel::GetSampleRate() const
		{
			return apu.settings.rate;
		}

		bool Apu::Channel::IsMuted() const
		{
			return apu.settings.muted;
		}

		void Apu::Channel::Update() const
		{
			apu.Update();
		}

		Cycle Apu::Channel::Clock(Cycle,Cycle,Cycle)
		{
			return Cpu::CYCLE_MAX;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Oscillator::Oscillator()
		: rate(1), fixed(1) {}

		void Apu::Oscillator::Reset()
		{
			active = false;
			timer = RESET_CYCLES * fixed;
			frequency = fixed;
			amp = 0;
		}

		inline void Apu::Oscillator::ClearAmp()
		{
			amp = 0;
		}

		void Apu::Oscillator::UpdateSettings(dword r,uint f)
		{
			NST_ASSERT( r && f );

			frequency = frequency / fixed * f;
			timer = timer / fixed * f;
			fixed = f;
			rate = r;
		}

		void Apu::Square::Reset()
		{
			Oscillator::Reset();

			frequency = fixed * 2;
			step = 0;
			duty = 0;

			envelope.Reset();
			lengthCounter.Reset();

			validFrequency = false;

			sweepRate = 0;
			sweepCount = 1;
			sweepReload = false;
			sweepIncrease = ~0U;
			sweepShift = 0;

			waveLength = 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline bool Apu::Square::CanOutput() const
		{
			return lengthCounter.GetCount() && envelope.Volume() && validFrequency;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Square::UpdateSettings(uint v,dword r,uint f)
		{
			Oscillator::UpdateSettings( r, f );
			envelope.SetOutputVolume( (v * Channel::OUTPUT_MUL + Channel::DEFAULT_VOLUME/2) / Channel::DEFAULT_VOLUME );
			active = CanOutput();
		}

		void Apu::Square::SaveState(State::Saver& state,const dword chunk) const
		{
			state.Begin( chunk );

			{
				byte data[4];

				data[0] = waveLength & 0xFFU;
				data[1] = (waveLength >> 8) | (duty ? duty << (2+3) : 2U << 3); // for version compatibility
				data[2] = (sweepCount - 1U) << 4;

				if (sweepRate)
					data[2] |= 0x08U | (sweepRate - 1);

				if (sweepReload)
					data[2] |= 0x80U;

				data[3] = sweepShift;

				if (!sweepIncrease)
					data[3] |= 0x08U;

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );
			envelope.SaveState( state, AsciiId<'E','N','V'>::V );

			{
				const byte data[13] = 
				{
					static_cast<byte>(step),
					static_cast<byte>(timer & 0xFFU),
					static_cast<byte>(timer >> 8),
					static_cast<byte>(timer >> 16),
					static_cast<byte>(timer >> 24),
					static_cast<byte>(frequency & 0xFFU),
					static_cast<byte>(frequency >> 8),
					static_cast<byte>(frequency >> 16),
					static_cast<byte>(frequency >> 24),
					static_cast<byte>(amp & 0xFFU),
					static_cast<byte>(amp >> 8),
					static_cast<byte>(amp >> 16),
					static_cast<byte>(amp >> 24),
				};

				state.Begin( AsciiId<'S','0','0'>::V ).Write( data ).End();
			}

			state.End();
		}

		void Apu::Square::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<4> data( state );

						waveLength = data[0] | (data[1] << 8 & 0x0700);

						// for version compatibility
						switch (data[1] >> 3 & 0xF)
						{
							case 4:  duty = 1; break;
							case 8:  duty = 2; break;
							case 12: duty = 3; break;
							default: duty = 0; break;
						}

						if (data[2] & 0x08)
							sweepRate = (data[2] & 0x07) + 1;
						else
							sweepRate = 0;

						sweepCount = (data[2] >> 4 & 0x07) + 1;
						sweepReload = data[2] >> 7;
						sweepShift = data[3] & 0x07;
						sweepIncrease = (data[3] & 0x08) ? 0U : ~0U;

						step = 0;
						timer = 0;
						break;
					}

					case AsciiId<'L','E','N'>::V:

						lengthCounter.LoadState( state );
						break;

					case AsciiId<'E','N','V'>::V:

						envelope.LoadState( state );

						UpdateFrequency();
						break;

					case AsciiId<'S','0','0'>::V:
					{
						State::Loader::Data<13> data( state );

						step = data[0];
						timer = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
						frequency = data[5] | (data[6] << 8) | (data[7] << 16) | (data[8] << 24);
						amp = data[9] | (data[10] << 8) | (data[11] << 16) | (data[12] << 24);
						break;
					}
				}

				state.End();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_SINGLE_CALL void Apu::Square::Disable(const bool disable)
		{
			active &= lengthCounter.Disable( disable );
		}

		void Apu::Square::UpdateFrequency()
		{
			if (waveLength >= MIN_FRQ && waveLength + (sweepIncrease & waveLength >> sweepShift) <= MAX_FRQ)
			{
				frequency = (waveLength + 1UL) * 2 * fixed;
				validFrequency = true;
				active = lengthCounter.GetCount() && envelope.Volume();
			}
			else
			{
				validFrequency = false;
				active = false;
			}
		}

		NST_SINGLE_CALL void Apu::Square::WriteReg0(const uint data)
		{
			envelope.Write( data );
			duty = data >> REG0_DUTY_SHIFT;
			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Square::WriteReg1(const uint data)
		{
			sweepIncrease = (data & REG1_SWEEP_DECREASE) ? 0U : ~0U;
			sweepShift = data & REG1_SWEEP_SHIFT;
			sweepRate = 0;

			if ((data & (REG1_SWEEP_ENABLED|REG1_SWEEP_SHIFT)) > REG1_SWEEP_ENABLED)
			{
				sweepRate = ((data & REG1_SWEEP_RATE) >> REG1_SWEEP_RATE_SHIFT) + 1;
				sweepReload = true;
			}

			UpdateFrequency();
		}

		NST_SINGLE_CALL void Apu::Square::WriteReg2(const uint data)
		{
			waveLength = (waveLength & uint(REG3_WAVELENGTH_HIGH)) | (data & REG3_WAVELENGTH_LOW);

			UpdateFrequency();
		}

		NST_SINGLE_CALL void Apu::Square::WriteReg3(const uint data,const Cycle frameCounterDelta)
		{
			step = 0;

			envelope.ResetClock();
			lengthCounter.Write( data, frameCounterDelta );

			waveLength = (data << 8 & REG3_WAVELENGTH_HIGH) | (waveLength & uint(REG3_WAVELENGTH_LOW));

			UpdateFrequency();
		}

		NST_SINGLE_CALL void Apu::Square::ClockEnvelope()
		{
			envelope.Clock();
			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Square::ClockSweep(const uint complement)
		{
			if (!envelope.Looping() && lengthCounter.Clock())
				active = false;

			if (sweepRate && !--sweepCount)
			{
				sweepCount = sweepRate;

				if (waveLength >= MIN_FRQ)
				{
					const uint shifted = waveLength >> sweepShift;

					if (!sweepIncrease)
					{
						waveLength += complement - shifted;
						UpdateFrequency();
					}
					else if (waveLength + shifted <= MAX_FRQ)
					{
						waveLength += shifted;
						UpdateFrequency();
					}
				}
			}

			if (sweepReload)
			{
				sweepReload = false;
				sweepCount = sweepRate;
			}
		}

		inline uint Apu::Square::GetLengthCounter() const
		{
			return lengthCounter.GetCount();
		}

		dword Apu::Square::GetLevel() const
		{
			static const byte forms[4][8] =
			{
				{0x1F,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F},
				{0x1F,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F},
				{0x1F,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F},
				{0x00,0x1F,0x1F,0x00,0x00,0x00,0x00,0x00}
			};

			return active ? (envelope.Volume() / Channel::OUTPUT_MUL) >> forms[duty][step] : 0;
		}

		dword Apu::Square::Remaining() const
		{
			/* A silenced square sits at zero whatever its phase is doing, so
			 * the walk never has to stop for it. Advance still runs the phase.
			*/
			return active ? dword(timer) : ~dword(0);
		}

		void Apu::Square::Advance(dword span)
		{
			timer -= idword(span);

			if (timer <= 0)
			{
				if (active)
				{
					do
					{
						step = (step + 1) & 0x7;
						timer += idword(frequency);
					}
					while (timer <= 0);
				}
				else
				{
					const dword count = dword(-timer) / frequency + 1;
					step = (step + count) & 0x7;
					timer += idword(count * frequency);
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Triangle::Triangle()
		: outputVolume(0) {}

		void Apu::Triangle::Reset()
		{
			Oscillator::Reset();

			step = 0x7;
			status = STATUS_COUNTING;
			waveLength = 0;
			//linearCtrl = 0;
			linearCounter = 0;

			lengthCounter.Reset();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline bool Apu::Triangle::CanOutput() const
		{
			return lengthCounter.GetCount() && linearCounter && waveLength >= MIN_FRQ && outputVolume;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Triangle::UpdateSettings(uint v,dword r,uint f)
		{
			Oscillator::UpdateSettings( r, f );

			outputVolume = (v * Channel::OUTPUT_MUL + Channel::DEFAULT_VOLUME/2) / Channel::DEFAULT_VOLUME;
			active = CanOutput();
		}

		void Apu::Triangle::SaveState(State::Saver& state,const dword chunk) const
		{
			state.Begin( chunk );

			{
				const byte data[4] =
				{
					static_cast<byte>(waveLength & 0xFFU),
					static_cast<byte>(waveLength >> 8),
					static_cast<byte>(linearCounter | (uint(status) << 7)),
					static_cast<byte>(linearCtrl)
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );

			{
				const byte data[9] =
				{
					static_cast<byte>(step),
					static_cast<byte>(timer & 0xFFU),
					static_cast<byte>(timer >> 8),
					static_cast<byte>(timer >> 16),
					static_cast<byte>(timer >> 24),
					static_cast<byte>(amp & 0xFFU),
					static_cast<byte>(amp >> 8),
					static_cast<byte>(amp >> 16),
					static_cast<byte>(amp >> 24),
				};

				state.Begin( AsciiId<'S','0','0'>::V ).Write( data ).End();
			}

			state.End();
		}

		void Apu::Triangle::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<4> data( state );

						waveLength = data[0] | (data[1] << 8 & 0x0700);
						linearCounter = data[2] & 0x7F;
						status = static_cast<Status>(data[2] >> 7);
						linearCtrl = data[3];

						frequency = (waveLength + 1UL) * fixed;
						timer = 0;
						step = 0;
						break;
					}

					case AsciiId<'L','E','N'>::V:

						lengthCounter.LoadState( state );
						break;

					case AsciiId<'S','0','0'>::V:
					{
						State::Loader::Data<9> data( state );

						step = data[0];
						timer = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
						amp = data[5] | (data[6] << 8) | (data[7] << 16) | (data[8] << 24);
						break;
					}
				}

				state.End();
			}

			active = CanOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_SINGLE_CALL void Apu::Triangle::Disable(const bool disable)
		{
			active &= lengthCounter.Disable( disable );
		}

		NST_SINGLE_CALL void Apu::Triangle::WriteReg0(const uint data)
		{
			linearCtrl = data;
		}

		NST_SINGLE_CALL void Apu::Triangle::WriteReg2(const uint data)
		{
			waveLength = (waveLength & uint(REG3_WAVE_LENGTH_HIGH)) | (data & REG2_WAVE_LENGTH_LOW);
			frequency = (waveLength + 1UL) * fixed;

			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Triangle::WriteReg3(const uint data,const Cycle frameCounterDelta)
		{
			waveLength = (data << 8 & REG3_WAVE_LENGTH_HIGH) | (waveLength & uint(REG2_WAVE_LENGTH_LOW));
			frequency = (waveLength + 1UL) * fixed;

			status = STATUS_RELOAD;
			lengthCounter.Write( data, frameCounterDelta );

			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Triangle::ClockLinearCounter()
		{
			if (status == STATUS_COUNTING)
			{
				if (linearCounter && !--linearCounter)
					active = false;
			}
			else
			{
				if (!(linearCtrl & uint(REG0_LINEAR_COUNTER_START)))
					status = STATUS_COUNTING;

				linearCounter = linearCtrl & uint(REG0_LINEAR_COUNTER_LOAD);
				active = CanOutput();
			}
		}

		NST_SINGLE_CALL void Apu::Triangle::ClockLengthCounter()
		{
			if (!(linearCtrl & uint(REG0_LINEAR_COUNTER_START)) && lengthCounter.Clock())
				active = false;
		}

		NST_SINGLE_CALL dword Apu::Triangle::GetLevel() const
		{
			/* amp is the last level the sequencer clocked out, and zero until
			 * it has clocked at all. Advance is the only thing that sets it.
			*/
			return outputVolume ? amp : 0;
		}

		NST_SINGLE_CALL dword Apu::Triangle::Remaining() const
		{
			return active ? dword(timer) : ~dword(0);
		}

		NST_SINGLE_CALL void Apu::Triangle::Advance(dword span)
		{
			if (!active)
				return;

			timer -= idword(span);

			while (timer <= 0)
			{
				static const byte pyramid[32] =
				{
					0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
					0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
					0xF,0xE,0xD,0xC,0xB,0xA,0x9,0x8,
					0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0
				};

				step = (step + 1) & 0x1F;
				amp = pyramid[step];
				timer += idword(frequency);
			}
		}

		inline uint Apu::Triangle::GetLengthCounter() const
		{
			return lengthCounter.GetCount();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Noise::Reset(const CpuModel model)
		{
			Oscillator::Reset();

			frequency = lut[model][0] * dword(fixed);

			bits = 1;
			shifter = 13;

			envelope.Reset();
			lengthCounter.Reset();
		}

		uint Apu::Noise::GetFrequencyIndex() const
		{
			for (uint v=frequency/fixed, i=0; i < 16; ++i)
			{
				if (v == lut[0][i] || v == lut[1][i])
					return i;
			}

			return 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline bool Apu::Noise::CanOutput() const
		{
			return lengthCounter.GetCount() && envelope.Volume();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Noise::UpdateSettings(uint v,dword r,uint f)
		{
			Oscillator::UpdateSettings( r, f );
			envelope.SetOutputVolume( (v * Channel::OUTPUT_MUL + Channel::DEFAULT_VOLUME/2) / Channel::DEFAULT_VOLUME );
			active = CanOutput();
		}

		void Apu::Noise::SaveState(State::Saver& state,const dword chunk) const
		{
			state.Begin( chunk );

			state.Begin( AsciiId<'R','E','G'>::V ).Write8( (shifter == 8 ? 0x10 : 0x00) | GetFrequencyIndex() ).End();
			lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );
			envelope.SaveState( state, AsciiId<'E','N','V'>::V );

			{
				const byte data[6] =
				{
					static_cast<byte>(bits & 0xFFU),
					static_cast<byte>(bits >> 8),
					static_cast<byte>(timer & 0xFFU),
					static_cast<byte>(timer >> 8),
					static_cast<byte>(timer >> 16),
					static_cast<byte>(timer >> 24)
				};

				state.Begin( AsciiId<'S','0','0'>::V ).Write( data ).End();
			}

			state.End();
		}

		void Apu::Noise::LoadState(State::Loader& state,const CpuModel model)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						const uint data = state.Read8();

						frequency = lut[model][data & 0x0F] * dword(fixed);
						shifter = (data & 0x10) ? 8 : 13;

						timer = 0;
						bits = 1;
						break;
					}

					case AsciiId<'L','E','N'>::V:

						lengthCounter.LoadState( state );
						break;

					case AsciiId<'E','N','V'>::V:

						envelope.LoadState( state );
						break;

					case AsciiId<'S','0','0'>::V:
					{
						State::Loader::Data<6> data( state );

						bits = data[0] | (data[1] << 8);
						timer = data[2] | (data[3] << 8) | (data[4] << 16) | (data[5] << 24);
						break;
					}
				}

				state.End();
			}

			active = CanOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_SINGLE_CALL void Apu::Noise::Disable(const bool disable)
		{
			active &= lengthCounter.Disable( disable );
		}

		NST_SINGLE_CALL void Apu::Noise::WriteReg0(const uint data)
		{
			envelope.Write( data );
			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Noise::WriteReg2(const uint data,const CpuModel model)
		{
			frequency = lut[model][data & REG2_FREQUENCY] * dword(fixed);
			shifter = (data & REG2_93BIT_MODE) ? 8 : 13;
		}

		NST_SINGLE_CALL void Apu::Noise::WriteReg3(const uint data,const Cycle frameCounterDelta)
		{
			envelope.ResetClock();
			lengthCounter.Write( data, frameCounterDelta );

			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Noise::ClockEnvelope()
		{
			envelope.Clock();
			active = CanOutput();
		}

		NST_SINGLE_CALL void Apu::Noise::ClockLengthCounter()
		{
			if (!envelope.Looping() && lengthCounter.Clock())
				active = false;
		}

		NST_SINGLE_CALL dword Apu::Noise::GetLevel() const
		{
			return (active && !(bits & 0x4000)) ? envelope.Volume() / Channel::OUTPUT_MUL : 0;
		}

		NST_SINGLE_CALL dword Apu::Noise::Remaining() const
		{
			return active ? dword(timer) : ~dword(0);
		}

		NST_SINGLE_CALL void Apu::Noise::Advance(dword span)
		{
			/* Every period feeds the shift register, so no bulk skip here.
			*/
			timer -= idword(span);

			while (timer <= 0)
			{
				bits = (bits << 1) | ((bits >> 14 ^ bits >> shifter) & 0x1);
				timer += idword(frequency);
			}
		}

		inline uint Apu::Noise::GetLengthCounter() const
		{
			return lengthCounter.GetCount();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Dmc::Dmc()
		: outputVolume(0)
		{
			frequency = GetResetFrequency( CPU_RP2A03 );
		}

		void Apu::Dmc::Reset(const CpuModel model)
		{
			loadClock = 0;
			enableClock = 0;
			curSample          = 0;
			linSample          = 0;
			frequency          = GetResetFrequency( model );
			regs.ctrl          = 0;
			regs.lengthCounter = 1;
			regs.address       = 0xC000;
			out.active         = false;
			lastLoadFetch      = 0;
			abortClock         = 0;
			out.shifter        = 0;
			out.dac            = 0;
			out.buffer         = 0x00;
			dma.lengthCounter  = 0;
			dma.buffered       = false;
			dma.address        = 0xC000;
			dma.buffer         = 0x00;
		}

		Cycle Apu::Dmc::GetResetFrequency(CpuModel model)
		{
			return lut[model][0];
		}

		void Apu::Dmc::UpdateSettings(uint v)
		{
			v = (v * Channel::OUTPUT_MUL + Channel::DEFAULT_VOLUME/2) / Channel::DEFAULT_VOLUME;

			if (outputVolume)
				linSample /= outputVolume;

			if (outputVolume)
				curSample /= outputVolume;

			linSample *= v;
			curSample *= v;
			outputVolume = v;

			if (!v)
				out.active = false;
		}

		inline void Apu::Dmc::ClearAmp()
		{
			curSample = 0;
			linSample = 0;
		}

		void Apu::Dmc::SaveState(State::Saver& state,const dword chunk,const Cpu& cpu,const Cycle dmcMcClock) const
		{
			NST_VERIFY( dmcMcClock >= cpu.GetCycles() );

			dword dmcClock = dmcMcClock;

			if (dmcClock > cpu.GetCycles())
				dmcClock = (dmcClock - cpu.GetCycles()) / cpu.GetClock();
			else
				dmcClock = 0;

			NST_VERIFY( dmcClock <= 0x1FFF && dmcMcClock == cpu.GetCycles() + dmcClock * cpu.GetClock() );

			state.Begin( chunk );

			{
				const byte data[12] =
				{
					static_cast<byte>(dmcClock & 0xFF),
					static_cast<byte>(dmcClock >> 8),
					static_cast<byte>((
						( ( regs.ctrl & REG0_FREQUENCY  )              ) |
						( ( regs.ctrl & REG0_LOOP       ) ? 0x10U : 0U ) |
						( ( regs.ctrl & REG0_IRQ_ENABLE ) ? 0x20U : 0U ) |
						( ( dma.lengthCounter           ) ? 0x40U : 0U )
					)),
					static_cast<byte>((regs.address - 0xC000U) >> 6),
					static_cast<byte>((regs.lengthCounter - 1U) >> 4),
					static_cast<byte>((dma.address >> 0 & 0xFFU)),
					static_cast<byte>((dma.address >> 8 & 0x7FU) | (dma.buffered ? 0x80 : 0x00)),
					static_cast<byte>(dma.lengthCounter ? (dma.lengthCounter - 1U) >> 4 : 0),
					static_cast<byte>(dma.buffer),
					static_cast<byte>(7 - out.shifter),
					out.buffer,
					out.dac,
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			{
				/* Pending DMA scheduling state, as small biased cycle deltas
				 * from the current CPU cycle. A zero byte encodes none/inert;
				 * builds without this chunk skip it.
				*/
				byte data[3] = {0,0,0};

				const Cycle now = cpu.GetCycles();
				const long bias = 8;

				if (loadClock)
				{
					const long d = ((long) loadClock - (long) now) / (long) cpu.GetClock() + bias;
					if (d >= 1 && d <= 255) data[0] = static_cast<byte>(d);
				}

				if (abortClock)
				{
					const long d = ((long) abortClock - (long) now) / (long) cpu.GetClock() + bias;
					if (d >= 1 && d <= 255) data[1] = static_cast<byte>(d);
				}

				if (lastLoadFetch)
				{
					const long d = ((long) lastLoadFetch - (long) now) / (long) cpu.GetClock() + bias;
					if (d >= 1 && d <= 255 && d <= bias + 8) data[2] = static_cast<byte>(d);
				}

				state.Begin( AsciiId<'D','M','A'>::V ).Write( data ).End();
			}

			{
				/* The $4015 enable in flight, same encoding. Its own chunk rather
				 * than a fourth byte on DMA, so a state written before this still
				 * parses and one written after loads in a build without it.
				*/
				byte data[1] = {0};

				if (enableClock)
				{
					const long bias = 8;
					const long d = ((long) enableClock - (long) cpu.GetCycles()) / (long) cpu.GetClock() + bias;
					if (d >= 1 && d <= 255) data[0] = static_cast<byte>(d);
				}

				state.Begin( AsciiId<'D','M','E'>::V ).Write( data ).End();
			}

			{
				const byte data[4] =
				{
					static_cast<byte>(linSample & 0xFFU),
					static_cast<byte>(linSample >> 8),
					static_cast<byte>(dma.lengthCounter & 0xFFU),
					static_cast<byte>(dma.lengthCounter >> 8),
				};

				state.Begin( AsciiId<'S','0','0'>::V ).Write( data ).End();
			}
			
			state.End();
		}

		void Apu::Dmc::LoadState(State::Loader& state,const Cpu& cpu,const CpuModel model,Cycle& dmcClock)
		{
			// Defaults for states without the DMA chunk: nothing pending.
			loadClock = 0;
			lastLoadFetch = 0;
			abortClock = 0;
			enableClock = 0;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'D','M','A'>::V:
					{
						State::Loader::Data<3> data( state );

						const Cycle now = cpu.GetCycles();
						const long bias = 8;

						if (data[0])
							loadClock = now + ((long) data[0] - bias) * (long) cpu.GetClock();

						if (data[1])
							abortClock = now + ((long) data[1] - bias) * (long) cpu.GetClock();

						if (data[2])
							lastLoadFetch = now + ((long) data[2] - bias) * (long) cpu.GetClock();

						break;
					}

					case AsciiId<'D','M','E'>::V:
					{
						State::Loader::Data<1> data( state );

						if (data[0])
							enableClock = cpu.GetCycles() + ((long) data[0] - 8) * (long) cpu.GetClock();

						break;
					}

					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<12> data( state );

						dmcClock = cpu.GetCycles() + ((data[0] | data[1] << 8) * cpu.GetClock());

						regs.ctrl =
						(
							( ( data[2] & 0x10 ) ? REG0_LOOP       : 0U ) |
							( ( data[2] & 0x20 ) ? REG0_IRQ_ENABLE : 0U ) |
							( ( data[2] & REG0_FREQUENCY )              )
						);

						frequency          = lut[model][regs.ctrl & REG0_FREQUENCY];
						regs.address       = 0xC000 | (data[3] << 6);
						regs.lengthCounter = (data[4] << 4) + 1;
						dma.address        = 0x8000 | data[5] | (data[6] << 8 & 0x7F00);
						dma.buffered       = data[6] >> 7;
						dma.lengthCounter  = (data[2] & 0x40) ? (data[7] << 4) + 1 : 0;
						dma.buffer         = data[8];
						out.shifter        = 7 - (data[9] & 0x7);
						out.buffer         = data[10];
						out.dac            = data[11] & 0x7F;

						curSample = out.dac * outputVolume;
						linSample = curSample;
						out.active = dma.buffered && outputVolume;
						break;
					}

					case AsciiId<'S','0','0'>::V:
					{
						State::Loader::Data<4> data( state );

						linSample = data[0] | (data[1] << 8);
						dma.lengthCounter = data[2] | (data[3] << 8);
						break;
					}
				}

				state.End();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Apu::Dmc::RebaseFrame(const Cycle frame)
		{
			// Pending DMA scheduling cycles live in the same timebase as the
			// DMC clock and must survive the end-of-frame rebase with it.
			if (loadClock)
				loadClock -= NST_MIN( loadClock, frame );

			if (abortClock)
				abortClock -= NST_MIN( abortClock, frame );

			if (enableClock)
				enableClock -= NST_MIN( enableClock, frame );

			lastLoadFetch -= NST_MIN( lastLoadFetch, frame );
		}

		NST_SINGLE_CALL void Apu::Dmc::Disable(const bool disable,Cpu& cpu,const Cycle nextDmcClock)
		{
			cpu.ClearIRQ( Cpu::IRQ_DMC );

			if (disable)
			{
				/* Explicit DMA abort: a DMA committed by the cycle after this
				 * write (clock passed or load scheduled) has already asserted
				 * RDY, and releasing it still costs the CPU one cycle.
				*/
				bool committed;

				if (loadClock)
					committed = (loadClock <= cpu.GetCycles() + cpu.GetClock());
				else
					committed = (out.shifter == 0 && dma.buffered && dma.lengthCounter && nextDmcClock <= cpu.GetCycles() + cpu.GetClock());

				if (committed)
					cpu.StealCycles( cpu.GetClock() );

				dma.lengthCounter = 0;
				loadClock = 0;
				abortClock = 0;
			}
			else if (!dma.lengthCounter)
			{
				dma.lengthCounter = regs.lengthCounter;
				dma.address = regs.address;

				/* The channel is not enabled on the cycle of the write. Until it
				 * is, a transfer the output timer brings due is neither dropped
				 * nor run on schedule: it re-attempts every cycle and starts on
				 * the first one where the channel is enabled.
				*/
				enableClock = cpu.GetCycles() + cpu.GetClock( DMC_ENABLE_DELAY );

				if (!dma.buffered)
					ScheduleLoadDMA( cpu, nextDmcClock );
			}
		}

		void Apu::Dmc::ScheduleLoadDMA(Cpu& cpu,const Cycle putAnchor)
		{
			/* "The first, 'load' DMC DMA after the $4015 write attempts to
			 * halt on the get cycle during the 2nd following APU cycle."
			 * (nesdev.org/wiki/DMA)
			 *
			 * An APU cycle is a [get,put] pair, so the halt target is pinned
			 * to the get grid and depends only on which half of its APU cycle
			 * the write landed in:
			 *
			 *   write on a get cycle -> halt 4 CPU cycles later
			 *   write on a put cycle -> halt 3 CPU cycles later
			 *
			 * The DMC clock supplies the grid. It is locked to the put phase
			 * (a reload halts at clock + 2, on a put, per the same source),
			 * so a write sharing its parity is itself on a put cycle - which
			 * is what IsDmaPutCycle reports. DoDMA() derives the halt as
			 * clock + 2, so loadClock is the halt minus two.
			*/
			const Cycle span = (putAnchor >= cpu.GetCycles()) ?
				(putAnchor - cpu.GetCycles()) : (cpu.GetCycles() - putAnchor);

			const uint haltDelay = (((span / cpu.GetClock()) & 1) == 0) ? 3 : 4;

			loadClock = cpu.GetCycles() + cpu.GetClock( haltDelay - 2 );

			cpu.WakeAt( loadClock + cpu.GetClock(2) );
		}

		void Apu::Dmc::ClockImplicitAbort(Cpu& cpu)
		{
			/* The spurious DMA is aborted one cycle after being triggered:
			 * the single halted CPU cycle lands on the DMC clock itself.
			*/
			const Cycle halt = abortClock;
			abortClock = 0;

			if (!cpu.IsWriteCycle( halt ))
				cpu.StealCycles( cpu.GetClock() );
		}

		void Apu::Dmc::ClockLoadDMA(Cpu& cpu,const uint readAddress,const uint haltParity,const Cycle nextDmcClock)
		{
			const Cycle clock = loadClock;
			loadClock = 0;

			if (!dma.buffered && dma.lengthCounter)
			{
				DoDMA( cpu, clock, readAddress, haltParity );

				// The sample fetch lands two cycles after the halt (halt + dummy + fetch).
				lastLoadFetch = clock + cpu.GetClock(4);

				/* Implicit DMA abort: a final-byte load (no looping) completing
				 * right before the shifter empties triggers a spurious DMA that
				 * is aborted one cycle later, halting the CPU for one cycle.
				 * Unlike a normal DMA it is never delayed by a write cycle: it
				 * simply doesn't happen.
				*/
				if (!dma.lengthCounter && !(regs.ctrl & REG0_LOOP) && out.shifter == 0 &&
					nextDmcClock >= lastLoadFetch && nextDmcClock < lastLoadFetch + cpu.GetClock(2))
				{
					abortClock = nextDmcClock;
					cpu.WakeAt( abortClock + cpu.GetClock() );
				}
			}
		}

		NST_SINGLE_CALL dword Apu::Dmc::GetLevel() const
		{
			/* Straight off curSample. linSample is retained for the save state
			 * only; slewing the level would distort what the DAC reads out.
			*/
			return outputVolume ? curSample / outputVolume : 0;
		}

		Cycle Apu::Dmc::DoDMA(Cpu& cpu,const Cycle clock,const uint readAddress,const uint haltParity)
		{
			NST_VERIFY( !dma.buffered );

			/* DMC DMA adds:
			 * case 1: 4 cycles normally
			 * case 2: 3 if it lands on a CPU write
			 * case 3: 2 if it lands on the $4014 write or during OAM DMA
			 * case 4: 1 if on the next-to-next-to-last DMA cycle
			 * case 5: 3 if on the last DMA cycle
			 * https://forums.nesdev.org/viewtopic.php?f=3&t=6100
			 * https://www.nesdev.org/wiki/DMA
			*/
			// The CPU halt takes effect 2 CPU cycles after the DMC output unit clocks.
			const Cycle haltClock = clock + cpu.GetClock(2);

			/* RDY is only sampled on read cycles: consecutive CPU write cycles
			 * (e.g. JSR's stack pushes) delay the halt. During an OAM DMA the
			 * CPU executes no write cycles of its own.
			*/
			uint haltDelay = 0;

			/* A transfer held for the enable is delayed exactly as one held by a
			 * run of CPU write cycles: the halt moves and its parity moves with
			 * it. That parity is what the ROM is measuring - it says a one-cycle
			 * hold "will be 3 CPU cycles long instead of the typical 4", which is
			 * what (haltParity + haltDelay) & 1 produces below. Feeding the hold
			 * in as a shifted clock instead leaves haltDelay at zero, moves the
			 * halt but not the parity, and the transfer still steals four.
			 *
			 * Only a transfer coming due around the write is held; one landing
			 * well after it has nothing to wait for.
			*/
			if (enableClock)
			{
				if (haltClock < enableClock && enableClock - haltClock <= cpu.GetClock(3))
				{
					while (haltDelay < 3 && haltClock + cpu.GetClock() * haltDelay < enableClock)
						++haltDelay;
				}

				enableClock = 0;
			}

			if (!cpu.GetOamDMA())
			{
				while (haltDelay < 3 && cpu.IsWriteCycle( haltClock + cpu.GetClock() * haltDelay ))
					++haltDelay;
			}

			/* Reloads take halt + dummy + alignment + get (4) from their normal
			 * put-phase halt, one less when a write delay flips the parity.
			*/
			uint cyclesToSteal = ((haltParity + haltDelay) & 1) ? 3 : 4;

			/* Loads always take halt + dummy + fetch (3): the transfer start
			 * delay already aligned the sequence, write delays don't add the
			 * alignment cycle back.
			*/
			if (haltParity)
				cyclesToSteal = 3;

			if (cpu.GetOamDMA())
			{
				if (cpu.GetOamDMACycle() == 255)
				{
					cyclesToSteal = 3;
				}
				else if (cpu.GetOamDMACycle() == 254)
				{
					cyclesToSteal = 1;
				}
				else
				{
					cyclesToSteal = 2;
				}
			}

			const bool collision = readAddress && cpu.GetCycles() == haltClock && !cpu.IsWriteCycle(haltClock);

			if (collision)
			{
				NST_DEBUG_MSG("DMA/Read conflict!");

				/* A CPU halted on one of its read cycles keeps repeating that
				 * read on each halt/dummy/alignment cycle. On $4016/$4017 the
				 * controller clock filters consecutive reads into one (NES and
				 * AV Famicom behavior).
				*/
				uint repeats = cyclesToSteal ? cyclesToSteal - 1 : 0;

				if ((readAddress & 0xFFFE) == 0x4016)
					repeats = 1;

				while (repeats--)
					cpu.PeekBus( readAddress );
			}

			cpu.StealCycles( cpu.GetClock() * cyclesToSteal);

			const uint sampleAddress = dma.address;

			/* Only a DMA halting the CPU on this very read cycle leaves the
			 * sample byte on the bus; a DMA processed late has already had
			 * the bus re-driven by the fetches that followed it.
			*/
			dma.buffer = cpu.Peek( sampleAddress );

			// The fetch drives the external bus only - it never reaches the
			// CPU's internal bus, so it cannot show up in a $4015 read.
			if (collision)
				cpu.SetBusData( dma.buffer );

			/* Get-cycle bus conflict: with the halted CPU's address in
			 * $4000-$401F, the register select lines follow the DMA address
			 * low bits, so $4000|(sampleAddress&$1F) is read on the same
			 * cycle as the sample fetch.
			*/
			if (collision && (readAddress & 0xFFE0) == 0x4000)
			{
				const uint reg = 0x4000 | (sampleAddress & 0x1F);

				if (reg == 0x4015 || reg == 0x4016 || reg == 0x4017)
					cpu.PeekBus( reg );
			}

			dma.address = 0x8000 | ((sampleAddress + 1U) & 0x7FFF);
			dma.buffered = true;

			/* The sample fetch is the last of the stolen cycles: halt, dummy,
			 * (alignment,) fetch.  The DMC IRQ is asserted there, so it must
			 * be timestamped with that cycle rather than wherever the CPU
			 * happens to have reached when the DMA got processed - the hook
			 * runs at an instruction boundary, which is always later.
			 *
			 * GetClock(n) indexes cycles.clock[n-1], so cyclesToSteal must be
			 * at least 2 before subtracting one from it.  It is 1 in the
			 * end-of-OAM-DMA case, where the fetch is the halt cycle itself.
			*/
			const Cycle fetchClock = (cyclesToSteal > 1) ?
				haltClock + cpu.GetClock( cyclesToSteal - 1 ) : haltClock;

			NST_VERIFY( dma.lengthCounter );

			if (!--dma.lengthCounter)
			{
				if (regs.ctrl & REG0_LOOP)
				{
					dma.address = regs.address;
					dma.lengthCounter = regs.lengthCounter;
				}
				else if (regs.ctrl & REG0_IRQ_ENABLE)
				{
					cpu.DoIRQ( Cpu::IRQ_DMC, fetchClock );
				}
			}

			return fetchClock;
		}

		NST_SINGLE_CALL bool Apu::Dmc::WriteReg0(const uint data,const CpuModel model)
		{
			regs.ctrl = data;
			frequency = lut[model][data & REG0_FREQUENCY];
			return data & REG0_IRQ_ENABLE;
		}

		NST_SINGLE_CALL void Apu::Dmc::WriteReg1(const uint data)
		{
			out.dac = data & 0x7F;
			curSample = out.dac * outputVolume;
		}

		NST_SINGLE_CALL void Apu::Dmc::WriteReg2(const uint data)
		{
			regs.address = 0xC000 | (data << 6);
		}

		NST_SINGLE_CALL void Apu::Dmc::WriteReg3(const uint data)
		{
			regs.lengthCounter = (data << 4) + 1;
		}

		NST_SINGLE_CALL bool Apu::Dmc::ClockDAC()
		{
			if (out.active)
			{
				const uint next = out.dac + ((out.buffer & 0x1U) << 2) - 2;
				out.buffer >>= 1;

				if (next <= 0x7F && next != out.dac)
				{
					out.dac = next;
					return true;
				}
			}

			return false;
		}

		NST_SINGLE_CALL void Apu::Dmc::Update()
		{
			curSample = out.dac * outputVolume;
		}

		NST_SINGLE_CALL void Apu::Dmc::ClockDMA(Cpu& cpu,Cycle& clock,const uint readAddress)
		{
			const Cycle tmp = clock;
			clock += frequency;

			if (out.shifter)
			{
				out.shifter--;
			}
			else
			{
				out.shifter = 7;
				out.active = dma.buffered;

				if (out.active)
				{
					out.active = outputVolume;
					dma.buffered = false;
					out.buffer = dma.buffer;

					if (dma.lengthCounter)
						DoDMA( cpu, tmp, readAddress );
				}
			}
		}

		inline uint Apu::Dmc::GetLengthCounter() const
		{
			return dma.lengthCounter;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::ClearBuffers()
		{
			ClearBuffers( true );
		}

		NST_NO_INLINE void Apu::ClearBuffers(bool resync)
		{
			if (resync)
				synchronizer.Resync( settings.speed, cpu );

			square[0].ClearAmp();
			square[1].ClearAmp();
			triangle.ClearAmp();
			noise.ClearAmp();
			dmc.ClearAmp();

			dcBlocker.Reset();

			buffer.Reset( false );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Apu::IsDmaPutCycle(const Cycle clock) const
		{
			// The DMC clock is locked to one phase of the APU get/put pattern
			// and serves as the grid reference for DMA alignment.
			const Cycle span = (cycles.dmcClock >= clock) ? (cycles.dmcClock - clock) : (clock - cycles.dmcClock);

			return ((span / cpu.GetClock()) & 1) == 0;
		}

		void Apu::ClockPendingLoad(const Cycle target,const uint readAddress)
		{
			if (dmc.HasPendingLoad() && dmc.GetLoadClock() + cpu.GetClock(2) <= target)
			{
				const Cycle halt = dmc.GetLoadClock() + cpu.GetClock(2);

				/* A DMC output clock due at or before the load's sample fetch
				 * happens first: an empty buffer at that clock stays empty
				 * rather than seeing the not-yet-fetched sample byte.
				*/
				if (cycles.dmcClock < halt + cpu.GetClock(2))
					ClockDmc( target );

				dmc.ClockLoadDMA( cpu, readAddress, 1, cycles.dmcClock );
			}
		}

		Cycle Apu::Clock()
		{
			// DMAs are processed (and the CPU woken) at their halt time, 2
			// CPU cycles after the DMC clock, rather than at the clock itself.
			// A halt landing exactly on the upcoming cycle is left for that
			// cycle's own memory access to claim, so a halted read observes
			// the DMA's sample fetch on the bus.
			ClockPendingLoad( cpu.GetCycles(), 0 );

			if (cycles.dmcClock + cpu.GetClock(2) < cpu.GetCycles())
				ClockDmc( cpu.GetCycles() );

			if (cycles.frameIrqClock <= cpu.GetCycles())
				ClockFrameIRQ( cpu.GetCycles() );

			// Evaluated only once the halt cycle has fully passed, so the
			// write-cycle test sees the instruction that executed on it.
			if (dmc.GetAbortClock() && dmc.GetAbortClock() + cpu.GetClock() <= cpu.GetCycles())
				dmc.ClockImplicitAbort( cpu );

			Cycle next = NST_MIN(cycles.dmcClock + cpu.GetClock(2),cycles.frameIrqClock);

			if (dmc.HasPendingLoad())
				next = NST_MIN(next,dmc.GetLoadClock() + cpu.GetClock(2));

			if (dmc.GetAbortClock())
				next = NST_MIN(next,dmc.GetAbortClock() + cpu.GetClock(2));

			/* A DMC halt landing exactly on the upcoming cycle is skipped
			 * above on purpose - that cycle's own memory access claims it via
			 * ClockDMA, which is what lets the halted read observe the sample
			 * fetch on the bus. Scheduling the wakeup for that same cycle
			 * would hand back a target the CPU has already reached, so the
			 * scheduler re-enters with no progress made. Step past it; the
			 * pending DMA is still claimed by ClockDMA or the next Clock.
			*/
			if (next <= cpu.GetCycles())
				next = cpu.GetCycles() + cpu.GetClock();

			return next;
		}

		void Apu::ClockDMA(uint readAddress)
		{
			ClockPendingLoad( cpu.GetCycles(), readAddress );

			if (cycles.dmcClock + cpu.GetClock(2) <= cpu.GetCycles())
				ClockDmc( cpu.GetCycles(), readAddress );
		}

		NST_NO_INLINE void Apu::ClockOscillators(const bool twoClocks)
		{
			for (uint i=0; i < 2; ++i)
				square[i].ClockEnvelope();

			triangle.ClockLinearCounter();
			noise.ClockEnvelope();

			if (twoClocks)
			{
				for (uint i=0; i < 2; ++i)
					square[i].ClockSweep( i-1 );

				triangle.ClockLengthCounter();
				noise.ClockLengthCounter();
			}
		}

		NST_NO_INLINE void Apu::ClockDmc(const Cycle target,const uint readAddress)
		{
			NST_ASSERT( cycles.dmcClock + cpu.GetClock(2) <= target );

			do
			{
				if (dmc.ClockDAC())
				{
					Update( cycles.dmcClock );
					dmc.Update();
				}

				dmc.ClockDMA( cpu, cycles.dmcClock, readAddress );
			}
			while (cycles.dmcClock + cpu.GetClock(2) <= target);
		}

		NST_NO_INLINE void Apu::ClockFrameCounter()
		{
			NST_COMPILE_ASSERT( STATUS_SEQUENCE_5_STEP == 0x80 );
			NST_VERIFY( cycles.frameCounter <= cpu.GetCycles() * cycles.fixed );

			ClockOscillators( cycles.frameDivider & 0x1U );

			cycles.frameDivider = (cycles.frameDivider + 1) & 0x3U;
			cycles.frameCounter += Cycles::oscillatorClocks[cpu.GetModel()][ctrl >> 7][cycles.frameDivider] * cycles.fixed;
		}

		NST_NO_INLINE void Apu::ClockFrameIRQ(const Cycle target)
		{
			NST_VERIFY( !(ctrl & STATUS_SEQUENCE_5_STEP) );

			if (ctrl & STATUS_NO_FRAME_IRQ)
			{
				/* Inhibited: $4015.6 still reads set on the first two cycles of
				 * the three, and the third is where the inhibit takes effect.
				 * The IRQ line is never pulled.
				*/
				if (cycles.frameIrqRepeat % 3 == 0)
					cycles.frameIrqPhantom = cycles.frameIrqClock + cpu.GetClock(2);
			}
			else
			{
				cpu.DoIRQ( Cpu::IRQ_FRAME, cycles.frameIrqClock );
			}

			Cycle clock = cycles.frameIrqClock;
			uint repeat = cycles.frameIrqRepeat;

			do
			{
				clock += Cycles::frameClocks[cpu.GetModel()][1 + repeat++ % 3];
			}
			while (clock <= target);

			cycles.frameIrqClock = clock;
			cycles.frameIrqRepeat = repeat;
		}

		void Apu::UpdateMixLut()
		{
			for (uint i=0; i < 31; ++i)
			{
				const dword dac = i * dword(Channel::OUTPUT_MUL);
				lutPulse[i] = dac ? NLN_SQ_0 / (NLN_SQ_1 / dac + NLN_SQ_2) : 0;
			}

			for (uint i=0; i < 203; ++i)
			{
				const dword dac = i * dword(Channel::OUTPUT_MUL);
				lutTnd[i] = dac ? NLN_TND_0 / (NLN_TND_1 / dac + NLN_TND_2) : 0;
			}
		}

		NST_NO_INLINE Apu::Channel::Sample Apu::GetSample()
		{
			/* Both DACs are non-linear, and f(mean(x)) is not mean(f(x)), so
			 * the channels have to be mixed at full rate and averaged after.
			 * Walk the sample period one transition at a time, take the mixed
			 * level over each span and weight it by the length of the span.
			 * The walk stops only where a channel changes.
			 *
			 * The DMC DAC is driven from the CPU side and cannot move within
			 * one output sample, so it is read once.
			*/
			const dword level = dmc.GetLevel();

			qaword sum = 0;
			dword left = cycles.rate;

			do
			{
				dword span = left;

				span = NST_MIN( span, square[0].Remaining() );
				span = NST_MIN( span, square[1].Remaining() );
				span = NST_MIN( span, triangle.Remaining() );
				span = NST_MIN( span, noise.Remaining() );

				sum += qaword
				(
					lutPulse[ square[0].GetLevel() + square[1].GetLevel() ] +
					lutTnd  [ triangle.GetLevel() * 3 + noise.GetLevel() * 2 + level ]
				) * span;

				square[0].Advance( span );
				square[1].Advance( span );
				triangle.Advance( span );
				noise.Advance( span );

				left -= span;
			}
			while (left);

			return Clamp<Channel::OUTPUT_MIN,Channel::OUTPUT_MAX>
			(
				dcBlocker.Apply( Channel::Sample(sum / cycles.rate) ) +
				(extChannel ? extChannel->GetSample() : 0)
			);
		}

		NES_POKE_AD(Apu,4000)
		{
			UpdateLatency();
			square[address >> 2 & 0x1].WriteReg0( data );
		}

		NES_POKE_AD(Apu,4001)
		{
			Update();
			square[address >> 2 & 0x1].WriteReg1( data );
		}

		NES_POKE_AD(Apu,4002)
		{
			Update();
			square[address >> 2 & 0x1].WriteReg2( data );
		}

		NES_POKE_AD(Apu,4003)
		{
			square[address >> 2 & 0x1].WriteReg3( data, UpdateDelta() );
		}

		NES_POKE_D(Apu,4008)
		{
			Update();
			triangle.WriteReg0( data );
		}

		NES_POKE_D(Apu,400A)
		{
			Update();
			triangle.WriteReg2( data );
		}

		NES_POKE_D(Apu,400B)
		{
			triangle.WriteReg3( data, UpdateDelta() );
		}

		NES_POKE_D(Apu,400C)
		{
			UpdateLatency();
			noise.WriteReg0( data );
		}

		NES_POKE_D(Apu,400E)
		{
			Update();
			noise.WriteReg2( data, cpu.GetModel() );
		}

		NES_POKE_D(Apu,400F)
		{
			noise.WriteReg3( data, UpdateDelta() );
		}

		NES_POKE_D(Apu,4010)
		{
			if (!dmc.WriteReg0( data, cpu.GetModel() ))
				cpu.ClearIRQ( Cpu::IRQ_DMC );
		}

		NES_POKE_D(Apu,4011)
		{
			Update();
			dmc.WriteReg1( data );
		}

		NES_POKE_D(Apu,4012)
		{
			dmc.WriteReg2( data );
		}

		NES_POKE_D(Apu,4013)
		{
			dmc.WriteReg3( data );
		}

		NES_POKE_D(Apu,4015)
		{
			// A DMA halting at or before the cycle after this write completes
			// in full before the write's effect reaches the DMA unit.
			cpu.Update();

			ClockPendingLoad( cpu.GetCycles() + cpu.GetClock(), 0 );

			if (cycles.dmcClock + cpu.GetClock(2) <= cpu.GetCycles() + cpu.GetClock())
				ClockDmc( cpu.GetCycles() + cpu.GetClock() );

			Update();

			data = ~data;

			square[0].Disable ( data >> 0 & 0x1  );
			square[1].Disable ( data >> 1 & 0x1  );
			triangle.Disable  ( data >> 2 & 0x1  );
			noise.Disable     ( data >> 3 & 0x1  );
			dmc.Disable       ( data & 0x10, cpu, cycles.dmcClock );
		}

		NES_PEEK_A(Apu,4015)
		{
			NST_COMPILE_ASSERT( Cpu::IRQ_FRAME == 0x40 && Cpu::IRQ_DMC == 0x80 );

			const Cycle elapsed = cpu.Update( address );

			if (cycles.frameIrqClock <= elapsed)
				ClockFrameIRQ( elapsed );

			if (cycles.frameCounter < elapsed * cycles.fixed)
				Update( elapsed );

			uint data = cpu.GetIRQ();
			cpu.ClearIRQ( Cpu::IRQ_FRAME );

			/* The frame counter flag is cleared on a put-to-get transition of
			 * the APU cycle: a read landing on a get cycle still observes the
			 * flag on the immediately following read.
			*/
			if (data & Cpu::IRQ_FRAME)
			{
				cycles.frameIrqHold = !IsDmaPutCycle( cpu.GetCycles() ) ? cpu.GetCycles() + cpu.GetClock(2) : 0;
			}
			else if (cpu.GetCycles() < cycles.frameIrqHold)
			{
				data |= Cpu::IRQ_FRAME;
				cycles.frameIrqHold = 0;
			}
			else if (cpu.GetCycles() < cycles.frameIrqPhantom)
			{
				data |= Cpu::IRQ_FRAME;
			}

			return (data & (Cpu::IRQ_FRAME|Cpu::IRQ_DMC)) |
			(
				( square[0].GetLengthCounter() ? 0x01U : 0x00U ) |
				( square[1].GetLengthCounter() ? 0x02U : 0x00U ) |
				( triangle.GetLengthCounter()  ? 0x04U : 0x00U ) |
				( noise.GetLengthCounter()     ? 0x08U : 0x00U ) |
				( dmc.GetLengthCounter()       ? 0x10U : 0x00U ) |
				( cpu.GetInternalBusData()     & 0x20U )
			);
		}

		void Apu::WriteFrameCtrl(uint data)
		{
			Cycle next = cpu.Update();

			if (cpu.IsOddCycle())
				next += cpu.GetClock();

			Update( next );

			if (cycles.frameIrqClock <= next)
				ClockFrameIRQ( next );

			next += cpu.GetClock();

			data &= STATUS_BITS;

			cycles.frameCounter = (next + Cycles::oscillatorClocks[cpu.GetModel()][data >> 7][0]) * cycles.fixed;
			cycles.frameDivider = 0;
			cycles.frameIrqRepeat = 0;

			ctrl = data;

			if (data & STATUS_NO_FRAME_IRQ)
				cpu.ClearIRQ( Cpu::IRQ_FRAME );

			if (data & STATUS_SEQUENCE_5_STEP)
			{
				cycles.frameIrqClock = Cpu::CYCLE_MAX;
				ClockOscillators( true );
			}
			else
			{
				// 4-step still runs the sequence when interrupts are inhibited;
				// the flag sets for two cycles even though the line is not pulled
				cycles.frameIrqClock = next + Cycles::frameClocks[cpu.GetModel()][0];
			}
		}

		NES_PEEK_A(Apu,40xx)
		{
			// Write-only / unmapped register: open bus. Process any DMA that
			// lands on this read first, since the DMA's sample fetch drives
			// the data bus that this read will observe.
			cpu.Update( address );
			return cpu.GetBusData();
		}
	}
}
