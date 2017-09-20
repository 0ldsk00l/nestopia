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

#ifndef NST_PPU_H
#define NST_PPU_H

#ifndef NST_IO_PORT_H
#include "NstIoPort.hpp"
#endif

#include "NstIoAccessor.hpp"
#include "NstIoLine.hpp"
#include "NstHook.hpp"
#include "NstMemory.hpp"
#include "NstVideoScreen.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ppu
		{
		public:

			explicit Ppu(Cpu&);

			void Reset(bool,bool);
			void PowerOff();
			void BeginFrame(bool);
			void EndFrame();

			enum
			{
				SCANLINE_HDUMMY = -1,
				SCANLINE_VBLANK = 240
			};

			enum NmtMirroring
			{
				NMT_H = 0xC,
				NMT_V = 0xA,
				NMT_0 = 0x0,
				NMT_1 = 0xF
			};

			void SetModel(PpuModel,bool);
			void SetMirroring(NmtMirroring);
			void SetMirroring(const byte (&)[4]);
			uint SetAddressLineHook(const Core::Io::Line&);
			void SetHActiveHook(const Hook&);
			void SetHBlankHook(const Hook&);
			uint GetPixelCycles() const;
			void EnableCpuSynchronization();

			void LoadState(State::Loader&);
			void SaveState(State::Saver&,dword) const;

			class ChrMem : public Memory<SIZE_8K,SIZE_1K,2>
			{
				NES_DECL_ACCESSOR( Pattern );

			protected:

				Io::Accessor accessor;

			public:

				void ResetAccessor();

				template<typename T,typename U>
				void SetAccessor(T t,U u)
				{
					accessor.Set( t, u );
				}
			};

			class NmtMem : public Memory<SIZE_4K,SIZE_1K,2>
			{
				NES_DECL_ACCESSOR( Name_2000 );
				NES_DECL_ACCESSOR( Name_2400 );
				NES_DECL_ACCESSOR( Name_2800 );
				NES_DECL_ACCESSOR( Name_2C00 );

			protected:

				Io::Accessor accessors[4];

			public:

				void ResetAccessors();

				template<typename T,typename U,typename V,typename W,typename X>
				void SetAccessors(T t,U u,V v,W w,X x)
				{
					accessors[0].Set( t, u );
					accessors[1].Set( t, v );
					accessors[2].Set( t, w );
					accessors[3].Set( t, x );
				}
			};

		private:

			struct Chr : ChrMem
			{
				NST_FORCE_INLINE uint FetchPattern(uint) const;
			};

			struct Nmt : NmtMem
			{
				NST_FORCE_INLINE uint FetchName(uint) const;
				NST_FORCE_INLINE uint FetchAttribute(uint) const;
			};

			enum
			{
				HCLOCK_POSTRENDER = 340,
				HCLOCK_DUMMY    = 341,
				HCLOCK_VBLANK_0 = 681,
				HCLOCK_VBLANK_1 = 682,
				HCLOCK_VBLANK_2 = 684,
				HCLOCK_BOOT     = 685
			};

			NES_DECL_POKE( 2000 );
			NES_DECL_PEEK( 2002 );
			NES_DECL_PEEK( 2002_RC2C05_01_04 );
			NES_DECL_PEEK( 2002_RC2C05_02 );
			NES_DECL_PEEK( 2002_RC2C05_03 );
			NES_DECL_POKE( 2001 );
			NES_DECL_POKE( 2003 );
			NES_DECL_PEEK( 2004 );
			NES_DECL_POKE( 2004 );
			NES_DECL_POKE( 2005 );
			NES_DECL_POKE( 2006 );
			NES_DECL_PEEK( 2007 );
			NES_DECL_POKE( 2007 );
			NES_DECL_PEEK( 2xxx );
			NES_DECL_POKE( 2xxx );
			NES_DECL_PEEK( 3000 );
			NES_DECL_PEEK( 4014 );
			NES_DECL_POKE( 4014 );

			NES_DECL_HOOK( Sync );

			NST_FORCE_INLINE Cycle GetCycles() const;
			NST_FORCE_INLINE Cycle GetLocalCycles(Cycle) const;

			NST_FORCE_INLINE bool IsDead() const;
			NST_FORCE_INLINE uint Coloring() const;
			NST_FORCE_INLINE uint Emphasis() const;

			NST_FORCE_INLINE void UpdateAddressLine(uint);
			NST_FORCE_INLINE void UpdateVramAddress();

			NST_FORCE_INLINE void OpenName();
			NST_FORCE_INLINE void FetchName();
			NST_FORCE_INLINE void OpenAttribute();
			NST_FORCE_INLINE void FetchAttribute();
			NST_FORCE_INLINE void OpenPattern(uint);
			NST_FORCE_INLINE uint FetchSpPattern() const;
			NST_FORCE_INLINE void FetchBgPattern0();
			NST_FORCE_INLINE void FetchBgPattern1();

			NST_FORCE_INLINE void EvaluateSpritesEven();
			NST_FORCE_INLINE void EvaluateSpritesOdd();

			void EvaluateSpritesPhase0();
			void EvaluateSpritesPhase1();
			void EvaluateSpritesPhase2();
			void EvaluateSpritesPhase3();
			void EvaluateSpritesPhase4();
			void EvaluateSpritesPhase5();
			void EvaluateSpritesPhase6();
			void EvaluateSpritesPhase7();
			void EvaluateSpritesPhase8();
			void EvaluateSpritesPhase9();

			void Reset(bool,bool,bool);
			void Update(Cycle,uint=0);
			void UpdateStates();
			void UpdatePalette();
			void LoadExtendedSprites();

			NST_FORCE_INLINE uint OpenSprite() const;
			NST_FORCE_INLINE uint OpenSprite(const byte* NST_RESTRICT) const;
			NST_FORCE_INLINE  void LoadSprite(uint,uint,const byte* NST_RESTRICT);
			NST_SINGLE_CALL void PreLoadTiles();
			NST_SINGLE_CALL void LoadTiles();
			NST_FORCE_INLINE void RenderPixel();
			NST_SINGLE_CALL void RenderPixel255();
			NST_NO_INLINE void Run();

			struct Regs
			{
				enum
				{
					CTRL0_NAME_OFFSET        = 0x03,
					CTRL0_INC32              = 0x04,
					CTRL0_SP_OFFSET          = 0x08,
					CTRL0_BG_OFFSET          = 0x10,
					CTRL0_SP8X16             = 0x20,
					CTRL0_NMI                = 0x80,
					CTRL0_NMI_OCCUR          = 0x100,
					CTRL1_MONOCHROME         = 0x01,
					CTRL1_BG_NO_CLIP         = 0x02,
					CTRL1_SP_NO_CLIP         = 0x04,
					CTRL1_BG_ENABLED         = 0x08,
					CTRL1_SP_ENABLED         = 0x10,
					CTRL1_BG_ENABLED_NO_CLIP = CTRL1_BG_ENABLED|CTRL1_BG_NO_CLIP,
					CTRL1_SP_ENABLED_NO_CLIP = CTRL1_SP_ENABLED|CTRL1_SP_NO_CLIP,
					CTRL1_BG_SP_ENABLED      = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED,
					CTRL1_EMPHASIS           = 0xE0,
					STATUS_LATCH             = 0x1F,
					STATUS_SP_OVERFLOW       = 0x20,
					STATUS_SP_ZERO_HIT       = 0x40,
					STATUS_VBLANK            = 0x80,
					STATUS_BITS              = STATUS_SP_OVERFLOW|STATUS_SP_ZERO_HIT|STATUS_VBLANK,
					STATUS_VBLANKING         = 0x100,
					FRAME_ODD                = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED
				};

				uint ctrl[2];
				uint status;
				uint frame;
				uint oam;
			};

			struct Scroll
			{
				enum
				{
					X_TILE    = 0x001F,
					Y_TILE    = 0x03E0,
					Y_FINE    = 0x7000,
					LOW       = 0x00FF,
					HIGH      = 0xFF00,
					NAME      = 0x0C00,
					NAME_LOW  = 0x0400,
					NAME_HIGH = 0x0800
				};

				NST_FORCE_INLINE void ClockX();
				NST_SINGLE_CALL  void ResetX();
				NST_SINGLE_CALL  void ClockY();

				uint address;
				uint toggle;
				uint latch;
				uint xFine;
			};

			struct Tiles
			{
				Tiles();

				byte pattern[2];
				byte attribute;
				byte index;
				byte pixels[16];
				uint mask;
				byte show[2];
				const byte padding0;
				const byte padding1;
			};

			struct Palette
			{
				enum
				{
					SIZE          = 0x20,
					COLORS        = 0x40,
					SPRITE_OFFSET = 0x10,
					COLOR         = 0x3F,
					MONO          = 0x30
				};

				byte ram[SIZE];
			};

			struct Output
			{
				explicit Output(Video::Screen::Pixel*);

				Video::Screen::Pixel* target;
				Video::Screen::Pixel* pixels;
				uint burstPhase;
				word palette[Palette::SIZE];
				uint bgColor;
			};

			struct Oam
			{
				Oam();

				enum
				{
					SIZE             = 0x100,
					OFFSET_TO_0_1    = 0xF8,
					STD_LINE_SPRITES = 8,
					MAX_LINE_SPRITES = 32,
					GARBAGE          = 0xFF,
					COLOR            = 0x03,
					BEHIND           = 0x20,
					X_FLIP           = 0x40,
					Y_FLIP           = 0x80,
					XFINE            = 0x07,
					RANGE_MSB        = 0x08,
					TILE_LSB         = 0x01
				};

				struct Output
				{
					byte x;
					byte behind;
					byte zero;
					byte palette;
					byte pixels[8];
				};

				typedef void (Ppu::*Phase)();

				byte ram[0x100];
				byte buffer[MAX_LINE_SPRITES * 4];

				const byte* limit;
				Output* visible;
				Phase phase;
				uint latch;
				uint index;
				byte* buffered;
				uint address;
				uint height;
				uint mask;
				byte show[2];
				bool spriteZeroInLine;
				bool spriteLimit;

				Output output[MAX_LINE_SPRITES];
			};

			struct NameTable
			{
				enum
				{
					SIZE = SIZE_2K,
					GARBAGE = 0xFF
				};

				byte ram[SIZE];
			};

			struct TileLut
			{
				TileLut();

				byte block[0x400][4];
			};

			struct Io
			{
				enum
				{
					BUFFER_GARBAGE = 0xE8
				};

				uint address;
				uint pattern;
				uint latch;
				uint buffer;
				Core::Io::Line line;
			};

			Cpu& cpu;

			struct
			{
				Cycle count;
				Cycle hClock;
				Cycle vClock;
				uint  one;
				Cycle reset;
			}   cycles;

			Io io;
			Regs regs;
			Scroll scroll;
			Tiles tiles;
			Chr chr;
			Nmt nmt;
			int scanline;
			int scanline_sleep;
			int ssleep;
			bool overclocked;

			PpuModel model;
			Hook hActiveHook;
			Hook hBlankHook;
			const byte* rgbMap;
			const byte* yuvMap;
			Oam oam;
			Palette palette;
			NameTable nameTable;
			const TileLut tileLut;
			Video::Screen screen;

			static const byte yuvMaps[4][0x40];

		public:
			Output output;

			void Update()
			{
				Update(0);
			}

			PpuModel GetModel() const
			{
				return model;
			}

			ibool IsEnabled() const
			{
				return regs.ctrl[1] & Regs::CTRL1_BG_SP_ENABLED;
			}

			int GetScanline() const
			{
				return scanline;
			}

			uint GetCtrl(uint i) const
			{
				NST_ASSERT( i < 2 );
				return regs.ctrl[i];
			}

			Video::Screen& GetScreen()
			{
				return screen;
			}

			Video::Screen::Pixel* GetOutputPixels()
			{
				return output.pixels;
			}

			void SetOutputPixels(Video::Screen::Pixel* pixels)
			{
				NST_ASSERT( pixels );
				output.pixels = pixels;
			}

			const Palette& GetPalette() const
			{
				return palette;
			}

			uint GetPixel(uint i) const
			{
				NST_ASSERT( i < Video::Screen::PIXELS );
				return output.pixels[i];
			}

			uint GetYuvColor(uint i) const
			{
				NST_ASSERT( i < Palette::COLORS );
				return yuvMap ? yuvMap[i] : i;
			}

			ChrMem& GetChrMem()
			{
				return chr;
			}

			NmtMem& GetNmtMem()
			{
				return nmt;
			}

			Cycle GetClock(dword count=1) const
			{
				NST_ASSERT( count );
				return cycles.one * count;
			}

			Cycle GetHSyncClock() const
			{
				return model == PPU_RP2C07 ? PPU_RP2C07_HSYNC : model == PPU_DENDY ? PPU_DENDY_HSYNC : PPU_RP2C02_HSYNC;
			}

			Cycle GetHVIntClock() const
			{
				return model == PPU_RP2C07 ? PPU_RP2C07_HVINT : model == PPU_DENDY ? PPU_DENDY_HVINT : PPU_RP2C02_HVINT;
			}

			const Core::Io::Line& GetAddressLineHook() const
			{
				return io.line;
			}

			bool IsShortFrame() const
			{
				return regs.ctrl[1] & regs.frame;
			}

			uint GetBurstPhase() const
			{
				return output.burstPhase;
			}

			void EnableSpriteLimit(bool enable)
			{
				oam.spriteLimit = enable;
			}

			bool HasSpriteLimit() const
			{
				return oam.spriteLimit;
			}

			bool GetOverclockState() const
			{
				return overclocked;
			}

			void SetOverclockState(bool overclock2x)
			{
				overclocked = overclock2x;
			}
		};
	}
}

#endif
