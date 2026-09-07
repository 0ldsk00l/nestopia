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

#include "../NstTimer.hpp"
#include "NstBoard.hpp"
#include "NstBoardWhirlwind.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Whirlwind
			{
				void W2706::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &W2706::Peek_6000 );
					Map( 0x8FFFU,          &W2706::Poke_8FFF );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(~0U);
				}

				NES_PEEK_A(W2706,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(W2706,8FFF)
				{
					wrk.SwapBank<SIZE_8K,0x0000U>(data);
				}

				Lh53::Lh53(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Lh53::Irq::Reset(bool)
				{
					count = 0;
				}

				void Lh53::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &Lh53::Peek_6000 );
					Map( 0x6000U, 0x7FFFU, NOP_POKE );

					// PRG-RAM overlays the fixed ROM window here
					Map( 0xB800U, 0xD7FFU, &Lh53::Peek_B800, &Lh53::Poke_B800 );

					Map( 0xE000U, 0xEFFFU, &Lh53::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &Lh53::Poke_F000 );

					irq.Reset( hard, hard ? false : irq.Connected() );

					if (hard)
					{
						reg = 0;
						prg.SwapBank<SIZE_32K,0x0000>(3);
					}
				}

				void Lh53::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'L','H','3'>::V) );

					if (baseChunk == AsciiId<'L','H','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									reg = state.Read8();
									break;

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<3> data( state );

									irq.Connect( data[0] & 0x1 );
									irq.unit.count = data[1] | (data[2] << 8 & 0x1F00);

									break;
								}
							}

							state.End();
						}
					}
				}

				void Lh53::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						static_cast<byte>(irq.Connected() ? 0x1 : 0x0),
						static_cast<byte>(irq.unit.count >> 0 & 0xFF),
						static_cast<byte>(irq.unit.count >> 8 & 0x1F)
					};

					state.Begin( AsciiId<'L','H','3'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End();
					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					state.End();
				}

				NES_PEEK_A(Lh53,6000)
				{
					return *(prg.Source().Mem(reg * dword(SIZE_8K)) + (address & 0x1FFF));
				}

				NES_PEEK_A(Lh53,B800)
				{
					NST_VERIFY( wrk.Readable(0) );
					return wrk.Readable(0) ? wrk[0][address - 0xB800] : (address >> 8);
				}

				NES_POKE_AD(Lh53,B800)
				{
					NST_VERIFY( wrk.Writable(0) );

					if (wrk.Writable(0))
						wrk[0][address - 0xB800] = data;
				}

				NES_POKE_D(Lh53,E000)
				{
					irq.Update();

					irq.unit.count = 0;
					irq.ClearIRQ();
					irq.Connect( data & 0x2 );
				}

				NES_POKE_D(Lh53,F000)
				{
					reg = data;
				}

				bool Lh53::Irq::Clock()
				{
					// Asserted once on reaching the terminal count; the line is held
					// until a write to $E000-$EFFF acknowledges it.
					return count < 7560 && ++count == 7560;
				}

				void Lh53::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
