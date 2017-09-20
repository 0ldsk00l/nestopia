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
#include "NstPpu.hpp"
#include "NstState.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		const byte Ppu::yuvMaps[4][0x40] =
		{
			{
				0x35, 0x23, 0x16, 0x22, 0x1C, 0x09, 0x2D, 0x15,
				0x20, 0x00, 0x27, 0x05, 0x04, 0x28, 0x08, 0x20,
				0x21, 0x27, 0x07, 0x29, 0x3C, 0x32, 0x36, 0x12,
				0x28, 0x2B, 0x0D, 0x08, 0x10, 0x3D, 0x24, 0x01,
				0x01, 0x31, 0x33, 0x2A, 0x2C, 0x0C, 0x1B, 0x14,
				0x0D, 0x07, 0x34, 0x06, 0x13, 0x02, 0x26, 0x0D,
				0x0D, 0x19, 0x10, 0x0A, 0x39, 0x03, 0x37, 0x17,
				0x09, 0x11, 0x1A, 0x1D, 0x38, 0x25, 0x18, 0x3A
			},
			{
				0x0D, 0x27, 0x18, 0x39, 0x3A, 0x25, 0x1C, 0x31,
				0x16, 0x13, 0x38, 0x34, 0x20, 0x23, 0x3C, 0x1A,
				0x09, 0x21, 0x06, 0x10, 0x1B, 0x29, 0x08, 0x22,
				0x2D, 0x24, 0x01, 0x2B, 0x32, 0x08, 0x0D, 0x03,
				0x04, 0x36, 0x26, 0x33, 0x11, 0x07, 0x10, 0x02,
				0x14, 0x28, 0x00, 0x09, 0x12, 0x0D, 0x28, 0x20,
				0x27, 0x1D, 0x2A, 0x17, 0x0C, 0x01, 0x15, 0x19,
				0x0D, 0x2C, 0x07, 0x37, 0x35, 0x05, 0x0A, 0x3D
			},
			{
				0x14, 0x25, 0x3A, 0x10, 0x1A, 0x20, 0x31, 0x09,
				0x01, 0x0D, 0x36, 0x08, 0x15, 0x10, 0x27, 0x3C,
				0x22, 0x1C, 0x05, 0x12, 0x19, 0x18, 0x17, 0x1B,
				0x00, 0x03, 0x0D, 0x02, 0x16, 0x06, 0x34, 0x35,
				0x23, 0x09, 0x01, 0x37, 0x1D, 0x27, 0x26, 0x20,
				0x29, 0x04, 0x21, 0x24, 0x11, 0x3D, 0x0D, 0x07,
				0x2C, 0x08, 0x39, 0x33, 0x07, 0x2A, 0x28, 0x2D,
				0x0A, 0x0D, 0x32, 0x38, 0x13, 0x2B, 0x28, 0x0C
			},
			{
				0x18, 0x03, 0x1C, 0x28, 0x0D, 0x35, 0x01, 0x17,
				0x10, 0x07, 0x2A, 0x01, 0x36, 0x37, 0x1A, 0x39,
				0x25, 0x08, 0x12, 0x34, 0x0D, 0x2D, 0x06, 0x26,
				0x27, 0x1B, 0x22, 0x19, 0x04, 0x0D, 0x3A, 0x21,
				0x05, 0x0A, 0x07, 0x02, 0x13, 0x14, 0x00, 0x15,
				0x0C, 0x10, 0x11, 0x09, 0x1D, 0x38, 0x3D, 0x24,
				0x33, 0x20, 0x08, 0x16, 0x28, 0x2B, 0x20, 0x3C,
				0x0D, 0x27, 0x23, 0x31, 0x29, 0x32, 0x2C, 0x09
			}
		};

		Ppu::Tiles::Tiles()
		: padding0(0), padding1(0) {}

		Ppu::Oam::Oam()
		: limit(buffer + STD_LINE_SPRITES*4), spriteLimit(true) {}

		Ppu::Output::Output(Video::Screen::Pixel* p)
		: pixels(p) {}

		Ppu::TileLut::TileLut()
		{
			for (uint i=0; i < 0x400; ++i)
			{
				block[i][0] = (i & 0xC0) ? (i >> 6 & 0xC) | (i >> 6 & 0x3) : 0;
				block[i][1] = (i & 0x30) ? (i >> 6 & 0xC) | (i >> 4 & 0x3) : 0;
				block[i][2] = (i & 0x0C) ? (i >> 6 & 0xC) | (i >> 2 & 0x3) : 0;
				block[i][3] = (i & 0x03) ? (i >> 6 & 0xC) | (i >> 0 & 0x3) : 0;
			}
		}

		Ppu::Ppu(Cpu& c)
		:
		cpu    (c),
		output (screen.pixels),
		model  (PPU_RP2C02),
		rgbMap (NULL),
		yuvMap (NULL)
		{
			cycles.one = PPU_RP2C02_CC;
			overclocked = false;
			PowerOff();
		}

		void Ppu::PowerOff()
		{
			Reset( true, false, false );
		}

		void Ppu::Reset(bool hard,bool acknowledged)
		{
			Reset( hard, acknowledged, true );
		}

		void Ppu::Reset(const bool hard,const bool acknowledged,const bool map)
		{
			if (map)
			{
				for (uint i=0x2000; i < 0x4000; i += 0x8)
				{
					cpu.Map( i+0 ).Set( this, i != 0x3000 ? &Ppu::Peek_2xxx : &Ppu::Peek_3000, &Ppu::Poke_2000 );
					cpu.Map( i+1 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2001 );
					cpu.Map( i+2 ).Set( this,               &Ppu::Peek_2002,                   &Ppu::Poke_2xxx );
					cpu.Map( i+3 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2003 );
					cpu.Map( i+4 ).Set( this,               &Ppu::Peek_2004,                   &Ppu::Poke_2004 );
					cpu.Map( i+5 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2005 );
					cpu.Map( i+6 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2006 );
					cpu.Map( i+7 ).Set( this,               &Ppu::Peek_2007,                   &Ppu::Poke_2007 );
				}

				if (model == PPU_RC2C05_01 || model == PPU_RC2C05_04)
				{
					for (uint i=0x2002; i < 0x4000; i += 0x8)
						cpu.Map( i ).Set( &Ppu::Peek_2002_RC2C05_01_04 );
				}
				else if (model == PPU_RC2C05_02)
				{
					for (uint i=0x2002; i < 0x4000; i += 0x8)
						cpu.Map( i ).Set( &Ppu::Peek_2002_RC2C05_02 );
				}
				else if (model == PPU_RC2C05_03)
				{
					for (uint i=0x2002; i < 0x4000; i += 0x8)
						cpu.Map( i ).Set( &Ppu::Peek_2002_RC2C05_03 );
				}
				else if (model == PPU_RC2C05_05)
				{
					for (uint i=0x2000; i < 0x4000; i += 0x8)
					{
						cpu.Map( i+0 ).Set( &Ppu::Poke_2001 );
						cpu.Map( i+1 ).Set( &Ppu::Poke_2000 );
					}
				}

				cpu.Map( 0x4014U ).Set( this, &Ppu::Peek_4014, &Ppu::Poke_4014 );
			}

			if (hard)
			{
				static const byte powerUpPalette[] =
				{
					0x3F,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,
					0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
					0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,
					0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08
				};

				std::memcpy( palette.ram, powerUpPalette, Palette::SIZE );
				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
				std::memset( nameTable.ram, NameTable::GARBAGE, NameTable::SIZE );

				io.latch = 0;
				io.buffer = Io::BUFFER_GARBAGE;

				regs.status = 0;
				regs.ctrl[0] = 0;
				regs.ctrl[1] = 0;
				regs.frame = 0;
				regs.oam = 0;

				scroll.latch = 0;
				scroll.xFine = 0;
				scroll.toggle = 0;
				scroll.address = 0;

				output.burstPhase = 0;

				cycles.reset = 0;
				cycles.hClock = HCLOCK_BOOT;
			}
			else if (acknowledged)
			{
				io.buffer = 0;

				regs.status = 0;
				regs.ctrl[0] = 0;
				regs.ctrl[1] = 0;

				scroll.latch = 0;
				scroll.xFine = 0;
				scroll.toggle = 0;

				cycles.reset = Cpu::CYCLE_MAX;
				cycles.hClock = HCLOCK_BOOT;

				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
			}
			else
			{
				cycles.hClock = HCLOCK_DUMMY;
				cycles.reset = 0;
			}

			if (chr.Source().Empty())
			{
				chr.Source().Set( Ram::RAM, true, false, NameTable::SIZE, nameTable.ram );
				chr.SwapBanks<SIZE_2K,0x0000>(0,0,0,0);
			}

			if (nmt.Source().Empty())
			{
				nmt.Source().Set( Ram::RAM, true, true, NameTable::SIZE, nameTable.ram );
				nmt.SwapBanks<SIZE_2K,0x0000>(0,0);
			}

			chr.ResetAccessor();
			nmt.ResetAccessors();

			cycles.vClock = 0;
			cycles.count = Cpu::CYCLE_MAX;

			scanline = SCANLINE_VBLANK;
			scanline_sleep = -1;
			ssleep = -1;

			io.address = 0;
			io.pattern = 0;
			io.line.Unset();

			tiles.pattern[0] = 0;
			tiles.pattern[1] = 0;
			tiles.attribute = 0;
			tiles.index = 8;
			tiles.mask = 0;

			oam.index = 0;
			oam.address = 0;
			oam.latch = 0;
			oam.spriteZeroInLine = false;
			oam.phase = &Ppu::EvaluateSpritesPhase0;
			oam.buffered = oam.buffer;
			oam.visible = oam.output;
			oam.mask = 0;

			output.target = NULL;

			hActiveHook.Unset();
			hBlankHook.Unset();

			UpdateStates();

			screen.Clear();
		}

		uint Ppu::SetAddressLineHook(const Core::Io::Line& line)
		{
			io.line = line;
			return io.address;
		}

		void Ppu::SetHActiveHook(const Hook& hook)
		{
			hActiveHook = hook;
		}

		void Ppu::SetHBlankHook(const Hook& hook)
		{
			hBlankHook = hook;
		}

		void Ppu::UpdateStates()
		{
			oam.height = (regs.ctrl[0] >> 2 & 8) + 8;

			tiles.show[0] = (regs.ctrl[1] & Regs::CTRL1_BG_ENABLED) ? 0xFF : 0x00;
			tiles.show[1] = (regs.ctrl[1] & Regs::CTRL1_BG_ENABLED_NO_CLIP) == Regs::CTRL1_BG_ENABLED_NO_CLIP ? 0xFF : 0x00;

			oam.show[0] = (regs.ctrl[1] & Regs::CTRL1_SP_ENABLED) ? 0xFF : 0x00;
			oam.show[1] = (regs.ctrl[1] & Regs::CTRL1_SP_ENABLED_NO_CLIP) == Regs::CTRL1_SP_ENABLED_NO_CLIP ? 0xFF : 0x00;

			UpdatePalette();
		}

		void Ppu::UpdatePalette()
		{
			for (uint i=0, c=Coloring(), e=Emphasis(); i < Palette::SIZE; ++i)
				output.palette[i] = ((rgbMap ? rgbMap[palette.ram[i] & uint(Palette::COLOR)] : palette.ram[i]) & c) | e;
		}

		void Ppu::SaveState(State::Saver& state,const dword baseChunk) const
		{
			state.Begin( baseChunk );

			{
				const byte data[11] =
				{
					regs.ctrl[0],
					regs.ctrl[1],
					regs.status,
					scroll.address & 0xFF,
					scroll.address >> 8,
					scroll.latch & 0xFF,
					scroll.latch >> 8,
					scroll.xFine | scroll.toggle << 3,
					regs.oam,
					io.buffer,
					io.latch
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			state.Begin( AsciiId<'P','A','L'>::V ).Compress( palette.ram   ).End();
			state.Begin( AsciiId<'O','A','M'>::V ).Compress( oam.ram       ).End();
			state.Begin( AsciiId<'N','M','T'>::V ).Compress( nameTable.ram ).End();

			if (model == PPU_RP2C02)
				state.Begin( AsciiId<'F','R','M'>::V ).Write8( (regs.frame & Regs::FRAME_ODD) == 0 ).End();

			if (cycles.hClock == HCLOCK_BOOT)
				state.Begin( AsciiId<'P','O','W'>::V ).Write8( 0x0 ).End();

			state.End();
		}

		void Ppu::LoadState(State::Loader& state)
		{
			cycles.hClock = HCLOCK_DUMMY;
			regs.frame = 0;
			output.burstPhase = 0;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<11> data( state );

						regs.ctrl[0]   = data[0];
						regs.ctrl[1]   = data[1];
						regs.status    = data[2] & Regs::STATUS_BITS;
						scroll.address = data[3] | (data[4] << 8 & 0x7F00);
						scroll.latch   = data[5] | (data[6] << 8 & 0x7F00);
						scroll.xFine   = data[7] & 0x7;
						scroll.toggle  = data[7] >> 3 & 0x1;
						regs.oam       = data[8];
						io.buffer      = data[9];
						io.latch       = data[10];

						break;
					}

					case AsciiId<'P','A','L'>::V:

						state.Uncompress( palette.ram );
						break;

					case AsciiId<'O','A','M'>::V:

						state.Uncompress( oam.ram );
						break;

					case AsciiId<'N','M','T'>::V:

						state.Uncompress( nameTable.ram );
						break;

					case AsciiId<'F','R','M'>::V:

						if (model == PPU_RP2C02)
							regs.frame = (state.Read8() & 0x1) ? 0 : Regs::FRAME_ODD;

						break;

					case AsciiId<'P','O','W'>::V:

						cycles.hClock = HCLOCK_BOOT;
						break;
				}

				state.End();
			}

			UpdateStates();
		}

		void Ppu::EnableCpuSynchronization()
		{
			cpu.AddHook( Hook(this,&Ppu::Hook_Sync) );
		}

		void Ppu::ChrMem::ResetAccessor()
		{
			accessor.Set( this, &ChrMem::Access_Pattern );
		}

		void Ppu::NmtMem::ResetAccessors()
		{
			accessors[0].Set( this, &NmtMem::Access_Name_2000 );
			accessors[1].Set( this, &NmtMem::Access_Name_2400 );
			accessors[2].Set( this, &NmtMem::Access_Name_2800 );
			accessors[3].Set( this, &NmtMem::Access_Name_2C00 );
		}

		void Ppu::SetModel(const PpuModel m,const bool yuvConversion)
		{
			if (model != m)
			{
				model = m;
				regs.frame = 0;
				output.burstPhase = 0;

				switch (model)
				{
					case PPU_RP2C07: cycles.one = PPU_RP2C07_CC; break;
					case PPU_DENDY:  cycles.one = PPU_DENDY_CC;  break;
					default:         cycles.one = PPU_RP2C02_CC; break;
				}
			}

			const byte* const map =
			(
				model == PPU_RP2C04_0001 ? yuvMaps[0] :
				model == PPU_RP2C04_0002 ? yuvMaps[1] :
				model == PPU_RP2C04_0003 ? yuvMaps[2] :
				model == PPU_RP2C04_0004 ? yuvMaps[3] :
                                           NULL
			);

			const byte* const tmp[2] =
			{
				yuvConversion ? NULL : map,
				yuvConversion ? map : NULL
			};

			if (yuvMap != tmp[0] || rgbMap != tmp[1])
			{
				yuvMap = tmp[0];
				rgbMap = tmp[1];

				UpdatePalette();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_FORCE_INLINE Cycle Ppu::GetCycles() const
		{
			return (cycles.vClock + cycles.hClock) * cycles.one;
		}

		NST_FORCE_INLINE Cycle Ppu::GetLocalCycles(Cycle clock) const
		{
			NST_COMPILE_ASSERT( PPU_DENDY_CC == PPU_RP2C02_CC || PPU_DENDY_CC == PPU_RP2C07_CC );
			return cycles.one == PPU_RP2C02_CC ? clock / PPU_RP2C02_CC : (clock+PPU_RP2C07_CC-1) / PPU_RP2C07_CC;
		}

		void Ppu::BeginFrame(bool frameLock)
		{
			NST_ASSERT
			(
				(scanline == SCANLINE_VBLANK) &&
				(cycles.hClock == HCLOCK_BOOT || cycles.hClock == HCLOCK_DUMMY) &&
				(cpu.GetModel() == CPU_RP2A07) == (model == PPU_RP2C07) &&
				(cpu.GetModel() == CPU_DENDY)  == (model == PPU_DENDY)
			);

			oam.limit = oam.buffer + ((oam.spriteLimit || frameLock) ? Oam::STD_LINE_SPRITES*4 : Oam::MAX_LINE_SPRITES*4);
			output.target = output.pixels;

			Cycle frame;

			scanline_sleep = -1;

			switch (model)
			{
				case PPU_RP2C02:

					regs.frame ^= Regs::FRAME_ODD;

				default:

					ssleep = PPU_RP2C02_VSLEEP - 2;

					if (cycles.hClock == HCLOCK_DUMMY)
					{
						cycles.vClock = PPU_RP2C02_HVINT / PPU_RP2C02_CC - HCLOCK_DUMMY;
						cycles.count = PPU_RP2C02_HVINT;
						frame = PPU_RP2C02_HVSYNC_0;
					}
					else
					{
						cycles.vClock = PPU_RP2C02_HVSYNCBOOT / PPU_RP2C02_CC - HCLOCK_BOOT;
						cycles.count = PPU_RP2C02_HVSYNCBOOT;
						frame = PPU_RP2C02_HVSYNCBOOT;
					}
					break;

				case PPU_RP2C07:

					ssleep = PPU_RP2C07_VSLEEP - 2;

					if (cycles.hClock == HCLOCK_DUMMY)
					{
						cycles.vClock = PPU_RP2C07_HVINT / PPU_RP2C07_CC - HCLOCK_DUMMY;
						cycles.count = PPU_RP2C07_HVINT;
						frame = PPU_RP2C07_HVSYNC;
					}
					else
					{
						cycles.vClock = PPU_RP2C07_HVSYNCBOOT / PPU_RP2C07_CC - HCLOCK_BOOT;
						cycles.count = PPU_RP2C07_HVSYNCBOOT;
						frame = PPU_RP2C07_HVSYNCBOOT;
					}
					break;

				case PPU_DENDY:

					ssleep = PPU_DENDY_VSLEEP - 2;

					if (cycles.hClock == HCLOCK_DUMMY)
					{
						cycles.vClock = PPU_DENDY_HVINT / PPU_DENDY_CC - HCLOCK_DUMMY;
						cycles.count = PPU_DENDY_HVINT;
						frame = PPU_DENDY_HVSYNC;
					}
					else
					{
						cycles.vClock = PPU_DENDY_HVSYNCBOOT / PPU_DENDY_CC - HCLOCK_BOOT;
						cycles.count = PPU_DENDY_HVSYNCBOOT;
						frame = PPU_DENDY_HVSYNCBOOT;
					}
					break;
			}

			if (overclocked)
			{
				Apu& audioSafeOverclock = cpu.GetApu();
				if (audioSafeOverclock.GetOverclockSafety())
				{
					switch (model)
					{
						case PPU_RP2C02:
						default:

						cpu.SetOverclocking(true,PPU_RP2C02_HSYNC * PPU_RP2C02_VACTIVE);
						break;

						case PPU_RP2C07:

						cpu.SetOverclocking(true,PPU_RP2C07_HSYNC * PPU_RP2C07_VACTIVE);
						break;

						case PPU_DENDY:

						cpu.SetOverclocking(true,PPU_DENDY_HSYNC * PPU_DENDY_VACTIVE);
						break;
					}
				}
				else
				{
					cpu.SetOverclocking(false,0);
				}

				audioSafeOverclock.SetOverclockSafety(true);//overclocking is only safe if direct pcm audio has not been written for one frame
			}

			cpu.SetFrameCycles( frame );
		}

		NES_HOOK(Ppu,Sync)
		{
			const Cycle elapsed = cpu.GetCycles();

			if (cycles.count < elapsed)
			{
				cycles.count = GetLocalCycles( elapsed ) - cycles.vClock;
				Run();
			}
		}

		void Ppu::EndFrame()
		{
			if (cycles.count != Cpu::CYCLE_MAX)
			{
				cycles.count = Cpu::CYCLE_MAX;
				Run();
			}
		}

		void Ppu::Update(Cycle dataSetup,const uint readAddress)
		{
			dataSetup += cpu.Update( readAddress );

			if (cycles.count < dataSetup)
			{
				cycles.count = GetLocalCycles( dataSetup ) - cycles.vClock;
				Run();
			}
		}

		void Ppu::SetMirroring(const byte (&banks)[4])
		{
			Update( cycles.one );

			NST_ASSERT( banks[0] < 4 && banks[1] < 4 && banks[2] < 4 && banks[3] < 4 );
			nmt.SwapBanks<SIZE_1K,0x0000>( banks[0], banks[1], banks[2], banks[3] );
		}

		void Ppu::SetMirroring(NmtMirroring mirroring)
		{
			Update( cycles.one );

			nmt.SwapBanks<SIZE_1K,0x0000>
			(
				uint(mirroring) >> 0 & 0x1U,
				uint(mirroring) >> 1 & 0x1U,
				uint(mirroring) >> 2 & 0x1U,
				uint(mirroring) >> 3 & 0x1U
			);
		}

		NES_ACCESSOR(Ppu::ChrMem,Pattern)
		{
			return Peek( address );
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2000)
		{
			return (*this)[0][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2400)
		{
			return (*this)[1][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2800)
		{
			return (*this)[2][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2C00)
		{
			return (*this)[3][address];
		}

		NST_FORCE_INLINE uint Ppu::Chr::FetchPattern(uint address) const
		{
			return accessor.Fetch( address & 0x1FFF );
		}

		NST_FORCE_INLINE uint Ppu::Nmt::FetchName(uint address) const
		{
			return accessors[address >> 10 & 0x3].Fetch( address & 0x3FF );
		}

		NST_FORCE_INLINE uint Ppu::Nmt::FetchAttribute(uint address) const
		{
			return accessors[address >> 10 & 0x3].Fetch( 0x3C0 | (address & 0x03F) );
		}

		NST_FORCE_INLINE void Ppu::UpdateAddressLine(uint address)
		{
			NST_ASSERT( address <= 0x3FFF );
			io.address = address;

			if (io.line)
				io.line.Toggle( io.address, GetCycles() );
		}

		NST_FORCE_INLINE void Ppu::UpdateVramAddress()
		{
			if ((scanline != SCANLINE_VBLANK ) && (regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED))
			{
				scroll.ClockX();
				scroll.ClockY();
			}
			else
			{
				scroll.address = (scroll.address + ((regs.ctrl[0] & Regs::CTRL0_INC32) ? 32 : 1)) & 0x7FFF;
			}
		}

		NST_FORCE_INLINE void Ppu::OpenName()
		{
			UpdateAddressLine( 0x2000 | (scroll.address & 0x0FFF) );
		}

		NST_FORCE_INLINE void Ppu::FetchName()
		{
			io.pattern = nmt.FetchName( io.address ) << 4 | scroll.address >> 12 | (regs.ctrl[0] << 8 & 0x1000);
		}

		NST_FORCE_INLINE void Ppu::OpenAttribute()
		{
			UpdateAddressLine( 0x23C0 | (scroll.address & 0x0C00) | (scroll.address >> 4 & 0x0038) | (scroll.address >> 2 & 0x0007) );
		}

		NST_FORCE_INLINE void Ppu::FetchAttribute()
		{
			tiles.attribute = nmt.FetchAttribute( io.address ) >> ((scroll.address & 0x2) | (scroll.address >> 4 & 0x4));
		}

		NST_FORCE_INLINE void Ppu::OpenPattern(uint address)
		{
			UpdateAddressLine( address );
		}

		NST_FORCE_INLINE uint Ppu::FetchSpPattern() const
		{
			return chr.FetchPattern( io.address );
		}

		NST_FORCE_INLINE void Ppu::FetchBgPattern0()
		{
			const uint pattern = chr.FetchPattern( io.address );

			tiles.pattern[1] = pattern >> 0 & 0x55;
			tiles.pattern[0] = pattern >> 1 & 0x55;
		}

		NST_FORCE_INLINE void Ppu::FetchBgPattern1()
		{
			const uint pattern = chr.FetchPattern( io.address );

			tiles.pattern[0] |= pattern << 0 & 0xAA;
			tiles.pattern[1] |= pattern << 1 & 0xAA;
		}

		uint Ppu::GetPixelCycles() const
		{
			return (scanline+1)-1U < 240 ? scanline * 256 + NST_MIN(cycles.hClock,255) : ~0U;
		}

		NST_FORCE_INLINE bool Ppu::IsDead() const
		{
			return scanline == SCANLINE_VBLANK || !(regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED);
		}

		NST_FORCE_INLINE uint Ppu::Coloring() const
		{
			return (regs.ctrl[1] & Regs::CTRL1_MONOCHROME) ? Palette::MONO : Palette::COLOR;
		}

		NST_FORCE_INLINE uint Ppu::Emphasis() const
		{
			return (regs.ctrl[1] & Regs::CTRL1_EMPHASIS) << 1;
		}

		NES_POKE_D(Ppu,2000)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				scroll.latch = (scroll.latch & 0x73FF) | (data & 0x03) << 10;
				oam.height = (data >> 2 & 8) + 8;

				io.latch = data;
				data = regs.ctrl[0] ;
				regs.ctrl[0] = io.latch;

				if ((regs.ctrl[0] & regs.status & Regs::CTRL0_NMI) > data)
				{
					const Cycle clock = cpu.GetCycles() + cycles.one;

					if (clock < GetHVIntClock())
						cpu.DoNMI( clock );
				}
			}
		}

		NES_POKE_D(Ppu,2001)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				if ((regs.ctrl[1] ^ data) & (Regs::CTRL1_BG_ENABLED_NO_CLIP|Regs::CTRL1_SP_ENABLED_NO_CLIP))
				{
					tiles.show[0] = (data & Regs::CTRL1_BG_ENABLED) ? 0xFF : 0x00;
					tiles.show[1] = (data & Regs::CTRL1_BG_ENABLED_NO_CLIP) == Regs::CTRL1_BG_ENABLED_NO_CLIP ? 0xFF : 0x00;

					oam.show[0] = (data & Regs::CTRL1_SP_ENABLED) ? 0xFF : 0x00;
					oam.show[1] = (data & Regs::CTRL1_SP_ENABLED_NO_CLIP) == Regs::CTRL1_SP_ENABLED_NO_CLIP ? 0xFF : 0x00;

					const uint pos = (cycles.hClock - 8) >= (256-16);

					tiles.mask = tiles.show[pos];
					oam.mask = oam.show[pos];

					if ((regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED) && !(data & Regs::CTRL1_BG_SP_ENABLED))
						UpdateAddressLine(scroll.address & 0x3fff);
				}

				io.latch = data;
				data = (regs.ctrl[1] ^ data) & (Regs::CTRL1_EMPHASIS|Regs::CTRL1_MONOCHROME);
				regs.ctrl[1] = io.latch;

				if (data)
				{
					const uint ce[] = { Coloring(), Emphasis() };

					const byte* const NST_RESTRICT map = rgbMap;

					if (!map)
					{
						for (uint i=0; i < Palette::SIZE; ++i)
							output.palette[i] = (palette.ram[i] & ce[0]) | ce[1];
					}
					else
					{
						for (uint i=0; i < Palette::SIZE; ++i)
							output.palette[i] = (map[palette.ram[i] & Palette::COLOR] & ce[0]) | ce[1];
					}
				}
			}
		}

		NES_PEEK_A(Ppu,2002)
		{
			Update( cycles.one, address );

			uint status = regs.status & 0xFF;

			regs.status &= (Regs::STATUS_VBLANK^0xFFU);
			scroll.toggle = 0;
			io.latch = (io.latch & Regs::STATUS_LATCH) | status;

			return io.latch;
		}

		NES_PEEK_A(Ppu,2002_RC2C05_01_04)
		{
			return (NES_DO_PEEK(2002,address) & 0xC0) | 0x1B;
		}

		NES_PEEK_A(Ppu,2002_RC2C05_02)
		{
			return (NES_DO_PEEK(2002,address) & 0xC0) | 0x3D;
		}

		NES_PEEK_A(Ppu,2002_RC2C05_03)
		{
			return (NES_DO_PEEK(2002,address) & 0xC0) | 0x1C;
		}

		NES_POKE_D(Ppu,2003)
		{
			Update( cycles.one );

			regs.oam = data;
			io.latch = data;
		}

		NES_POKE_D(Ppu,2004)
		{
			Update( cycles.one );

			NST_ASSERT( regs.oam < Oam::SIZE );
			NST_VERIFY( IsDead() );

			if (IsDead())
			{
				if ((regs.oam & 0x03) == 0x02)
					data &= 0xE3;
			}
			else
			{
				data = 0xFF;
			}

			byte* const NST_RESTRICT value = oam.ram + regs.oam;
			regs.oam = (regs.oam + 1) & 0xFF;
			io.latch = data;
			*value = data;
		}

		NES_PEEK(Ppu,2004)
		{
			NST_ASSERT( regs.oam <= 0xFF );

			if (!(regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED) || cpu.GetCycles() - (cpu.GetFrameCycles() - (341 * 241) * cycles.one) >= (341 * 240) * cycles.one)
			{
				io.latch = oam.ram[regs.oam];
			}
			else
			{
				Update( cycles.one );

				io.latch = oam.latch;
			}

			return io.latch;
		}

		NES_POKE_D(Ppu,2005)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				io.latch = data;

				if (scroll.toggle ^= 1)
				{
					scroll.latch = (scroll.latch & 0x7FE0) | (data >> 3);
					scroll.xFine = data & 0x7;
				}
				else
				{
					scroll.latch = (scroll.latch & 0x0C1F) | ((data << 2 | data << 12) & 0x73E0);
				}
			}
		}

		NES_POKE_D(Ppu,2006)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				io.latch = data;

				if (scroll.toggle ^= 1)
				{
					scroll.latch = (scroll.latch & 0x00FF) | (data & 0x3F) << 8;
				}
				else
				{
					scroll.latch = (scroll.latch & 0x7F00) | data;
					scroll.address = scroll.latch;
					if (!(regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED) ||
					    (scanline == SCANLINE_VBLANK)) {
						UpdateAddressLine(scroll.address & 0x3fff);
					}
				}
			}
		}

		NES_POKE_D(Ppu,2007)
		{
			Update( cycles.one * 4 );

			uint address = scroll.address;

			UpdateVramAddress();
			if (!(regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED) ||
			    (scanline == SCANLINE_VBLANK)) {
				UpdateAddressLine(scroll.address & 0x3fff);
			}
			else {
				return;
			}

			io.latch = data;

			if ((address & 0x3F00) == 0x3F00)
			{
				address &= 0x1F;

				const uint final = ((!rgbMap ? data : rgbMap[data & Palette::COLOR]) & Coloring()) | Emphasis();

				palette.ram[address] = data;
				output.palette[address] = final;

				if (!(address & 0x3))
				{
					palette.ram[address ^ 0x10] = data;
					output.palette[address ^ 0x10] = final;
				}
				
				output.bgColor = palette.ram[0] & uint(Palette::COLOR);
			}
			else
			{
				address &= 0x3FFF;

				if (address >= 0x2000)
					nmt.Poke( address & 0xFFF, data );
				else
					chr.Poke( address, data );
			}
		}

		NES_PEEK_A(Ppu,2007)
		{
			Update( cycles.one, address );

			address = scroll.address & 0x3FFF;
			UpdateVramAddress();
			if (!(regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED) ||
			    (scanline == SCANLINE_VBLANK)) {
				UpdateAddressLine(scroll.address & 0x3fff);
			}

			io.latch = (address & 0x3F00) != 0x3F00 ? io.buffer : palette.ram[address & 0x1F] & Coloring();
			io.buffer = (address >= 0x2000 ? nmt.FetchName( address ) : chr.FetchPattern( address ));

			return io.latch;
		}

		NES_POKE_D(Ppu,2xxx)
		{
			io.latch = data;
		}

		NES_PEEK(Ppu,2xxx)
		{
			return io.latch;
		}

		NES_PEEK(Ppu,3000)
		{
			Update( cycles.one );

			return io.latch;
		}

		NES_POKE_D(Ppu,4014)
		{
			if (cpu.IsOddCycle())
				cpu.StealCycles( cpu.GetClock() );

			Update( cycles.one );
			cpu.StealCycles( cpu.GetClock() );

			NST_ASSERT( regs.oam < 0x100 );

			data <<= 8;

			if ((regs.oam == 0x00 && data < 0x2000) && (!(regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED) || cpu.GetCycles() <= GetHVIntClock() - cpu.GetClock() * 512))
			{
				cpu.StealCycles( cpu.GetClock() * 512 );

				const byte* const NST_RESTRICT cpuRam = cpu.GetRam() + (data & (Cpu::RAM_SIZE-1));
				byte* const NST_RESTRICT oamRam = oam.ram;

				for (uint i=0x00; i < 0x100; i += 0x4)
				{
					oamRam[i+0x0] = cpuRam[i+0x0];
					oamRam[i+0x1] = cpuRam[i+0x1];
					oamRam[i+0x2] = cpuRam[i+0x2] & 0xE3U;
					oamRam[i+0x3] = cpuRam[i+0x3];
				}

				io.latch = oamRam[0xFF];
			}
			else do
			{
				io.latch = cpu.Peek( data++ );
				cpu.StealCycles( cpu.GetClock() );

				Update( cycles.one );
				cpu.StealCycles( cpu.GetClock() );

				NST_VERIFY( IsDead() );

				if (IsDead())
				{
					if ((regs.oam & 0x03) == 0x02)
						io.latch &= 0xE3;
				}
				else
				{
					io.latch = 0xFF;
				}

				byte* const NST_RESTRICT out = oam.ram + regs.oam;
				regs.oam = (regs.oam + 1) & 0xFF;
				*out = io.latch;
			}
			while (data & 0xFF);
		}

		NES_PEEK(Ppu,4014)
		{
			return 0x40;
		}

		NST_FORCE_INLINE void Ppu::Scroll::ClockX()
		{
			if ((address & X_TILE) != X_TILE)
				address++;
			else
				address ^= (X_TILE|NAME_LOW);
		}

		NST_SINGLE_CALL void Ppu::Scroll::ResetX()
		{
			address = (address & ((X_TILE|NAME_LOW) ^ 0x7FFFU)) | (latch & (X_TILE|NAME_LOW));
		}

		NST_SINGLE_CALL void Ppu::Scroll::ClockY()
		{
			if ((address & Y_FINE) != (7U << 12))
			{
				address += (1U << 12);
			}
			else switch (address & Y_TILE)
			{
				default:         address = (address & (Y_FINE ^ 0x7FFFU)) + (1U << 5); break;
				case (29U << 5): address ^= NAME_HIGH;
				case (31U << 5): address &= (Y_FINE|Y_TILE) ^ 0x7FFFU; break;
			}
		}

		NST_SINGLE_CALL void Ppu::PreLoadTiles()
		{
			const byte* const NST_RESTRICT src[] =
			{
				tileLut.block[tiles.pattern[0] | (tiles.attribute & 0x3U) << 8],
				tileLut.block[tiles.pattern[1] | (tiles.attribute & 0x3U) << 8]
			};

			NST_ASSERT( tiles.index == 8 );

			byte* const NST_RESTRICT dst = tiles.pixels;

			dst[0] = src[0][0];
			dst[1] = src[1][0];
			dst[2] = src[0][1];
			dst[3] = src[1][1];
			dst[4] = src[0][2];
			dst[5] = src[1][2];
			dst[6] = src[0][3];
			dst[7] = src[1][3];
		}

		NST_SINGLE_CALL void Ppu::LoadTiles()
		{
			const byte* const NST_RESTRICT src[] =
			{
				tileLut.block[tiles.pattern[0] | (tiles.attribute & 0x3U) << 8],
				tileLut.block[tiles.pattern[1] | (tiles.attribute & 0x3U) << 8]
			};

			NST_ASSERT( tiles.index == 0 || tiles.index == 8 );

			byte* const NST_RESTRICT dst = tiles.pixels + tiles.index;
			tiles.index ^= 8U;

			dst[0] = src[0][0];
			dst[1] = src[1][0];
			dst[2] = src[0][1];
			dst[3] = src[1][1];
			dst[4] = src[0][2];
			dst[5] = src[1][2];
			dst[6] = src[0][3];
			dst[7] = src[1][3];
		}

		NST_FORCE_INLINE void Ppu::EvaluateSpritesEven()
		{
			if (cycles.hClock >= 64)
				oam.latch = oam.ram[oam.address];
		}

		NST_FORCE_INLINE void Ppu::EvaluateSpritesOdd()
		{
			(*this.*oam.phase)();
		}

		void Ppu::EvaluateSpritesPhase0()
		{
		}

		void Ppu::EvaluateSpritesPhase1()
		{
			oam.index++;

			if (scanline - oam.latch >= oam.height)
			{
				if (oam.index != 64)
				{
					oam.address = (oam.index != 2 ? oam.address + 4 : 8);
				}
				else
				{
					oam.address = 0;
					oam.phase = &Ppu::EvaluateSpritesPhase9;
				}
			}
			else
			{
				oam.address++;
				oam.phase = &Ppu::EvaluateSpritesPhase2;
				oam.buffered[0] = oam.latch;
			}
		}

		void Ppu::EvaluateSpritesPhase2()
		{
			oam.address++;
			oam.phase = &Ppu::EvaluateSpritesPhase3;
			oam.buffered[1] = oam.latch;
		}

		void Ppu::EvaluateSpritesPhase3()
		{
			oam.address++;
			oam.phase = &Ppu::EvaluateSpritesPhase4;
			oam.buffered[2] = oam.latch;
		}

		void Ppu::EvaluateSpritesPhase4()
		{
			oam.buffered[3] = oam.latch;
			oam.buffered += 4;

			if (oam.index != 64)
			{
				oam.phase = (oam.buffered != oam.limit ? &Ppu::EvaluateSpritesPhase1 : &Ppu::EvaluateSpritesPhase5);

				if (oam.index != 2)
				{
					oam.address++;

					if (oam.index == 1)
						oam.spriteZeroInLine = true;
				}
				else
				{
					oam.address = 8;
				}
			}
			else
			{
				oam.address = 0;
				oam.phase = &Ppu::EvaluateSpritesPhase9;
			}
		}

		void Ppu::EvaluateSpritesPhase5()
		{
			if (scanline - oam.latch >= oam.height)
			{
				oam.address = ((oam.address + 4) & 0xFC) + ((oam.address + 1) & 0x03);

				if (oam.address <= 5)
				{
					oam.phase = &Ppu::EvaluateSpritesPhase9;
					oam.address &= 0xFC;
				}
			}
			else
			{
				oam.phase = &Ppu::EvaluateSpritesPhase6;
				oam.address = (oam.address + 1) & 0xFF;
				regs.status |= Regs::STATUS_SP_OVERFLOW;
			}
		}

		void Ppu::EvaluateSpritesPhase6()
		{
			oam.phase = &Ppu::EvaluateSpritesPhase7;
			oam.address = (oam.address + 1) & 0xFF;
		}

		void Ppu::EvaluateSpritesPhase7()
		{
			oam.phase = &Ppu::EvaluateSpritesPhase8;
			oam.address = (oam.address + 1) & 0xFF;
		}

		void Ppu::EvaluateSpritesPhase8()
		{
			oam.phase = &Ppu::EvaluateSpritesPhase9;
			oam.address = (oam.address + 1) & 0xFF;

			if ((oam.address & 0x3) == 0x3)
				oam.address++;

			oam.address &= 0xFC;
		}

		void Ppu::EvaluateSpritesPhase9()
		{
			oam.address = (oam.address + 4) & 0xFF;
		}

		NST_FORCE_INLINE uint Ppu::OpenSprite() const
		{
			return (regs.ctrl[0] & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16)) ? 0x1FF0 : 0x0FF0;
		}

		NST_FORCE_INLINE uint Ppu::OpenSprite(const byte* const NST_RESTRICT buffer) const
		{
			uint address;
			const uint comparitor = (uint(scanline) - buffer[0]) ^ ((buffer[2] & uint(Oam::Y_FLIP)) ? 0xF : 0x0);

			if (regs.ctrl[0] & Regs::CTRL0_SP8X16)
			{
				address =
				(
					((buffer[1] & uint(Oam::TILE_LSB)) << 12) |
					((buffer[1] & (Oam::TILE_LSB ^ 0xFFU)) << 4) |
					((comparitor & Oam::RANGE_MSB) << 1)
				);
			}
			else
			{
				address = (regs.ctrl[0] & Regs::CTRL0_SP_OFFSET) << 9 | buffer[1] << 4;
			}

			return address | (comparitor & Oam::XFINE);
		}

		NST_FORCE_INLINE void Ppu::LoadSprite(const uint pattern0,const uint pattern1,const byte* const NST_RESTRICT buffer)
		{
			if (pattern0 | pattern1)
			{
				uint a = (buffer[2] & uint(Oam::X_FLIP)) ? 7 : 0;

				uint p =
				(
					(pattern0 >> 1 & 0x0055) | (pattern1 << 0 & 0x00AA) |
					(pattern0 << 8 & 0x5500) | (pattern1 << 9 & 0xAA00)
				);

				Oam::Output* const NST_RESTRICT entry = oam.visible++;

				entry->pixels[( a^=6 )] = ( p       ) & 0x3;
				entry->pixels[( a^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( a^=6 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( a^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( a^=7 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( a^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( a^=6 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( a^=2 )] = ( p >>= 2 );

				const uint attribute = buffer[2];

				entry->x       = buffer[3];
				entry->palette = Palette::SPRITE_OFFSET + ((attribute & Oam::COLOR) << 2);
				entry->behind  = (attribute & Oam::BEHIND) ? 0x3 : 0x0;
				entry->zero    = (buffer == oam.buffer && oam.spriteZeroInLine) ? 0x3 : 0x0;
			}
		}

		void Ppu::LoadExtendedSprites()
		{
			const byte* NST_RESTRICT buffer = oam.buffer + (8*4);
			NST_ASSERT( buffer < oam.buffered );

			do
			{
				const uint address = OpenSprite( buffer );

				const uint patterns[2] =
				{
					chr.FetchPattern( address | 0x0 ),
					chr.FetchPattern( address | 0x8 )
				};

				LoadSprite( patterns[0], patterns[1], buffer );
				buffer += 4;
			}
			while (buffer != oam.buffered);
		}

		NST_FORCE_INLINE void Ppu::RenderPixel()
		{
			uint clock;
			uint pixel = tiles.pixels[((clock=cycles.hClock++) + scroll.xFine) & 15] & tiles.mask;

			for (const Oam::Output* NST_RESTRICT sprite=oam.output, *const end=oam.visible; sprite != end; ++sprite)
			{
				uint x = clock - sprite->x;

				if (x > 7)
					continue;

				x = sprite->pixels[x] & oam.mask;

				if (x)
				{
					if (pixel & sprite->zero)
						regs.status |= Regs::STATUS_SP_ZERO_HIT;

					if (!(pixel & sprite->behind))
						pixel = sprite->palette + x;

					break;
				}
			}

			Video::Screen::Pixel* const NST_RESTRICT target = output.target++;
			*target = output.palette[pixel];
		}

		NST_SINGLE_CALL void Ppu::RenderPixel255()
		{
			cycles.hClock = 256;
			uint pixel = tiles.pixels[(255 + scroll.xFine) & 15] & tiles.mask;

			for (const Oam::Output* NST_RESTRICT sprite=oam.output, *const end=oam.visible; sprite != end; ++sprite)
			{
				uint x = 255U - sprite->x;

				if (x > 7)
					continue;

				x = sprite->pixels[x] & oam.mask;

				if (x)
				{
					if (!(pixel & sprite->behind))
						pixel = sprite->palette + x;

					break;
				}
			}

			Video::Screen::Pixel* const NST_RESTRICT target = output.target++;
			*target = output.palette[pixel];
		}

		NST_NO_INLINE void Ppu::Run()
		{
			NST_VERIFY( cycles.count != cycles.hClock );

			if (scanline_sleep >= 0)
			{
				switch (cycles.hClock)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					case 25:
					case 26:
					case 27:
					case 28:
					case 29:
					case 30:
					case 31:
					case 32:
					case 33:
					case 34:
					case 35:
					case 36:
					case 37:
					case 38:
					case 39:
					case 40:
					case 41:
					case 42:
					case 43:
					case 44:
					case 45:
					case 46:
					case 47:
					case 48:
					case 49:
					case 50:
					case 51:
					case 52:
					case 53:
					case 54:
					case 55:
					case 56:
					case 57:
					case 58:
					case 59:
					case 60:
					case 61:
					case 62:
					case 63:
					case 64:
					case 65:
					case 66:
					case 67:
					case 68:
					case 69:
					case 70:
					case 71:
					case 72:
					case 73:
					case 74:
					case 75:
					case 76:
					case 77:
					case 78:
					case 79:
					case 80:
					case 81:
					case 82:
					case 83:
					case 84:
					case 85:
					case 86:
					case 87:
					case 88:
					case 89:
					case 90:
					case 91:
					case 92:
					case 93:
					case 94:
					case 95:
					case 96:
					case 97:
					case 98:
					case 99:
					case 100:
					case 101:
					case 102:
					case 103:
					case 104:
					case 105:
					case 106:
					case 107:
					case 108:
					case 109:
					case 110:
					case 111:
					case 112:
					case 113:
					case 114:
					case 115:
					case 116:
					case 117:
					case 118:
					case 119:
					case 120:
					case 121:
					case 122:
					case 123:
					case 124:
					case 125:
					case 126:
					case 127:
					case 128:
					case 129:
					case 130:
					case 131:
					case 132:
					case 133:
					case 134:
					case 135:
					case 136:
					case 137:
					case 138:
					case 139:
					case 140:
					case 141:
					case 142:
					case 143:
					case 144:
					case 145:
					case 146:
					case 147:
					case 148:
					case 149:
					case 150:
					case 151:
					case 152:
					case 153:
					case 154:
					case 155:
					case 156:
					case 157:
					case 158:
					case 159:
					case 160:
					case 161:
					case 162:
					case 163:
					case 164:
					case 165:
					case 166:
					case 167:
					case 168:
					case 169:
					case 170:
					case 171:
					case 172:
					case 173:
					case 174:
					case 175:
					case 176:
					case 177:
					case 178:
					case 179:
					case 180:
					case 181:
					case 182:
					case 183:
					case 184:
					case 185:
					case 186:
					case 187:
					case 188:
					case 189:
					case 190:
					case 191:
					case 192:
					case 193:
					case 194:
					case 195:
					case 196:
					case 197:
					case 198:
					case 199:
					case 200:
					case 201:
					case 202:
					case 203:
					case 204:
					case 205:
					case 206:
					case 207:
					case 208:
					case 209:
					case 210:
					case 211:
					case 212:
					case 213:
					case 214:
					case 215:
					case 216:
					case 217:
					case 218:
					case 219:
					case 220:
					case 221:
					case 222:
					case 223:
					case 224:
					case 225:
					case 226:
					case 227:
					case 228:
					case 229:
					case 230:
					case 231:
					case 232:
					case 233:
					case 234:
					case 235:
					case 236:
					case 237:
					case 238:
					case 239:
					case 240:
					case 241:
					case 242:
					case 243:
					case 244:
					case 245:
					case 246:
					case 247:
					case 248:
					case 249:
					case 250:
					case 251:
					case 252:
					case 253:
					case 254:
					case 255:
					case 256:
					case 257:
					case 258:
					case 260:
					case 261:
					case 262:
					case 263:
					case 264:
					case 266:
					case 267:
					case 268:
					case 269:
					case 270:
					case 271:
					case 272:
					case 273:
					case 274:
					case 275:
					case 276:
					case 277:
					case 278:
					case 279:
					case 280:
					case 281:
					case 282:
					case 283:
					case 284:
					case 285:
					case 286:
					case 287:
					case 288:
					case 289:
					case 290:
					case 291:
					case 292:
					case 293:
					case 294:
					case 295:
					case 296:
					case 297:
					case 298:
					case 299:
					case 300:
					case 301:
					case 302:
					case 303:
					case 304:
					case 305:
					case 306:
					case 307:
					case 308:
					case 309:
					case 310:
					case 311:
					case 312:
					case 313:
					case 314:
					case 315:
					case 316:
					case 317:
					case 318:
					case 319:
					case 320:
					case 321:
					case 322:
					case 323:
					case 324:
					case 325:
					case 326:
					case 327:
					case 328:
					case 329:
					case 330:
					case 331:
					case 332:
					case 333:
					case 334:
					case 335:
					case 336:
					case 337:
					HActiveSleep:

						cycles.hClock = 338;

						if (cycles.count <= 338)
							break;

					case 338:

						if (scanline_sleep++ != ssleep)
						{
							cycles.hClock = 0;
							cycles.vClock += 341;

							if (cycles.count <= 341)
								break;

							cycles.count -= 341;

							goto HActiveSleep;
						}
						else
						{
							cycles.hClock = HCLOCK_VBLANK_0;

							if (cycles.count <= HCLOCK_VBLANK_0)
								break;
						}

					case HCLOCK_VBLANK_0:
					VBlank0:

						regs.status |= Regs::STATUS_VBLANKING;
						cycles.hClock = HCLOCK_VBLANK_1;

						if (cycles.count <= HCLOCK_VBLANK_1)
							break;

					case HCLOCK_VBLANK_1:
					VBlank1:

						regs.status = (regs.status & 0xFF) | (regs.status >> 1 & Regs::STATUS_VBLANK);
						oam.visible = oam.output;
						cycles.hClock = HCLOCK_VBLANK_2;

						if (cycles.count <= HCLOCK_VBLANK_2)
							break;

					case HCLOCK_VBLANK_2:
					VBlank2:

						scanline_sleep = -1;

						cycles.hClock = HCLOCK_DUMMY;
						cycles.count = Cpu::CYCLE_MAX;
						cycles.reset = 0;

						if (regs.ctrl[0] & regs.status & Regs::CTRL0_NMI)
							cpu.DoNMI( cpu.GetFrameCycles() );

						return;
				}
			}
			else if (regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED)
			{
				switch (cycles.hClock)
				{
					case 0:
					case 8:
					case 16:
					case 24:
					case 32:
					case 40:
					case 48:
					case 56:
					case 72:
					case 80:
					case 88:
					case 96:
					case 104:
					case 112:
					case 120:
					case 128:
					case 136:
					case 144:
					case 152:
					case 160:
					case 168:
					case 176:
					case 184:
					case 192:
					case 200:
					case 208:
					case 216:
					case 224:
					case 232:
					case 240:
					case 248:
					HActive:

						LoadTiles();
						EvaluateSpritesEven();
						OpenName();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 1:
					case 9:
					case 17:
					case 25:
					case 33:
					case 41:
					case 49:
					case 57:
					case 65:
					case 73:
					case 81:
					case 89:
					case 97:
					case 105:
					case 113:
					case 121:
					case 129:
					case 137:
					case 145:
					case 153:
					case 161:
					case 169:
					case 177:
					case 185:
					case 193:
					case 201:
					case 209:
					case 217:
					case 225:
					case 233:
					case 241:
					case 249:

						FetchName();
						EvaluateSpritesOdd();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 2:
					case 10:
					case 18:
					case 26:
					case 34:
					case 42:
					case 50:
					case 58:
					case 66:
					case 74:
					case 82:
					case 90:
					case 98:
					case 106:
					case 114:
					case 122:
					case 130:
					case 138:
					case 146:
					case 154:
					case 162:
					case 170:
					case 178:
					case 186:
					case 194:
					case 202:
					case 210:
					case 218:
					case 226:
					case 234:
					case 242:
					case 250:

						EvaluateSpritesEven();
						OpenAttribute();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 3:
					case 11:
					case 19:
					case 27:
					case 35:
					case 43:
					case 51:
					case 59:
					case 67:
					case 75:
					case 83:
					case 91:
					case 99:
					case 107:
					case 115:
					case 123:
					case 131:
					case 139:
					case 147:
					case 155:
					case 163:
					case 171:
					case 179:
					case 187:
					case 195:
					case 203:
					case 211:
					case 219:
					case 227:
					case 235:
					case 243:
					case 251:

						FetchAttribute();
						EvaluateSpritesOdd();

						if (cycles.hClock == 251)
							scroll.ClockY();

						scroll.ClockX();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 4:
					case 12:
					case 20:
					case 28:
					case 36:
					case 44:
					case 52:
					case 60:
					case 68:
					case 76:
					case 84:
					case 92:
					case 100:
					case 108:
					case 116:
					case 124:
					case 132:
					case 140:
					case 148:
					case 156:
					case 164:
					case 172:
					case 180:
					case 188:
					case 196:
					case 204:
					case 212:
					case 220:
					case 228:
					case 236:
					case 244:
					case 252:

						EvaluateSpritesEven();
						OpenPattern( io.pattern | 0x0 );
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 5:
					case 13:
					case 21:
					case 29:
					case 37:
					case 45:
					case 53:
					case 61:
					case 69:
					case 77:
					case 85:
					case 93:
					case 101:
					case 109:
					case 117:
					case 125:
					case 133:
					case 141:
					case 149:
					case 157:
					case 165:
					case 173:
					case 181:
					case 189:
					case 197:
					case 205:
					case 213:
					case 221:
					case 229:
					case 237:
					case 245:
					case 253:

						FetchBgPattern0();
						EvaluateSpritesOdd();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 6:
					case 14:
					case 22:
					case 30:
					case 38:
					case 46:
					case 54:
					case 62:
					case 70:
					case 78:
					case 86:
					case 94:
					case 102:
					case 110:
					case 118:
					case 126:
					case 134:
					case 142:
					case 150:
					case 158:
					case 166:
					case 174:
					case 182:
					case 190:
					case 198:
					case 206:
					case 214:
					case 222:
					case 230:
					case 238:
					case 246:
					case 254:

						EvaluateSpritesEven();
						OpenPattern( io.pattern | 0x8 );
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

						if (cycles.hClock == 255)
							goto HActive255;

					case 7:
					case 15:
					case 23:
					case 31:
					case 39:
					case 47:
					case 55:
					case 63:
					case 71:
					case 79:
					case 87:
					case 95:
					case 103:
					case 111:
					case 119:
					case 127:
					case 135:
					case 143:
					case 151:
					case 159:
					case 167:
					case 175:
					case 183:
					case 191:
					case 199:
					case 207:
					case 215:
					case 223:
					case 231:
					case 239:
					case 247:

						FetchBgPattern1();
						EvaluateSpritesOdd();
						RenderPixel();
						tiles.mask = tiles.show[0];
						oam.mask = oam.show[0];

						if (cycles.count <= cycles.hClock)
							break;

						if (cycles.hClock != 64)
							goto HActive;

					case 64:

						NST_VERIFY( regs.oam == 0 );
						oam.address = regs.oam & Oam::OFFSET_TO_0_1;
						oam.phase = &Ppu::EvaluateSpritesPhase1;
						oam.latch = 0xFF;
						goto HActive;

					case 255:
					HActive255:

						FetchBgPattern1();
						EvaluateSpritesOdd();
						RenderPixel255();

						if (cycles.count <= 256)
							break;

					case 256:

						OpenName();
						oam.latch = 0xFF;
						cycles.hClock = 257;

						if (cycles.count <= 257)
							break;

					case 257:

						if (hBlankHook)
							hBlankHook.Execute();

						scroll.ResetX();
						oam.visible = oam.output;
						cycles.hClock = 258;

						if (cycles.count <= 258)
							break;

					case 258:
					case 266:
					case 274:
					case 282:
					case 290:
					case 298:
					case 306:
					case 314:
					HBlankSp:

						OpenName();
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case 260:
					case 268:
					case 276:
					case 284:
					case 292:
					case 300:
					case 308:
					case 316:
					{
						const byte* const buffer = oam.buffer + ((cycles.hClock - 260) / 2);
						OpenPattern( buffer >= oam.buffered ? OpenSprite() : OpenSprite(buffer) );

						if (scanline == 238 && cycles.hClock == 316)
							regs.oam = 0;

						if (cycles.count <= ++cycles.hClock)
							break;
					}

					case 261:
					case 269:
					case 277:
					case 285:
					case 293:
					case 301:
					case 309:
					case 317:

						if (oam.buffer + ((cycles.hClock - 261) / 2) < oam.buffered)
							io.pattern = FetchSpPattern();

						if (cycles.count <= ++cycles.hClock)
							break;

					case 262:
					case 270:
					case 278:
					case 286:
					case 294:
					case 302:
					case 310:
					case 318:

						OpenPattern( io.address | 0x8 );

						if (cycles.count <= ++cycles.hClock)
							break;

					case 263:
					case 271:
					case 279:
					case 287:
					case 295:
					case 303:
					case 311:
					case 319:
					{
						const byte* const buffer = oam.buffer + ((cycles.hClock - 263) / 2);

						if (buffer < oam.buffered)
							LoadSprite( io.pattern, FetchSpPattern(), buffer );

						if (cycles.count <= ++cycles.hClock)
							break;

						if (cycles.hClock == 320)
							goto HBlankBg;
					}

					case 264:
					case 272:
					case 280:
					case 288:
					case 296:
					case 304:
					case 312:

						OpenName();
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

						goto HBlankSp;

					case 320:
					HBlankBg:

						if (oam.buffer + (8*4) < oam.buffered)
							LoadExtendedSprites();

						OpenName();

						if (hActiveHook)
							hActiveHook.Execute();

						oam.latch = oam.ram[0];
						oam.buffered = oam.buffer;
						oam.spriteZeroInLine = false;
						oam.index = 0;
						oam.phase = &Ppu::EvaluateSpritesPhase0;
						cycles.hClock = 321;

						if (cycles.count <= 321)
							break;

					case 321:

						FetchName();
						cycles.hClock = 322;

						if (cycles.count <= 322)
							break;

					case 322:

						OpenAttribute();
						cycles.hClock = 323;

						if (cycles.count <= 323)
							break;

					case 323:

						FetchAttribute();
						scroll.ClockX();
						cycles.hClock = 324;

						if (cycles.count <= 324)
							break;

					case 324:

						OpenPattern( io.pattern | 0x0 );
						cycles.hClock = 325;

						if (cycles.count <= 325)
							break;

					case 325:

						FetchBgPattern0();
						cycles.hClock = 326;

						if (cycles.count <= 326)
							break;

					case 326:

						OpenPattern( io.pattern | 0x8 );
						cycles.hClock = 327;

						if (cycles.count <= 327)
							break;

					case 327:

						FetchBgPattern1();
						cycles.hClock = 328;

						if (cycles.count <= 328)
							break;

					case 328:

						PreLoadTiles();
						OpenName();
						cycles.hClock = 329;

						if (cycles.count <= 329)
							break;

					case 329:

						FetchName();
						cycles.hClock = 330;

						if (cycles.count <= 330)
							break;

					case 330:

						OpenAttribute();
						cycles.hClock = 331;

						if (cycles.count <= 331)
							break;

					case 331:

						FetchAttribute();
						scroll.ClockX();
						cycles.hClock = 332;

						if (cycles.count <= 332)
							break;

					case 332:

						OpenPattern( io.pattern | 0x0 );
						cycles.hClock = 333;

						if (cycles.count <= 333)
							break;

					case 333:

						FetchBgPattern0();
						cycles.hClock = 334;

						if (cycles.count <= 334)
							break;

					case 334:

						OpenPattern( io.pattern | 0x8 );
						cycles.hClock = 335;

						if (cycles.count <= 335)
							break;

					case 335:

						FetchBgPattern1();
						cycles.hClock = 336;

						if (cycles.count <= 336)
							break;

					case 336:

						OpenName();
						cycles.hClock = 337;

						if (cycles.count <= 337)
							break;

					case 337:

						tiles.mask = tiles.show[1];
						oam.mask = oam.show[1];

						if (scanline == SCANLINE_HDUMMY && model == PPU_RP2C02)
						{
							if (regs.frame)
							{
								output.burstPhase = (output.burstPhase + 2) % 3;
								cpu.SetFrameCycles( PPU_RP2C02_HVSYNC_1 );
							}
							else
							{
								output.burstPhase = (output.burstPhase + 1) % 3;
							}
						}

						cycles.hClock = 338;

						if (cycles.count <= 338)
							break;

					case 338:

						OpenName();

						if (scanline++ != 239)
						{
							const uint line = (scanline != 0 || model != PPU_RP2C02 || !regs.frame ? 341 : 340);

							cycles.hClock = 0;
							cycles.vClock += line;

							if (cycles.count <= line)
								break;

							cycles.count -= line;

							goto HActive;
						}
						else
						{
							if (ssleep >= 0)
							{
								scanline_sleep = 0;
								cycles.hClock = 0;
								cycles.vClock += 341;

								if (cycles.count <= 341)
									break;

								cycles.count -= 341;

								goto HActiveSleep;
							}
							else
							{
								cycles.hClock = HCLOCK_VBLANK_0;

								if (cycles.count <= HCLOCK_VBLANK_0)
									break;
							}
						}

					case HCLOCK_VBLANK_0:
						goto VBlank0;

					case HCLOCK_VBLANK_1:
						goto VBlank1;

					case HCLOCK_VBLANK_2:
						goto VBlank2;

					case HCLOCK_BOOT:
						goto Boot;

					case HCLOCK_DUMMY+0:

						regs.status = 0;
						scanline = SCANLINE_HDUMMY;

					case HCLOCK_DUMMY+8:
					case HCLOCK_DUMMY+16:
					case HCLOCK_DUMMY+24:
					case HCLOCK_DUMMY+32:
					case HCLOCK_DUMMY+40:
					case HCLOCK_DUMMY+48:
					case HCLOCK_DUMMY+56:
					case HCLOCK_DUMMY+64:
					case HCLOCK_DUMMY+72:
					case HCLOCK_DUMMY+80:
					case HCLOCK_DUMMY+88:
					case HCLOCK_DUMMY+96:
					case HCLOCK_DUMMY+104:
					case HCLOCK_DUMMY+112:
					case HCLOCK_DUMMY+120:
					case HCLOCK_DUMMY+128:
					case HCLOCK_DUMMY+136:
					case HCLOCK_DUMMY+144:
					case HCLOCK_DUMMY+152:
					case HCLOCK_DUMMY+160:
					case HCLOCK_DUMMY+168:
					case HCLOCK_DUMMY+176:
					case HCLOCK_DUMMY+184:
					case HCLOCK_DUMMY+192:
					case HCLOCK_DUMMY+200:
					case HCLOCK_DUMMY+208:
					case HCLOCK_DUMMY+216:
					case HCLOCK_DUMMY+224:
					case HCLOCK_DUMMY+232:
					case HCLOCK_DUMMY+240:
					case HCLOCK_DUMMY+248:
					HDummyBg:

						OpenName();
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case HCLOCK_DUMMY+2:
					case HCLOCK_DUMMY+10:
					case HCLOCK_DUMMY+18:
					case HCLOCK_DUMMY+26:
					case HCLOCK_DUMMY+34:
					case HCLOCK_DUMMY+42:
					case HCLOCK_DUMMY+50:
					case HCLOCK_DUMMY+58:
					case HCLOCK_DUMMY+66:
					case HCLOCK_DUMMY+74:
					case HCLOCK_DUMMY+82:
					case HCLOCK_DUMMY+90:
					case HCLOCK_DUMMY+98:
					case HCLOCK_DUMMY+106:
					case HCLOCK_DUMMY+114:
					case HCLOCK_DUMMY+122:
					case HCLOCK_DUMMY+130:
					case HCLOCK_DUMMY+138:
					case HCLOCK_DUMMY+146:
					case HCLOCK_DUMMY+154:
					case HCLOCK_DUMMY+162:
					case HCLOCK_DUMMY+170:
					case HCLOCK_DUMMY+178:
					case HCLOCK_DUMMY+186:
					case HCLOCK_DUMMY+194:
					case HCLOCK_DUMMY+202:
					case HCLOCK_DUMMY+210:
					case HCLOCK_DUMMY+218:
					case HCLOCK_DUMMY+226:
					case HCLOCK_DUMMY+234:
					case HCLOCK_DUMMY+242:
					case HCLOCK_DUMMY+250:

						OpenAttribute();
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case HCLOCK_DUMMY+4:
					case HCLOCK_DUMMY+12:
					case HCLOCK_DUMMY+20:
					case HCLOCK_DUMMY+28:
					case HCLOCK_DUMMY+36:
					case HCLOCK_DUMMY+44:
					case HCLOCK_DUMMY+52:
					case HCLOCK_DUMMY+60:
					case HCLOCK_DUMMY+68:
					case HCLOCK_DUMMY+76:
					case HCLOCK_DUMMY+84:
					case HCLOCK_DUMMY+92:
					case HCLOCK_DUMMY+100:
					case HCLOCK_DUMMY+108:
					case HCLOCK_DUMMY+116:
					case HCLOCK_DUMMY+124:
					case HCLOCK_DUMMY+132:
					case HCLOCK_DUMMY+140:
					case HCLOCK_DUMMY+148:
					case HCLOCK_DUMMY+156:
					case HCLOCK_DUMMY+164:
					case HCLOCK_DUMMY+172:
					case HCLOCK_DUMMY+180:
					case HCLOCK_DUMMY+188:
					case HCLOCK_DUMMY+196:
					case HCLOCK_DUMMY+204:
					case HCLOCK_DUMMY+212:
					case HCLOCK_DUMMY+220:
					case HCLOCK_DUMMY+228:
					case HCLOCK_DUMMY+236:
					case HCLOCK_DUMMY+244:
					case HCLOCK_DUMMY+252:

						OpenPattern( regs.ctrl[0] << 8 & 0x1000 );
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case HCLOCK_DUMMY+6:
					case HCLOCK_DUMMY+14:
					case HCLOCK_DUMMY+22:
					case HCLOCK_DUMMY+30:
					case HCLOCK_DUMMY+38:
					case HCLOCK_DUMMY+46:
					case HCLOCK_DUMMY+54:
					case HCLOCK_DUMMY+62:
					case HCLOCK_DUMMY+70:
					case HCLOCK_DUMMY+78:
					case HCLOCK_DUMMY+86:
					case HCLOCK_DUMMY+94:
					case HCLOCK_DUMMY+102:
					case HCLOCK_DUMMY+110:
					case HCLOCK_DUMMY+118:
					case HCLOCK_DUMMY+126:
					case HCLOCK_DUMMY+134:
					case HCLOCK_DUMMY+142:
					case HCLOCK_DUMMY+150:
					case HCLOCK_DUMMY+158:
					case HCLOCK_DUMMY+166:
					case HCLOCK_DUMMY+174:
					case HCLOCK_DUMMY+182:
					case HCLOCK_DUMMY+190:
					case HCLOCK_DUMMY+198:
					case HCLOCK_DUMMY+206:
					case HCLOCK_DUMMY+214:
					case HCLOCK_DUMMY+222:
					case HCLOCK_DUMMY+230:
					case HCLOCK_DUMMY+238:
					case HCLOCK_DUMMY+246:
					case HCLOCK_DUMMY+254:

						OpenPattern( io.address | 0x8 );
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

						if (cycles.hClock != HCLOCK_DUMMY+256)
							goto HDummyBg;

					case HCLOCK_DUMMY+256:
					case HCLOCK_DUMMY+264:
					case HCLOCK_DUMMY+272:
					case HCLOCK_DUMMY+280:
					case HCLOCK_DUMMY+288:
					case HCLOCK_DUMMY+296:
					case HCLOCK_DUMMY+312:
					HDummySp:

						OpenName();
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case HCLOCK_DUMMY+258:
					case HCLOCK_DUMMY+266:
					case HCLOCK_DUMMY+274:
					case HCLOCK_DUMMY+282:
					case HCLOCK_DUMMY+290:
					case HCLOCK_DUMMY+298:
					case HCLOCK_DUMMY+306:
					case HCLOCK_DUMMY+314:

						OpenName();
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case HCLOCK_DUMMY+260:
					case HCLOCK_DUMMY+268:
					case HCLOCK_DUMMY+276:
					case HCLOCK_DUMMY+284:
					case HCLOCK_DUMMY+292:
					case HCLOCK_DUMMY+300:
					case HCLOCK_DUMMY+308:
					case HCLOCK_DUMMY+316:

						OpenPattern( OpenSprite() );
						cycles.hClock += 2;

						if (cycles.count <= cycles.hClock)
							break;

					case HCLOCK_DUMMY+262:
					case HCLOCK_DUMMY+270:
					case HCLOCK_DUMMY+278:
					case HCLOCK_DUMMY+286:
					case HCLOCK_DUMMY+294:
					case HCLOCK_DUMMY+302:
					case HCLOCK_DUMMY+310:
					case HCLOCK_DUMMY+318:

						OpenPattern( io.address | 0x8 );

						if (cycles.hClock != HCLOCK_DUMMY+318)
						{
							cycles.hClock += 2;

							if (cycles.count <= cycles.hClock)
								break;

							if (cycles.hClock != HCLOCK_DUMMY+304)
								goto HDummySp;
						}
						else
						{
							cycles.hClock = 320;
							cycles.vClock += HCLOCK_DUMMY;
							cycles.count -= HCLOCK_DUMMY;

							if (cycles.count <= cycles.hClock)
								break;

							goto HBlankBg;
						}

					case HCLOCK_DUMMY+304:

						scroll.address = scroll.latch;
						goto HDummySp;

					default:

						NST_UNREACHABLE();
				}
			}
			else
			{
				switch (cycles.hClock)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					case 25:
					case 26:
					case 27:
					case 28:
					case 29:
					case 30:
					case 31:
					case 32:
					case 33:
					case 34:
					case 35:
					case 36:
					case 37:
					case 38:
					case 39:
					case 40:
					case 41:
					case 42:
					case 43:
					case 44:
					case 45:
					case 46:
					case 47:
					case 48:
					case 49:
					case 50:
					case 51:
					case 52:
					case 53:
					case 54:
					case 55:
					case 56:
					case 57:
					case 58:
					case 59:
					case 60:
					case 61:
					case 62:
					case 63:
					case 64:
					case 65:
					case 66:
					case 67:
					case 68:
					case 69:
					case 70:
					case 71:
					case 72:
					case 73:
					case 74:
					case 75:
					case 76:
					case 77:
					case 78:
					case 79:
					case 80:
					case 81:
					case 82:
					case 83:
					case 84:
					case 85:
					case 86:
					case 87:
					case 88:
					case 89:
					case 90:
					case 91:
					case 92:
					case 93:
					case 94:
					case 95:
					case 96:
					case 97:
					case 98:
					case 99:
					case 100:
					case 101:
					case 102:
					case 103:
					case 104:
					case 105:
					case 106:
					case 107:
					case 108:
					case 109:
					case 110:
					case 111:
					case 112:
					case 113:
					case 114:
					case 115:
					case 116:
					case 117:
					case 118:
					case 119:
					case 120:
					case 121:
					case 122:
					case 123:
					case 124:
					case 125:
					case 126:
					case 127:
					case 128:
					case 129:
					case 130:
					case 131:
					case 132:
					case 133:
					case 134:
					case 135:
					case 136:
					case 137:
					case 138:
					case 139:
					case 140:
					case 141:
					case 142:
					case 143:
					case 144:
					case 145:
					case 146:
					case 147:
					case 148:
					case 149:
					case 150:
					case 151:
					case 152:
					case 153:
					case 154:
					case 155:
					case 156:
					case 157:
					case 158:
					case 159:
					case 160:
					case 161:
					case 162:
					case 163:
					case 164:
					case 165:
					case 166:
					case 167:
					case 168:
					case 169:
					case 170:
					case 171:
					case 172:
					case 173:
					case 174:
					case 175:
					case 176:
					case 177:
					case 178:
					case 179:
					case 180:
					case 181:
					case 182:
					case 183:
					case 184:
					case 185:
					case 186:
					case 187:
					case 188:
					case 189:
					case 190:
					case 191:
					case 192:
					case 193:
					case 194:
					case 195:
					case 196:
					case 197:
					case 198:
					case 199:
					case 200:
					case 201:
					case 202:
					case 203:
					case 204:
					case 205:
					case 206:
					case 207:
					case 208:
					case 209:
					case 210:
					case 211:
					case 212:
					case 213:
					case 214:
					case 215:
					case 216:
					case 217:
					case 218:
					case 219:
					case 220:
					case 221:
					case 222:
					case 223:
					case 224:
					case 225:
					case 226:
					case 227:
					case 228:
					case 229:
					case 230:
					case 231:
					case 232:
					case 233:
					case 234:
					case 235:
					case 236:
					case 237:
					case 238:
					case 239:
					case 240:
					case 241:
					case 242:
					case 243:
					case 244:
					case 245:
					case 246:
					case 247:
					case 248:
					case 249:
					case 250:
					case 251:
					case 252:
					case 253:
					case 254:
					case 255:
					HActiveOff:
					{
						const uint pixel = output.palette[(scroll.address & 0x3F00) == 0x3F00 ? (scroll.address & 0x001F) : 0];

						uint i = cycles.hClock;
						const uint hClock = NST_MIN(cycles.count,256);
						NST_ASSERT( i < hClock );

						cycles.hClock = hClock;
						tiles.index = (hClock - 1) & 8;

						byte* const NST_RESTRICT tile = tiles.pixels;
						Video::Screen::Pixel* NST_RESTRICT target = output.target;

						do
						{
							tile[i++ & 15] = 0;
							*target++ = pixel;
						}
						while (i != hClock);

						output.target = target;

						if (cycles.count <= 256)
							break;
					}

					case 256:

						cycles.hClock = 257;

						if (cycles.count <= 257)
							break;

					case 257:

						if (hBlankHook)
							hBlankHook.Execute();

						oam.visible = oam.output;
						cycles.hClock = 258;

						if (cycles.count <= 258)
							break;

					case 258:
					case 260:
					case 261:
					case 262:
					case 263:
					case 264:
					case 266:
					case 268:
					case 269:
					case 270:
					case 271:
					case 272:
					case 274:
					case 276:
					case 277:
					case 278:
					case 279:
					case 280:
					case 282:
					case 284:
					case 285:
					case 286:
					case 287:
					case 288:
					case 290:
					case 292:
					case 293:
					case 294:
					case 295:
					case 296:
					case 298:
					case 300:
					case 301:
					case 302:
					case 303:
					case 304:
					case 306:
					case 308:
					case 309:
					case 310:
					case 311:
					case 312:
					case 314:
					case 316:
					case 317:
					case 318:
					case 319:

						if (cycles.count <= 320)
						{
							cycles.hClock = cycles.count + ((cycles.count & 0x7) == 3 || (cycles.count & 0x7) == 1);
							break;
						}

					case 320:
					HBlankOff:

						cycles.hClock = 321;

						if (hActiveHook)
							hActiveHook.Execute();

						oam.buffered = oam.buffer;
						oam.spriteZeroInLine = false;
						oam.index = 0;
						oam.phase = &Ppu::EvaluateSpritesPhase0;

						if (cycles.count <= 321)
							break;

					case 321:
					case 322:
					case 323:
					case 324:
					case 325:
					case 326:
					case 327:
					case 328:
					case 329:
					case 330:
					case 331:
					case 332:
					case 333:
					case 334:
					case 335:
					case 336:
					case 337:

						cycles.hClock = cycles.count;

						if (cycles.count <= 338)
							break;

					case 338:

						if (scanline++ != 239)
						{
							tiles.mask = tiles.show[1];
							oam.mask = oam.show[1];

							if (scanline == 0 && model == PPU_RP2C02)
								output.burstPhase = (output.burstPhase + 1) % 3;

							cycles.vClock += 341;
							cycles.hClock = 0;

							if (cycles.count <= 341)
								break;

							cycles.count -= 341;

							goto HActiveOff;
						}
 						else
 						{
							if (ssleep >= 0)
							{
								scanline_sleep = 0;

								cycles.vClock += 341;
								cycles.hClock = 0;

								if (cycles.count <= 341)
									break;

								cycles.count -= 341;

								goto HActiveSleep;
							}
							else
							{
								cycles.hClock = HCLOCK_VBLANK_0;

								if (cycles.count <= HCLOCK_VBLANK_0)
									break;
							}
						}

					case HCLOCK_VBLANK_0:
						goto VBlank0;

					case HCLOCK_VBLANK_1:
						goto VBlank1;

					case HCLOCK_VBLANK_2:
						goto VBlank2;

					case HCLOCK_BOOT:
					Boot:

						regs.status |= Regs::STATUS_VBLANK;
						cycles.hClock = HCLOCK_DUMMY;
						cycles.count = Cpu::CYCLE_MAX;

						if (cycles.reset)
						{
							switch (model)
							{
								case PPU_RP2C07: cycles.reset = PPU_RP2C07_HVREGBOOT - PPU_RP2C07_HVSYNCBOOT; break;
								case PPU_DENDY:  cycles.reset = PPU_DENDY_HVREGBOOT  - PPU_DENDY_HVSYNCBOOT;  break;
								default:         cycles.reset = PPU_RP2C02_HVREGBOOT - PPU_RP2C02_HVSYNCBOOT; break;
							}
						}
						return;

					case HCLOCK_DUMMY+0:

						regs.status = 0;
						scanline = SCANLINE_HDUMMY;

					case HCLOCK_DUMMY+2:
					case HCLOCK_DUMMY+4:
					case HCLOCK_DUMMY+6:
					case HCLOCK_DUMMY+8:
					case HCLOCK_DUMMY+10:
					case HCLOCK_DUMMY+12:
					case HCLOCK_DUMMY+14:
					case HCLOCK_DUMMY+16:
					case HCLOCK_DUMMY+18:
					case HCLOCK_DUMMY+20:
					case HCLOCK_DUMMY+22:
					case HCLOCK_DUMMY+24:
					case HCLOCK_DUMMY+26:
					case HCLOCK_DUMMY+28:
					case HCLOCK_DUMMY+30:
					case HCLOCK_DUMMY+32:
					case HCLOCK_DUMMY+34:
					case HCLOCK_DUMMY+36:
					case HCLOCK_DUMMY+38:
					case HCLOCK_DUMMY+40:
					case HCLOCK_DUMMY+42:
					case HCLOCK_DUMMY+44:
					case HCLOCK_DUMMY+46:
					case HCLOCK_DUMMY+48:
					case HCLOCK_DUMMY+50:
					case HCLOCK_DUMMY+52:
					case HCLOCK_DUMMY+54:
					case HCLOCK_DUMMY+56:
					case HCLOCK_DUMMY+58:
					case HCLOCK_DUMMY+60:
					case HCLOCK_DUMMY+62:
					case HCLOCK_DUMMY+64:
					case HCLOCK_DUMMY+66:
					case HCLOCK_DUMMY+68:
					case HCLOCK_DUMMY+70:
					case HCLOCK_DUMMY+72:
					case HCLOCK_DUMMY+74:
					case HCLOCK_DUMMY+76:
					case HCLOCK_DUMMY+78:
					case HCLOCK_DUMMY+80:
					case HCLOCK_DUMMY+82:
					case HCLOCK_DUMMY+84:
					case HCLOCK_DUMMY+86:
					case HCLOCK_DUMMY+88:
					case HCLOCK_DUMMY+90:
					case HCLOCK_DUMMY+92:
					case HCLOCK_DUMMY+94:
					case HCLOCK_DUMMY+96:
					case HCLOCK_DUMMY+98:
					case HCLOCK_DUMMY+100:
					case HCLOCK_DUMMY+102:
					case HCLOCK_DUMMY+104:
					case HCLOCK_DUMMY+106:
					case HCLOCK_DUMMY+108:
					case HCLOCK_DUMMY+110:
					case HCLOCK_DUMMY+112:
					case HCLOCK_DUMMY+114:
					case HCLOCK_DUMMY+116:
					case HCLOCK_DUMMY+118:
					case HCLOCK_DUMMY+120:
					case HCLOCK_DUMMY+122:
					case HCLOCK_DUMMY+124:
					case HCLOCK_DUMMY+126:
					case HCLOCK_DUMMY+128:
					case HCLOCK_DUMMY+130:
					case HCLOCK_DUMMY+132:
					case HCLOCK_DUMMY+134:
					case HCLOCK_DUMMY+136:
					case HCLOCK_DUMMY+138:
					case HCLOCK_DUMMY+140:
					case HCLOCK_DUMMY+142:
					case HCLOCK_DUMMY+144:
					case HCLOCK_DUMMY+146:
					case HCLOCK_DUMMY+148:
					case HCLOCK_DUMMY+150:
					case HCLOCK_DUMMY+152:
					case HCLOCK_DUMMY+154:
					case HCLOCK_DUMMY+156:
					case HCLOCK_DUMMY+158:
					case HCLOCK_DUMMY+160:
					case HCLOCK_DUMMY+162:
					case HCLOCK_DUMMY+164:
					case HCLOCK_DUMMY+166:
					case HCLOCK_DUMMY+168:
					case HCLOCK_DUMMY+170:
					case HCLOCK_DUMMY+172:
					case HCLOCK_DUMMY+174:
					case HCLOCK_DUMMY+176:
					case HCLOCK_DUMMY+178:
					case HCLOCK_DUMMY+180:
					case HCLOCK_DUMMY+182:
					case HCLOCK_DUMMY+184:
					case HCLOCK_DUMMY+186:
					case HCLOCK_DUMMY+188:
					case HCLOCK_DUMMY+190:
					case HCLOCK_DUMMY+192:
					case HCLOCK_DUMMY+194:
					case HCLOCK_DUMMY+196:
					case HCLOCK_DUMMY+198:
					case HCLOCK_DUMMY+200:
					case HCLOCK_DUMMY+202:
					case HCLOCK_DUMMY+204:
					case HCLOCK_DUMMY+206:
					case HCLOCK_DUMMY+208:
					case HCLOCK_DUMMY+210:
					case HCLOCK_DUMMY+212:
					case HCLOCK_DUMMY+214:
					case HCLOCK_DUMMY+216:
					case HCLOCK_DUMMY+218:
					case HCLOCK_DUMMY+220:
					case HCLOCK_DUMMY+222:
					case HCLOCK_DUMMY+224:
					case HCLOCK_DUMMY+226:
					case HCLOCK_DUMMY+228:
					case HCLOCK_DUMMY+230:
					case HCLOCK_DUMMY+232:
					case HCLOCK_DUMMY+234:
					case HCLOCK_DUMMY+236:
					case HCLOCK_DUMMY+238:
					case HCLOCK_DUMMY+240:
					case HCLOCK_DUMMY+242:
					case HCLOCK_DUMMY+244:
					case HCLOCK_DUMMY+246:
					case HCLOCK_DUMMY+248:
					case HCLOCK_DUMMY+250:
					case HCLOCK_DUMMY+252:
					case HCLOCK_DUMMY+254:
					case HCLOCK_DUMMY+256:
					case HCLOCK_DUMMY+258:
					case HCLOCK_DUMMY+260:
					case HCLOCK_DUMMY+262:
					case HCLOCK_DUMMY+264:
					case HCLOCK_DUMMY+266:
					case HCLOCK_DUMMY+268:
					case HCLOCK_DUMMY+270:
					case HCLOCK_DUMMY+272:
					case HCLOCK_DUMMY+274:
					case HCLOCK_DUMMY+276:
					case HCLOCK_DUMMY+278:
					case HCLOCK_DUMMY+280:
					case HCLOCK_DUMMY+282:
					case HCLOCK_DUMMY+284:
					case HCLOCK_DUMMY+286:
					case HCLOCK_DUMMY+288:
					case HCLOCK_DUMMY+290:
					case HCLOCK_DUMMY+292:
					case HCLOCK_DUMMY+294:
					case HCLOCK_DUMMY+296:
					case HCLOCK_DUMMY+298:
					case HCLOCK_DUMMY+300:
					case HCLOCK_DUMMY+302:
					case HCLOCK_DUMMY+304:
					case HCLOCK_DUMMY+306:
					case HCLOCK_DUMMY+308:
					case HCLOCK_DUMMY+310:
					case HCLOCK_DUMMY+312:
					case HCLOCK_DUMMY+314:
					case HCLOCK_DUMMY+316:
					{
						NST_COMPILE_ASSERT( HCLOCK_DUMMY & 1 );

						cycles.hClock = cycles.count | 1;

						if (cycles.count <= HCLOCK_DUMMY+318)
							break;
					}

					case HCLOCK_DUMMY+318:

						cycles.hClock = 320;
						cycles.vClock += HCLOCK_DUMMY;
						cycles.count -= HCLOCK_DUMMY;

						if (cycles.count <= 320)
							break;

						goto HBlankOff;

					default:

						NST_UNREACHABLE();
				}
			}

			cycles.count = GetCycles();
		}
	}
}
