////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2026 Rupert Carmichael
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

/* References:
   https://github.com/libretro/libretro-fceumm/blob/master/src/boards/413.c
   https://github.com/mamedev/mame/blob/master/src/devices/bus/nes/batlab.cpp
*/

#include "NstBoard.hpp"
#include "NstBoardBatlabSrrx.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Batlab
			{
				Srrx::Srrx(const Context& c)
				: Board(c), irq(*c.cpu,*c.ppu) {}

				void Srrx::Irq::Reset(bool)
				{
					count = 0;
					latch = 0;
					enabled = false;
				}

				bool Srrx::Irq::Clock()
				{
					if (!count)
						count = latch;
					else
						--count;

					return enabled && !count;
				}

				void Srrx::SubReset(const bool hard)
				{
					irq.Reset( hard );

					/* The board decodes the speech port twice: once here, and
					 * again at $C000 where the APU can reach it. Super Russian
					 * Roulette only ever uses the $C000 window, so this one is
					 * inert - it is mapped because the hardware decodes it.
					*/
					Map( 0x4800U, 0x4FFFU, &Srrx::Peek_Pcm  );

					Map( 0x5000U, 0x5FFFU, &Srrx::Peek_5000 );
					Map( 0x6000U, 0x7FFFU, &Srrx::Peek_6000 );

					Map( 0x8000U, 0x8FFFU, &Srrx::Poke_8000 );
					Map( 0x9000U, 0x9FFFU, &Srrx::Poke_9000 );
					Map( 0xA000U, 0xBFFFU, &Srrx::Poke_A000 );
					Map( 0xC000U, 0xCFFFU, &Srrx::Peek_Pcm, &Srrx::Poke_C000 );
					Map( 0xD000U, 0xDFFFU, &Srrx::Poke_D000 );
					Map( 0xE000U, 0xFFFFU, &Srrx::Poke_E000 );

					if (hard)
					{
						regs[0] = 0;
						regs[1] = 0;
						regs[2] = 0;
						regs[3] = 0;

						pcmAddress = 0;
						pcmControl = 0;
					}

					pcmLastRead = 0;

					UpdateBanks();
				}

				void Srrx::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','R','R'>::V) );

					if (baseChunk == AsciiId<'S','R','R'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<4> data( state );

									for (uint i=0; i < 4; ++i)
										regs[i] = data[i] & 0x3F;

									break;
								}

								case AsciiId<'P','C','M'>::V:
								{
									State::Loader::Data<5> data( state );

									pcmAddress = data[0] | dword(data[1]) << 8 | dword(data[2]) << 16 | dword(data[3]) << 24;
									pcmControl = data[4];
									pcmLastRead = 0;
									break;
								}

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<3> data( state );

									irq.unit.count = data[0];
									irq.unit.latch = data[1];
									irq.unit.enabled = data[2] & 0x1;
									break;
								}
							}

							state.End();
						}
					}

					UpdateBanks();
				}

				void Srrx::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','R','R'>::V );

					{
						const byte data[4] =
						{
							static_cast<byte>(regs[0]),
							static_cast<byte>(regs[1]),
							static_cast<byte>(regs[2]),
							static_cast<byte>(regs[3])
						};

						state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					}

					{
						const byte data[5] =
						{
							static_cast<byte>(pcmAddress >>  0 & 0xFF),
							static_cast<byte>(pcmAddress >>  8 & 0xFF),
							static_cast<byte>(pcmAddress >> 16 & 0xFF),
							static_cast<byte>(pcmAddress >> 24 & 0xFF),
							static_cast<byte>(pcmControl)
						};

						state.Begin( AsciiId<'P','C','M'>::V ).Write( data ).End();
					}

					{
						const byte data[3] =
						{
							static_cast<byte>(irq.unit.count),
							static_cast<byte>(irq.unit.latch),
							static_cast<byte>(irq.unit.enabled ? 0x1 : 0x0)
						};

						state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					}

					state.End();
				}

				void Srrx::UpdateBanks()
				{
					prg.SwapBank<SIZE_8K,0x0000>( regs[1] );
					prg.SwapBank<SIZE_8K,0x2000>( regs[2] );

					/* $C000-$DFFF holds the 8k bank whose upper half is the fixed
					 * $D000 window; the lower half never reaches PRG-ROM because
					 * the PCM port is mapped over it.
					*/
					prg.SwapBank<SIZE_8K,0x4000>( 3 );
					prg.SwapBank<SIZE_8K,0x6000>( 4 );

					chr.SwapBank<SIZE_4K,0x0000>( regs[3] );
					chr.SwapBank<SIZE_4K,0x1000>( ~2U );
				}

				NES_PEEK_A(Srrx,5000)
				{
					return *prg.Source().Mem( SIZE_4K + (address & 0xFFF) );
				}

				NES_PEEK_A(Srrx,6000)
				{
					return *prg.Source().Mem( regs[0] * dword(SIZE_8K) + (address & 0x1FFF) );
				}

				/* The 8M speech ROM is exposed one byte at a time through a
				 * self-incrementing port. The game streams it by pointing the DMC
				 * at $C000 with a looping one-byte sample, so every DMA fetch
				 * advances the pointer.
				*/
				NES_PEEK(Srrx,Pcm)
				{
					if (misc.Size() == 0)
						return 0xFF;

					const uint data = *misc.Mem( pcmAddress );

					/* A DMA that halts the CPU on a read of this port repeats
					 * that read on each stolen cycle. The port steps once per
					 * access rather than once per bus cycle, so reads landing
					 * inside one access are filtered into one - the same way
					 * the controller ports filter theirs.
					*/
					const Cycle cycle = cpu.GetCycles();

					if (cycle < pcmLastRead || cycle > pcmLastRead + cpu.GetClock(READ_FILTER))
					{
						if (pcmControl & 0x2)
							++pcmAddress;

						pcmLastRead = cycle;
					}

					return data;
				}

				NES_POKE_D(Srrx,8000)
				{
					irq.Update();
					irq.unit.latch = data;
				}

				NES_POKE(Srrx,9000)
				{
					irq.Update();
					irq.unit.count = 0;
				}

				NES_POKE_A(Srrx,A000)
				{
					irq.Update();
					irq.unit.enabled = address & 0x1000;

					if (!(address & 0x1000))
						irq.ClearIRQ();
				}

				NES_POKE_D(Srrx,C000)
				{
					pcmAddress = pcmAddress << 1 | (data >> 7);
				}

				NES_POKE_D(Srrx,D000)
				{
					pcmControl = data;
				}

				NES_POKE_D(Srrx,E000)
				{
					regs[data >> 6] = data & 0x3F;
					UpdateBanks();
				}

				void Srrx::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
