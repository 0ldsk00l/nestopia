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

#include "../NstMachine.hpp"
#include "../NstImage.hpp"
#include "NstApiMemory.hpp"

namespace Nes
{
	namespace Api
	{
		/* Region 0 is CPU RAM, then whatever the image reports (work RAM,
		 * mapper RAM, program ROM, pattern data), then the PPU side, which
		 * lives outside the image entirely.
		 */
		enum
		{
			PPU_REGIONS = 3   // nametable RAM, palette RAM, OAM
		};

		ulong Memory::NumRegions() const throw()
		{
			if (!emulator.image)
				return 0;

			return 1 + emulator.image->NumMemoryRegions() + PPU_REGIONS;
		}

		Memory::Region Memory::GetRegion(ulong index) const throw()
		{
			Region region;

			region.space    = SPACE_CPU;
			region.type     = TYPE_SYSTEM_RAM;
			region.address  = 0x0000;
			region.size     = 0;
			region.data     = NULL;
			region.battery  = false;
			region.writable = true;

			if (index >= NumRegions())
				return region;

			if (index == 0)
			{
				region.size = Core::Cpu::RAM_SIZE;
				region.data = emulator.cpu.GetRam();
				return region;
			}

			--index;

			const ulong imageRegions = emulator.image->NumMemoryRegions();

			if (index < imageRegions)
			{
				const Core::Image::MemoryRegion source
				(
					emulator.image->GetMemoryRegion( uint(index) )
				);

				switch (source.space)
				{
					case Core::Image::MemoryRegion::SPACE_PPU:      region.space = SPACE_PPU;      break;
					case Core::Image::MemoryRegion::SPACE_INTERNAL: region.space = SPACE_INTERNAL; break;
					default:                                        region.space = SPACE_CPU;      break;
				}

				switch (source.type)
				{
					case Core::Image::MemoryRegion::TYPE_EXPANSION_RAM: region.type = TYPE_EXPANSION_RAM; break;
					case Core::Image::MemoryRegion::TYPE_DISK_RAM:      region.type = TYPE_DISK_RAM;      break;
					case Core::Image::MemoryRegion::TYPE_PRG_ROM:       region.type = TYPE_PRG_ROM;       break;
					case Core::Image::MemoryRegion::TYPE_CHR:           region.type = TYPE_CHR;           break;
					default:                                            region.type = TYPE_WORK_RAM;      break;
				}

				region.address  = source.address;
				region.size     = source.size;
				region.data     = source.data;
				region.battery  = source.battery;
				region.writable = source.writable;
				return region;
			}

			switch (index - imageRegions)
			{
				case 0:

					/* 4k of nametable storage at PPU $2000. Which of the four
					 * 1k pages a given address reaches depends on the current
					 * mirroring, so the whole block is reported at once.
					 */
					region.space   = SPACE_PPU;
					region.type    = TYPE_NAMETABLE_RAM;
					region.address = 0x2000;
					region.size    = Core::SIZE_4K;
					region.data    = emulator.ppu.GetNmtMem().Source().Mem();
					break;

				case 1:

					region.space   = SPACE_PPU;
					region.type    = TYPE_PALETTE_RAM;
					region.address = 0x3F00;
					region.size    = 0x20;   // palette RAM
					region.data    = emulator.ppu.GetPaletteRam();
					break;

				default:

					/* Not reachable from either bus; only through $2003/$2004
					 * or a DMA from $4014.
					 */
					region.space   = SPACE_INTERNAL;
					region.type    = TYPE_OAM;
					region.address = 0;
					region.size    = 0x100;  // OAM
					region.data    = emulator.ppu.GetOamRam();
					break;
			}

			return region;
		}
	}
}
