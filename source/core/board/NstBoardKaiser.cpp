////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
// Copyright (C) 2021 Rupert Carmichael
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
#include "NstBoardKaiser.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kaiser
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Ks202::Ks202(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Ks202::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						count = 0;
						latch = 0;
						ctrl = 0;
					}
				}

				void Ks202::SubReset(const bool hard)
				{
					Map( 0x8000U, 0x8FFFU, &Ks202::Poke_8000 );
					Map( 0x9000U, 0x9FFFU, &Ks202::Poke_9000 );
					Map( 0xA000U, 0xAFFFU, &Ks202::Poke_A000 );
					Map( 0xB000U, 0xBFFFU, &Ks202::Poke_B000 );
					Map( 0xC000U, 0xCFFFU, &Ks202::Poke_C000 );
					Map( 0xD000U, 0xDFFFU, &Ks202::Poke_D000 );
					Map( 0xE000U, 0xEFFFU, &Ks202::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &Ks202::Poke_F000 );

					if (hard)
						ctrl = 0;

					irq.Reset( hard, hard ? false : irq.Connected() );
				}

				void Ks7010::SubReset(const bool hard)
				{
					prg.SwapBank<SIZE_16K>( 0x0000, 0x5 );
					prg.SwapBank<SIZE_16K>( 0x4000, 0x3 );

					// At the time of writing, the true mask for bankswitching is unknown
					Map( 0x6000U, 0x7FFFU, &Ks7010::Peek_6000 );
					Map( 0xCAB6U, 0xCAD6U, &Ks7010::Peek_FFFC );
					Map( 0xEBE2U, 0xEBE3U, &Ks7010::Peek_FFFC );
					Map( 0xEE32U, &Ks7010::Peek_FFFC );
					Map( 0xFFFCU, &Ks7010::Peek_FFFC );

					reg = 0;
				}

				void Ks7013b::SubReset(const bool hard)
				{
					prg.SwapBank<SIZE_16K>( 0x4000, 0x7 );

					Map( 0x6000U, 0x7FFFU, &Ks7013b::Poke_6000 );
					Map( 0x8000U, 0xFFFFU, &Ks7013b::Poke_8000 );
				}

				void Ks7016::SubReset(const bool hard)
				{
					reg = 8;

					prg.SwapBank<SIZE_32K>( 0x0000, 0x3 );

					Map( 0x6000U, 0x7FFFU, &Ks7016::Peek_6000 );
					Map( 0x8000U, 0xFFFFU, &Ks7016::Poke_8000 );
				}

				void Ks7022::SubReset(const bool hard)
				{
					reg = 0;

					if (hard)
						prg.SwapBanks<SIZE_16K,0x0000>( 0, 0 );

					Map( 0x8000, &Ks7022::Poke_8000 );
					Map( 0xA000, &Ks7022::Poke_A000 );
					Map( 0xFFFC, &Ks7022::Peek_FFFC );
				}

				void Ks7031::SubReset(const bool hard)
				{
					Map( 0x6000U, 0xFFFEU, &Ks7031::Peek_6000 );
					Map( 0x8000U, 0xFFFFU, &Ks7031::Poke_8000 );

					regs[0] = 0;
					regs[1] = 0;
					regs[2] = 0;
					regs[3] = 0;
				}

				void Ks7032::SubReset(const bool hard)
				{
					Ks202::SubReset( hard );
					Map( 0x6000U, 0x7FFFU, &Ks7032::Peek_6000 );
				}

				void Ks7037::SubReset(const bool hard)
				{
					if (hard)
					{
						regNum = 0;

						for (uint i = 0; i < 8; ++i)
							regs[i] = 0;
					}

					Map( 0x6000U, 0x6FFFU, &Ks7037::Peek_6000 );
					Map( 0x6000U, 0x6FFFU, &Ks7037::Poke_6000 );

					Map( 0x7000U, 0x7FFFU, &Ks7037::Peek_7000 );
					Map( 0x8000U, 0x9FFFU, &Ks7037::Peek_8000 );

					for (uint i = 0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0x8000 + i, &Ks7037::Poke_8000 );
						Map( 0x8001 + i, &Ks7037::Poke_8001 );
					}

					Map( 0xA000U, 0xAFFFU, &Ks7037::Peek_A000 );

					Map( 0xB000U, 0xBFFFU, &Ks7037::Peek_B000 );
					Map( 0xB000U, 0xBFFFU, &Ks7037::Poke_B000 );

					Map( 0xC000U, 0xDFFFU, &Ks7037::Peek_C000 );
					Map( 0xE000U, 0xEFFFU, &Ks7037::Peek_E000 );
				}

				void Ks7057::SubReset(const bool hard)
				{
					prg.SwapBank<SIZE_8K>( 0x2000, 0xD );
					prg.SwapBank<SIZE_16K>( 0x4000, 0x7 );

					Map( 0x6000U, 0x9FFFU, &Ks7057::Peek_6000 );
					Map( 0x8000U, 0x9FFFU, &Ks7057::Poke_8000 );
					Map( 0xB000U, 0xE003U, &Ks7057::Poke_B000 );

					if (hard)
					{
						for (uint i = 0; i < 8; ++i)
							regs[i] = 0;
					}
				}

				void Ks7058::SubReset(bool)
				{
					for (uint i=0x000; i < 0x1000; i += 0x100)
					{
						Map( 0xF000+i, 0xF07F+i, CHR_SWAP_4K_0 );
						Map( 0xF080+i, 0xF0FF+i, CHR_SWAP_4K_1 );
					}
				}

				void Ks202::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','0','2'>::V) );

					if (baseChunk == AsciiId<'K','0','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									ctrl = state.Read8();
									break;

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<5> data( state );

									irq.unit.ctrl = data[0];
									irq.unit.count = data[1] | data[2] << 8;
									irq.unit.latch = data[3] | data[4] << 8;
									irq.Connect( data[0] & 0xF );

									break;
								}
							}

							state.End();
						}
					}
				}

				void Ks7010::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','7','0'>::V) );

					if (baseChunk == AsciiId<'K','7','0'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8();

							state.End();
						}
					}
				}

				void Ks7016::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','7','6'>::V) );

					if (baseChunk == AsciiId<'K','7','6'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8();

							state.End();
						}
					}
				}

				void Ks7022::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','7','2'>::V) );

					if (baseChunk == AsciiId<'K','7','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8();

							state.End();
						}
					}
				}

				void Ks7031::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','7','1'>::V) );

					if (baseChunk == AsciiId<'K','7','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<4> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
								regs[2] = data[2];
								regs[3] = data[3];
							}

							state.End();
						}
					}
				}

				void Ks7037::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','7','7'>::V) );

					if (baseChunk == AsciiId<'K','7','7'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<9> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
								regs[2] = data[2];
								regs[3] = data[3];
								regs[4] = data[4];
								regs[5] = data[5];
								regs[6] = data[6];
								regs[7] = data[7];
								regNum = data[8];
							}

							state.End();
						}
					}
				}

				void Ks7057::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','5','7'>::V) );

					if (baseChunk == AsciiId<'K','5','7'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<8> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
								regs[2] = data[2];
								regs[3] = data[3];
								regs[4] = data[4];
								regs[5] = data[5];
								regs[6] = data[6];
								regs[7] = data[7];
							}

							state.End();
						}
					}
				}

				void Ks202::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','0','2'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).End();

					const byte data[5] =
					{
						irq.unit.ctrl,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8,
						irq.unit.latch & 0xFF,
						irq.unit.latch >> 8
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					state.End();
				}

				void Ks7010::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','7','0'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				void Ks7016::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','7','6'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				void Ks7022::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','7','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				void Ks7031::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','7','1'>::V );

					state.Begin( AsciiId<'R','E','G'>::V ).Write( regs ).End();
					state.End();
				}

				void Ks7037::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','7','7'>::V );

					const byte data[9] =
					{
						regs[0], regs[1], regs[2], regs[3],
						regs[4], regs[5], regs[6], regs[7],
						regNum
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					state.End();
				}

				void Ks7057::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','5','7'>::V );

					const byte data[8] =
					{
						regs[0], regs[1], regs[2], regs[3],
						regs[4], regs[5], regs[6], regs[7]
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Ks202,8000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFFF0) | (data & 0xF) << 0;
				}

				NES_POKE_D(Ks202,9000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFF0F) | (data & 0xF) << 4;
				}

				NES_POKE_D(Ks202,A000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xF0FF) | (data & 0xF) << 8;
				}

				NES_POKE_D(Ks202,B000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0x0FFF) | (data & 0xF) << 12;
				}

				NES_POKE_D(Ks202,C000)
				{
					irq.Update();

					irq.unit.ctrl = data;

					if (irq.Connect( data & 0xF ))
						irq.unit.count = irq.unit.latch;

					irq.ClearIRQ();
				}

				NES_POKE(Ks202,D000)
				{
					irq.Update();
					irq.ClearIRQ();
				}

				NES_POKE_D(Ks202,E000)
				{
					ctrl = data;
				}

				NES_POKE_AD(Ks202,F000)
				{
					{
						uint offset = (ctrl & 0xF) - 1;

						if (offset < 3)
						{
							offset <<= 13;
							prg.SwapBank<SIZE_8K>( offset, (data & 0x0F) | (prg.GetBank<SIZE_8K>(offset) & 0x10) );
						}
						else if (offset < 4)
						{
							wrk.SwapBank<SIZE_8K,0x0000>( data );
						}
					}

					switch (address & 0xC00)
					{
						case 0x000:

							address &= 0x3;

							if (address < 3)
							{
								address <<= 13;
								prg.SwapBank<SIZE_8K>( address, (prg.GetBank<SIZE_8K>(address) & 0x0F) | (data & 0x10) );
							}
							break;

						case 0x800:

							ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_V : Ppu::NMT_H );
							break;

						case 0xC00:

							ppu.Update();
							chr.SwapBank<SIZE_1K>( (address & 0x7) << 10, data );
							break;
					}
				}

				bool Ks202::Irq::Clock()
				{
					return (count++ == 0xFFFF) ? (count=latch, true) : false;
				}

				void Ks202::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}

				NES_PEEK_A(Ks7010,6000)
				{
					return *(prg.Source().Mem(reg * SIZE_8K) + (address & 0x1FFF));
				}

				NES_PEEK_A(Ks7010,FFFC)
				{
					reg = (address >> 2) & 0xF;
					chr.SwapBank<SIZE_8K,0x0000>( reg );
					ppu.Update();

					return prg.Peek(address & 0x7FFF);
				}

				NES_POKE_D(Ks7013b,6000)
				{
					prg.SwapBank<SIZE_16K>( 0x0000, data & 0x7 );
				}

				NES_POKE_D(Ks7013b,8000)
				{
					ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_PEEK_A(Ks7016,6000)
				{
					return *(prg.Source().Mem(reg * SIZE_8K) + (address & 0x1FFF));
				}

				NES_POKE_A(Ks7016,8000)
				{
					bool mode = (address & 0x30) == 0x30;

					switch(address & 0xD943) {
						case 0xD943:
							reg = mode ? 0xB : (address >> 2) & 0xF;
							break;

						case 0xD903:
							reg = mode ? 0x8 | ((address >> 2) & 0x3) : reg = 0xB;
							break;
					}
				}

				NES_POKE_D(Ks7022,8000)
				{
					ppu.SetMirroring( (data & 0x4) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(Ks7022,A000)
				{
					reg = data & 0xF;
				}

				NES_PEEK(Ks7022,FFFC)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( reg );
					prg.SwapBanks<SIZE_16K,0x0000>( reg, reg );

					return prg.Peek(0x7FFC);
				}

				NES_POKE_AD(Ks7031,8000)
				{
					regs[(address >> 11) & 0x03] = data;
				}

				NES_PEEK_A(Ks7031,6000)
				{
					int bank, new_addr;

					if (address < 0x8000)
						bank = regs[(address >> 11) & 0x03];
					else
						bank = 0x0f - ((address >> 11) & 0x0f);

					new_addr = ((bank << 11) % prg.Source(0).Size()) | (address & 0x07ff);

					return prg[0][new_addr];
				}

				NES_PEEK_A(Ks7032,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_PEEK_A(Ks7037,6000)
				{
					NST_VERIFY( wrk.Readable(0) );
					return wrk.Readable(0) ? wrk[0][address - 0x6000] : (address >> 8);
				}

				NES_POKE_AD(Ks7037,6000)
				{
					NST_VERIFY( wrk.Writable(0) );

					if (wrk.Writable(0))
						wrk[0][address- 0x6000] = data;
				}

				NES_PEEK_A(Ks7037,7000)
				{
					return *(prg.Source().Mem(SIZE_4K * 15) + (address & 0xFFF));
				}

				NES_PEEK_A(Ks7037,8000)
				{
					return *(prg.Source().Mem(regs[6] * SIZE_8K) + (address & 0x1FFF));
				}

				NES_POKE_D(Ks7037,8000)
				{
					regNum = data & 0x7U;
					byte mirror[4] = { regs[2], regs[4], regs[3], regs[5] };
					ppu.SetMirroring(mirror);
				}

				NES_POKE_D(Ks7037,8001)
				{
					regs[regNum] = data;
				}

				NES_PEEK_A(Ks7037,A000)
				{
					return *(prg.Source().Mem(SIZE_4K * 28) + (address & 0xFFF));
				}

				NES_PEEK_A(Ks7037,B000)
				{
					NST_VERIFY( wrk.Readable(0) );
					return wrk.Readable(0) ? wrk[0][address - 0xA000] : (address >> 8);
				}

				NES_POKE_AD(Ks7037,B000)
				{
					NST_VERIFY( wrk.Writable(0) );

					if (wrk.Writable(0))
						wrk[0][address- 0xA000] = data;
				}

				NES_PEEK_A(Ks7037,C000)
				{
					return *(prg.Source().Mem(regs[7] * SIZE_8K) + (address & 0x1FFF));
				}

				NES_PEEK_A(Ks7037,E000)
				{
					return *(prg.Source().Mem(SIZE_8K * 15) + (address & 0x1FFF));
				}

				NES_PEEK_A(Ks7057,6000)
				{
					return *(prg.Source().Mem(regs[(address >> 11) - 0xC] * SIZE_2K) + (address & 0x7FF));
				}

				NES_POKE_D(Ks7057,8000)
				{
					ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_V : Ppu::NMT_H );
				}

				NES_POKE_AD(Ks7057,B000)
				{
					switch(address & 0xF003)
					{
						case 0xB000: regs[4] = (regs[4] & 0xF0) | (data & 0xF); break;
						case 0xB001: regs[4] = (regs[4] & 0xF) | (data << 4); break;
						case 0xB002: regs[5] = (regs[5] & 0xF0) | (data & 0xF); break;
						case 0xB003: regs[5] = (regs[5] & 0xF) | (data << 4); break;
						case 0xC000: regs[6] = (regs[6] & 0xF0) | (data & 0xF); break;
						case 0xC001: regs[6] = (regs[6] & 0xF) | (data << 4); break;
						case 0xC002: regs[7] = (regs[7] & 0xF0) | (data & 0xF); break;
						case 0xC003: regs[7] = (regs[7] & 0xF) | (data << 4); break;
						case 0xD000: regs[0] = (regs[0] & 0xF0) | (data & 0xF); break;
						case 0xD001: regs[0] = (regs[0] & 0xF) | (data << 4); break;
						case 0xD002: regs[1] = (regs[1] & 0xF0) | (data & 0xF); break;
						case 0xD003: regs[1] = (regs[1] & 0xF) | (data << 4); break;
						case 0xE000: regs[2] = (regs[2] & 0xF0) | (data & 0xF); break;
						case 0xE001: regs[2] = (regs[2] & 0xF) | (data << 4); break;
						case 0xE002: regs[3] = (regs[3] & 0xF0) | (data & 0xF); break;
						case 0xE003: regs[3] = (regs[3] & 0xF) | (data << 4); break;
					}
				}
			}
		}
	}
}
