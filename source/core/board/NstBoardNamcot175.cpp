////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
// Copyright (C) 2014 R. Danbrook
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
#include "NstBoard.hpp"
#include "../NstTimer.hpp"
#include "../NstFile.hpp"
#include "NstBoardNamcot175.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				N175::N175(const Context& c)
				:
				Board (c),
				irq   (*c.cpu)
				{
				}

				void N175::SubReset(const bool hard)
				{
				
					irq.Reset( hard, hard || irq.Connected() );
					
					Map( 0x8000U, 0x87FFU, CHR_SWAP_1K_0 );
					Map( 0x8800U, 0x8FFFU, CHR_SWAP_1K_1 );
					Map( 0x9000U, 0x97FFU, CHR_SWAP_1K_2 );
					Map( 0x9800U, 0x9FFFU, CHR_SWAP_1K_3 );
					Map( 0xA000U, 0xA7FFU, CHR_SWAP_1K_4 );
					Map( 0xA800U, 0xAFFFU, CHR_SWAP_1K_5 );
					Map( 0xB000U, 0xB7FFU, CHR_SWAP_1K_6 );
					Map( 0xB800U, 0xBFFFU, CHR_SWAP_1K_7 );
					Map( 0xE000U, 0xE7FFU, PRG_SWAP_8K_0 );
					Map( 0xE800U, 0xEFFFU, PRG_SWAP_8K_1 );
					Map( 0xF000U, 0xF7FFU, PRG_SWAP_8K_2 );
				}

				void N175::Irq::Reset(const bool hard)
				{
					if (hard)
						count = 0;
				}

				void N175::Load(File& file)
				{
					if (board.HasBattery())
					{
						const File::LoadBlock block[] =
						{
							{ wrk.Source().Mem(), board.GetWram() }
						};

						file.Load( File::BATTERY, block );
					}
					else
					{
						Board::Load( file );
					}
				}

				void N175::Save(File& file) const
				{
					if (board.HasBattery())
					{
						const File::SaveBlock block[] =
						{
							{ wrk.Source().Mem(), board.GetWram() }
						};

						file.Save( File::BATTERY, block );
					}
					else
					{
						Board::Save( file );
					}
				}

				void N175::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'N','6','3'>::V) );

					if (baseChunk == AsciiId<'N','6','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<3> data( state );
									irq.unit.count = data[1] | (data[2] << 8 & 0x7F00) | (data[0] << 15 & 0x8000);
									break;
								}
							}

							state.End();
						}
					}
				}

				void N175::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'N','6','3'>::V );

					const byte data[3] =
					{
						irq.unit.count >> 15,
						irq.unit.count >> 0 & 0xFF,
						irq.unit.count >> 8 & 0x7F
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				bool N175::Irq::Clock()
				{
					return (count - 0x8000 < 0x7FFF) && (++count == 0xFFFF);
				}

				NES_PEEK(N175,5000)
				{
					irq.Update();
					return irq.unit.count & 0xFF;
				}

				NES_POKE_D(N175,5000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xFF00) | data;
					irq.ClearIRQ();
				}

				NES_PEEK(N175,5800)
				{
					irq.Update();
					return irq.unit.count >> 8;
				}

				NES_POKE_D(N175,5800)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0x00FF) | (data << 8);
					irq.ClearIRQ();
				}

				void N175::SwapNmt(const uint address,const uint data) const
				{
					ppu.Update();
					nmt.Source( data < 0xE0 ).SwapBank<SIZE_1K>( address, data );
				}

				NES_POKE_D(N175,C000)
				{
					SwapNmt( 0x0000, data );
				}

				NES_POKE_D(N175,C800)
				{
					SwapNmt( 0x0400, data );
				}

				NES_POKE_D(N175,D000)
				{
					SwapNmt( 0x0800, data );
				}

				NES_POKE_D(N175,D800)
				{
					SwapNmt( 0x0C00, data );
				}

				void N175::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
