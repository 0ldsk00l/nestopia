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

#ifndef NST_IMAGE_H
#define NST_IMAGE_H

#include <iosfwd>

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			class Loader;
			class Saver;
		}

		class ImageDatabase;
		class Cpu;
		class Apu;
		class Ppu;

		class NST_NO_VTABLE Image
		{
		public:

			enum Type
			{
				UNKNOWN   = 0x0,
				CARTRIDGE = 0x1,
				DISK      = 0x2,
				SOUND     = 0x4
			};

			typedef void* ExternalDevice;

			/* A block of emulated memory a frontend may inspect, e.g. to build
			 * an achievement or cheat-search memory map. Regions are reported
			 * whether or not they are battery backed; the battery file path is
			 * about persistence, not visibility.
			*/
			struct MemoryRegion
			{
				enum Space
				{
					SPACE_CPU,      // visible on the 6502 bus
					SPACE_PPU,      // visible on the PPU bus
					SPACE_INTERNAL  // addressable by neither, e.g. OAM
				};

				enum Type
				{
					TYPE_WORK_RAM,      // cartridge RAM, commonly $6000-$7FFF
					TYPE_EXPANSION_RAM, // mapper RAM outside the usual window
					TYPE_DISK_RAM,      // FDS program RAM
					TYPE_PRG_ROM,       // program ROM, as currently banked
					TYPE_CHR            // pattern data, as currently banked
				};

				Space space;
				Type type;
				word address;   // address within its space
				dword size;
				byte* data;
				bool battery;
				bool writable;
			};

			enum ExternalDeviceType
			{
				EXT_DIP_SWITCHES = 1,
				EXT_BARCODE_READER
			};

			struct Context
			{
				const Type type;
				Cpu& cpu;
				Apu& apu;
				Ppu& ppu;
				std::istream& stream;
				std::istream* const patch;
				const bool patchBypassChecksum;
				Result* const patchResult;
				const FavoredSystem favoredSystem;
				const bool forcedSystem;
				const bool askProfile;
				const ImageDatabase* const database;
				Result result;

				Context(Type t,Cpu& c,Apu& a,Ppu& p,std::istream& s,std::istream* h,bool k,Result* r,FavoredSystem f,bool e,bool b,const ImageDatabase* d)
				: type(t), cpu(c), apu(a), ppu(p), stream(s), patch(h), patchBypassChecksum(k), patchResult(r), favoredSystem(f), forcedSystem(e), askProfile(b), database(d), result(RESULT_OK) {}
			};

			static Image* Load(Context&);
			static void Unload(Image*);

			virtual void Reset(bool) = 0;

			virtual bool PowerOff()
			{
				return true;
			}

			virtual void VSync() {}

			virtual void LoadState(State::Loader&) {}
			virtual void SaveState(State::Saver&,dword) const {}

			virtual uint GetDesiredController(uint) const;
			virtual uint GetDesiredAdapter() const;
			virtual Region GetDesiredRegion() const = 0;
			virtual System GetDesiredSystem(Region,CpuModel* = NULL,PpuModel* = NULL) const;

			virtual dword GetPrgCrc() const
			{
				return 0;
			}

			virtual ExternalDevice QueryExternalDevice(ExternalDeviceType)
			{
				return NULL;
			}

			virtual uint NumMemoryRegions() const
			{
				return 0;
			}

			virtual MemoryRegion GetMemoryRegion(uint) const
			{
				MemoryRegion region;
				region.space    = MemoryRegion::SPACE_CPU;
				region.type     = MemoryRegion::TYPE_WORK_RAM;
				region.address  = 0;
				region.size     = 0;
				region.data     = NULL;
				region.battery  = false;
				region.writable = false;
				return region;
			}

		protected:

			explicit Image(Type);
			virtual ~Image() {}

			enum
			{
				INES_ID    = AsciiId<'N','E','S'>::V | 0x1AUL << 24,
				UNIF_ID    = AsciiId<'U','N','I','F'>::V,
				FDS_ID     = AsciiId<'F','D','S'>::V | 0x1AUL << 24,
				FDS_RAW_ID = 0x494E2A01,
				NSF_ID     = AsciiId<'N','E','S','M'>::V
			};

		private:

			const Type type;

		public:

			Type GetType() const
			{
				return type;
			}
		};
	}
}

#endif
