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

#ifndef NST_API_MEMORY_H
#define NST_API_MEMORY_H

#include "NstApi.hpp"

namespace Nes
{
	namespace Api
	{
		/**
		* Emulated memory enumeration.
		*
		* Reports the blocks of memory the running machine holds, so a frontend
		* can build a memory map for achievements, cheat searching or debugging.
		* Regions are reported whether or not they are battery backed: a battery
		* controls whether contents are persisted to disk, not whether the memory
		* exists. Cartridge work RAM in particular is present on many boards that
		* have no battery at all, MMC3 being the common case.
		*
		* Pointers stay valid until the image is unloaded or the machine is
		* reset with a different image, and refer to live emulator memory.
		*/
		class Memory : public Base
		{
		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Memory(T& instance)
			: Base(instance) {}

			/**
			* Which bus a region is visible on.
			*
			* A frontend building an address map needs to keep the two buses
			* apart: an address means different things on each, and some blocks
			* are reachable from neither.
			*/
			enum Space
			{
				/**
				* Visible on the 6502 bus, $0000-$FFFF.
				*/
				SPACE_CPU,
				/**
				* Visible on the PPU bus, $0000-$3FFF.
				*/
				SPACE_PPU,
				/**
				* Addressable by neither bus, such as OAM.
				*/
				SPACE_INTERNAL
			};

			/**
			* Type of memory a region holds.
			*/
			enum Type
			{
				/**
				* CPU RAM, 2k at $0000.
				*/
				TYPE_SYSTEM_RAM,
				/**
				* Cartridge work RAM, commonly 8k at $6000.
				*/
				TYPE_WORK_RAM,
				/**
				* Mapper RAM outside the usual work RAM window, such as
				* the MMC5 expansion RAM at $5C00.
				*/
				TYPE_EXPANSION_RAM,
				/**
				* Famicom Disk System program RAM.
				*/
				TYPE_DISK_RAM,
				/**
				* Cartridge program ROM, as currently banked.
				*/
				TYPE_PRG_ROM,
				/**
				* Pattern data, as currently banked. Writable only on boards
				* carrying character RAM rather than ROM.
				*/
				TYPE_CHR,
				/**
				* Nametable RAM.
				*/
				TYPE_NAMETABLE_RAM,
				/**
				* Palette RAM, 32 bytes.
				*/
				TYPE_PALETTE_RAM,
				/**
				* Object attribute memory, 256 bytes.
				*/
				TYPE_OAM
			};

			/**
			* A single block of emulated memory.
			*/
			struct Region
			{
				/**
				* Which bus the block is visible on.
				*/
				Space space;
				/**
				* What the block holds.
				*/
				Type type;
				/**
				* CPU address the block appears at, or 0 if it is not
				* directly visible in the CPU address space.
				*/
				ushort address;
				/**
				* Size in bytes.
				*/
				ulong size;
				/**
				* Live pointer to the block, or NULL if the region index
				* is out of range.
				*/
				void* data;
				/**
				* True if the block is battery backed and therefore also
				* persisted through the file callbacks.
				*/
				bool battery;
				/**
				* False for read-only memory such as program ROM.
				*/
				bool writable;
			};

			/**
			* Returns the number of regions available.
			*
			* Zero before an image is loaded. Region 0 is always system RAM
			* once a machine is up.
			*
			* Regions covering banked memory report what is mapped at the time
			* of the call. A caller that needs to track bank switches has to
			* re-read them rather than cache the pointers.
			*
			* @return number of regions
			*/
			ulong NumRegions() const throw();

			/**
			* Returns a region by index.
			*
			* @param index region index, less than NumRegions()
			* @return region, with a NULL data pointer if index is out of range
			*/
			Region GetRegion(ulong index) const throw();
		};
	}
}

#endif
