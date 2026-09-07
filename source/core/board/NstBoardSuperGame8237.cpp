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

#include "NstBoard.hpp"
#include "NstBoardMmc3.hpp"
#include "NstBoardSuperGame8237.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace SuperGame
			{
				// iNES mapper 215, UNIF UNL-8237 (submapper 0) and UNL-8237A (submapper 1).
				// An MMC3 clone whose register addresses and bank-select values are
				// scrambled by one of eight patterns chosen through $5007.

				void Unl8237::SubReset(const bool hard)
				{
					// $5001 powers up at $xF and reverts to it whenever M2 is
					// interrupted, so a soft reset restores it as well.
					exRegs[0] = 0x00;
					exRegs[1] = 0x0F;
					exRegs[2] = 0x00;

					Mmc3::SubReset( hard );

					Map( 0x5000U, 0x5FFFU, &Unl8237::Poke_5000 );
					Map( 0x8000U, 0xFFFFU, &Unl8237::Poke_8000 );
				}

				void Unl8237::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','8','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
								exRegs[2] = data[2] & 0x7;
							}

							state.End();
						}
					}
					else if (baseChunk != AsciiId<'S','B','G'>::V && baseChunk != AsciiId<'S','P','2'>::V)
					{
						Mmc3::SubLoad( state, baseChunk );
					}

					// 'SBG' and 'SP2' are the chunks of the boards this one replaces.
					// Their registers have no counterpart here, so they are dropped;
					// the banking itself is restored from the PRG/CHR page chunks.
				}

				void Unl8237::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'S','8','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( exRegs ).End().End();
				}

				// $5001 bits 1-0 are PRG A18/A19 and bits 3-2 are CHR A18/A19 on the
				// UNL-8237. The UNL-8237A widens both to A20, taking PRG from bits 3/1/0
				// and CHR from bits 3/2/1.

				uint Unl8237::PrgOuter() const
				{
					if (variant)
						return (exRegs[1] << 5 & 0x60) | (exRegs[1] << 4 & 0x80);
					else
						return (exRegs[1] << 5 & 0x60);
				}

				uint Unl8237::ChrOuter() const
				{
					if (variant)
						return (exRegs[1] << 7 & 0x700);
					else
						return (exRegs[1] << 6 & 0x300);
				}

				void NST_FASTCALL Unl8237::UpdatePrg(uint address,uint bank)
				{
					if (exRegs[0] & 0x80)
						return;

					// $5000 bit 6 shrinks the window to 128k and takes A17 from
					// $5001 bit 4 instead of from the MMC3.
					if (exRegs[0] & 0x40)
						bank = (bank & 0x0F) | (exRegs[1] & 0x10);
					else
						bank = (bank & 0x1F);

					prg.SwapBank<SIZE_8K>( address, PrgOuter() | bank );
				}

				void NST_FASTCALL Unl8237::UpdateChr(uint address,uint bank) const
				{
					if (exRegs[0] & 0x40)
						bank = (bank & 0x7F) | (exRegs[1] << 2 & 0x80);

					chr.SwapBank<SIZE_1K>( address, ChrOuter() | bank );
				}

				void Unl8237::UpdatePrg()
				{
					if (exRegs[0] & 0x80)
					{
						uint bank = exRegs[0] & 0x0F;

						if (exRegs[0] & 0x40)
							bank = (bank & 0x07) | (exRegs[1] >> 1 & 0x08);

						bank |= PrgOuter() >> 1;

						if (exRegs[0] & 0x20)
							prg.SwapBank<SIZE_32K,0x0000>( bank >> 1 );
						else
							prg.SwapBanks<SIZE_16K,0x0000>( bank, bank );
					}
					else
					{
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_AD(Unl8237,5000)
				{
					switch (address & 0x7)
					{
						case 0x0:

							if (exRegs[0] != data)
							{
								exRegs[0] = data;
								UpdatePrg();
								Mmc3::UpdateChr();
							}
							break;

						case 0x1:

							if (exRegs[1] != data)
							{
								exRegs[1] = data;
								UpdatePrg();
								Mmc3::UpdateChr();
							}
							break;

						case 0x7:

							exRegs[2] = data & 0x7;
							break;
					}
				}

				NES_POKE_AD(Unl8237,8000)
				{
					// Index and result are (A13,A14,A0) of the MMC3 register address:
					// 0=$8000 1=$8001 2=$A000 3=$A001 4=$C000 5=$C001 6=$E000 7=$E001.
					//
					// Mode 4 follows the wiki: $A001 is the IRQ latch and $C001 the
					// IRQ reload. fceumm and Mesen swap those two, but their mode 1
					// places the pair the same way round as the wiki does, so the
					// swap looks like a transcription slip in FCEUX that Mesen
					// inherited. If mode 4 games regress, exchange the 4 and 5 below.
					static const byte lutAddress[8][8] =
					{
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 3, 2, 0, 4, 1, 5, 6, 7 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 5, 0, 1, 2, 3, 7, 6, 4 },
						{ 3, 1, 0, 4, 2, 5, 6, 7 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 }
					};

					static const byte lutData[8][8] =
					{
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 0, 2, 6, 1, 7, 3, 4, 5 },
						{ 0, 5, 4, 1, 7, 2, 6, 3 },
						{ 0, 6, 3, 7, 5, 2, 4, 1 },
						{ 0, 2, 5, 3, 6, 1, 7, 4 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 },
						{ 0, 1, 2, 3, 4, 5, 6, 7 }
					};

					switch (lutAddress[exRegs[2]][(address >> 12 & 0x6) | (address & 0x1)])
					{
						case 0:

							// Only the bank-select index is scrambled; bits 7-6 pass through.
							Mmc3::NES_DO_POKE( 8000, 0x8000, (data & 0xC0) | lutData[exRegs[2]][data & 0x7] );
							break;

						case 1: Mmc3::NES_DO_POKE( 8001, 0x8001, data ); break;

						case 2:

							if (board.GetNmt() != Type::NMT_FOURSCREEN)
								SetMirroringHV( data );
							break;

						case 3: Mmc3::NES_DO_POKE( A001, 0xA001, data ); break;
						case 4: Mmc3::NES_DO_POKE( C000, 0xC000, data ); break;
						case 5: Mmc3::NES_DO_POKE( C001, 0xC001, data ); break;
						case 6: Mmc3::NES_DO_POKE( E000, 0xE000, data ); break;

						default: Mmc3::NES_DO_POKE( E001, 0xE001, data ); break;
					}
				}
			}
		}
	}
}
