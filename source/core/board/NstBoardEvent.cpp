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
#include "NstBoard.hpp"
#include "../NstTimer.hpp"
#include "NstBoardMmc1.hpp"
#include "NstBoardMmc3.hpp"
#include "NstBoardEvent.hpp"
#include "../api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Event::CartSwitches::CartSwitches()
			: time(DEFAULT_DIP), showTime(true) {}

			Event::Event(const Context& c)
			: Mmc1(c,REV_B2), irq(*c.cpu)
			{
				NST_COMPILE_ASSERT( TIME_TEXT_MIN_OFFSET == 11 && TIME_TEXT_SEC_OFFSET == 13 );
				std::strcpy( text, "Time left: x:xx" );
			}

			uint Event::CartSwitches::NumDips() const
			{
				return 2;
			}

			uint Event::CartSwitches::NumValues(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? 16 : 2;
			}

			cstring Event::CartSwitches::GetDipName(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? "Time" : "Show Time";
			}

			cstring Event::CartSwitches::GetValueName(uint dip,uint value) const
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 16 );

					static const char times[16][7] =
					{
						"5:00.4",
						"5:19.2",
						"5:38.0",
						"5:56.7",
						"6:15.5",
						"6:34.3",
						"6:53.1",
						"7:11.9",
						"7:30.6",
						"7:49.4",
						"8:08.2",
						"8:27.0",
						"8:45.8",
						"9:04.5",
						"9:23.3",
						"9:42.1"
					};

					return times[value];
				}
				else
				{
					NST_ASSERT( value < 2 );

					return (value == 0) ? "no" : "yes";
				}
			}

			uint Event::CartSwitches::GetValue(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? time : showTime;
			}

			void Event::CartSwitches::SetValue(uint dip,uint value)
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 16 );
					time = value;
				}
				else
				{
					NST_ASSERT( value < 2 );
					showTime = value;
				}
			}

			inline dword Event::CartSwitches::GetTime() const
			{
				return BASE_TIME * (time + 16UL) - 1;
			}

			inline bool Event::CartSwitches::ShowTime() const
			{
				return showTime;
			}

			Event::Device Event::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return &cartSwitches;
				else
					return Board::QueryDevice( type );
			}

			void Event::SubReset(const bool hard)
			{
				irq.Reset( hard, true );
				time = 0;

				Mmc1::SubReset( hard );

				prg.SwapBank<SIZE_16K,0x4000>( 1 );
			}

			void Event::SubLoad(State::Loader& state,const dword baseChunk)
			{
				time = 0;

				if (baseChunk == AsciiId<'E','V','T'>::V)
				{
					irq.unit.count = 0;

					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'I','R','Q'>::V)
							irq.unit.count = state.Read32();

						state.End();
					}
				}
				else
				{
					Mmc1::SubLoad( state, baseChunk );
				}
			}

			void Event::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'E','V','T'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write32( irq.unit.count ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void NST_FASTCALL Event::UpdateRegisters(const uint index)
			{
				NST_ASSERT( index < 4 );

				if (index != 2)
				{
					if (regs[1] & 0x8U)
					{
						switch (regs[0] & 0xCU)
						{
							case 0x0:
							case 0x4:

								prg.SwapBank<SIZE_32K,0x0000>( 0x4 | (regs[3] >> 1 & 0x3U) );
								break;

							case 0x8:

								prg.SwapBanks<SIZE_16K,0x0000>( 0x8, 0x8 | (regs[3] & 0x7U) );
								break;

							case 0xC:

								prg.SwapBanks<SIZE_16K,0x0000>( 0x8 | (regs[3] & 0x7U), 0xF );
								break;
						}
					}
					else
					{
						prg.SwapBank<SIZE_32K,0x0000>( regs[1] >> 1 & 0x3U );
					}

					UpdateWrk();

					if (index == 0)
					{
						UpdateNmt();
					}
					else
					{
						irq.Update();

						if (regs[1] & 0x10U)
						{
							irq.unit.count = 0;
							irq.ClearIRQ();
						}
						else if (irq.unit.count == 0)
						{
							irq.unit.count = cartSwitches.GetTime();
						}
					}
				}
			}

			void Event::Irq::Reset(bool)
			{
				count = 0;
			}

			bool Event::Irq::Clock()
			{
				return count && --count == 0;
			}

			void Event::Sync(Board::Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
				{
					if (cartSwitches.ShowTime() && irq.unit.count)
					{
						const dword t = cpu.GetTime( irq.unit.count );

						if (time != t)
						{
							time = t;

							text[TIME_TEXT_MIN_OFFSET+0] = '0' + t / 60;
							text[TIME_TEXT_SEC_OFFSET+0] = '0' + t % 60 / 10;
							text[TIME_TEXT_SEC_OFFSET+1] = '0' + t % 60 % 10;

							Api::User::eventCallback( Api::User::EVENT_DISPLAY_TIMER, text );
						}
					}

					irq.VSync();
					Mmc1::Sync( event, controllers );
				}
			}

			// EVENT2 - Nintendo Campus Challenge 1991, repro cart from RetroUSB
			Event2::CartSwitches::CartSwitches()
			: time(DEFAULT_DIP), showTime(true) {}

			Event2::Event2(const Context& c)
			: Mmc3(c)
			{
				NST_COMPILE_ASSERT( TIME_TEXT_MIN_OFFSET == 11 && TIME_TEXT_SEC_OFFSET == 13 );
				std::strcpy( text, "Time left: x:xx" );
			}

			uint Event2::CartSwitches::NumDips() const
			{
				return 2;
			}

			uint Event2::CartSwitches::NumValues(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? 16 : 2;
			}

			cstring Event2::CartSwitches::GetDipName(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? "Time" : "Show Time";
			}

			cstring Event2::CartSwitches::GetValueName(uint dip,uint value) const
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 16 );

					static const char times[16][7] =
					{
						"5:00.4",
						"5:19.2",
						"5:38.0",
						"5:56.7",
						"6:15.5",
						"6:34.3",
						"6:53.1",
						"7:11.9",
						"7:30.6",
						"7:49.4",
						"8:08.2",
						"8:27.0",
						"8:45.8",
						"9:04.5",
						"9:23.3",
						"9:42.1"
					};

					return times[value];
				}
				else
				{
					NST_ASSERT( value < 2 );

					return (value == 0) ? "no" : "yes";
				}
			}

			uint Event2::CartSwitches::GetValue(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? time : showTime;
			}

			void Event2::CartSwitches::SetValue(uint dip,uint value)
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 16 );
					time = value;
				}
				else
				{
					NST_ASSERT( value < 2 );
					showTime = value;
				}
			}

			inline dword Event2::CartSwitches::GetTime() const
			{
				return BASE_TIME * (time + 16UL) - 1;
			}

			inline bool Event2::CartSwitches::ShowTime() const
			{
				return showTime;
			}

			Event::Device Event2::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return &cartSwitches;
				else
					return Board::QueryDevice( type );
			}

			void Event2::SubReset(const bool hard)
			{
				time = 0;
				ev2_reg[0] = ev2_reg[1] = 0;
				ev2_countdown = cartSwitches.GetTime();
				ev2_expired = false;

				Mmc3::SubReset(hard);

				Map( 0x5000U, 0x5FFFU, &Event2::Peek_5000, &Event2::Poke_5000 );
			}

			void Event2::SubLoad(State::Loader& state,const dword baseChunk)
			{
				time = 0;

				if (baseChunk == AsciiId<'E','V','2'>::V)
				{
					ev2_reg[0] = ev2_reg[1] = 0;
					ev2_expired = false;
					ev2_countdown = 0;

					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<2> data( state );
							ev2_reg[0] = data[0];
							ev2_reg[1] = data[1];
						}
						else if (chunk == AsciiId<'E','X','P'>::V)
						{
							ev2_expired = state.Read8();
						}
						else if (chunk == AsciiId<'C','N','T'>::V)
						{
							ev2_countdown = state.Read32();
						}
						state.End();
					}
				}
				else
				{
					Mmc3::SubLoad( state, baseChunk );
				}
			}

			void Event2::SubSave(State::Saver& state) const
			{
				const byte data[2] = { ev2_reg[0], ev2_reg[1] };
				state.Begin( AsciiId<'E','V','2'>::V );
				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				state.Begin( AsciiId<'E','X','P'>::V ).Write8( ev2_expired ).End();
				state.Begin( AsciiId<'C','N','T'>::V ).Write32( ev2_countdown ).End();
				state.End();
				Mmc3::SubSave(state);
			}

			NES_PEEK_A(Event2,5000)
			{
				if (address & 0x800)
				{
					return 0x5C | (ev2_expired ? 0x80 : 0);
				}

				return wrk.Source()[0x2000 | (address & 0x7FF)];
			}

			NES_POKE_AD(Event2,5000)
			{
				if (address & 0x800)
				{
					int rnum = (address >> 10) & 0x01;
					if (!rnum && !(data & 0x08))
					{
						time = 0;
						ev2_countdown = cartSwitches.GetTime();
						ev2_expired = false;
					}
					ev2_reg[rnum] = data;
					Mmc3::UpdatePrg();
					Mmc3::UpdateChr();
				}
				else
				{
					wrk.Source()[0x2000 | (address & 0x7FF)] = data;
				}
			}

			void NST_FASTCALL Event2::UpdatePrg(uint address,uint bank)
			{
				uint mask = ((ev2_reg[0] << 3) & 0x18) | 0x07;
				uint base = ((ev2_reg[0] << 3) & 0x20);
				prg.SwapBank<SIZE_8K>( address, base | (bank & mask) );
			}

			void NST_FASTCALL Event2::UpdateChr(uint address,uint bank) const
			{
				uint base = (ev2_reg[0] << 5) & 0x80;

				if ((ev2_reg[0] & 0x06) == 0x02)
				{
					const byte mask = (bank & 0x40) ? 0x07 : 0xFF;
					chr.Source( bank >> 6 & 0x1 ).SwapBank<SIZE_1K>( address, base | (bank & mask) );
				}
				else
				{
					chr.SwapBank<SIZE_1K>( address, base | (bank & 0x7F) );
				}
			}

			void Event2::Sync(Board::Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
				{
					if (ev2_reg[0] & 0x08)
					{
						uint cycs = cpu.GetFrameCycles() / cpu.GetClock();
						if (ev2_countdown >= cycs)
						{
							ev2_countdown -= cycs;
						}
						else
						{
							ev2_countdown = 0;
							ev2_expired = true;
						}
					}

					if (cartSwitches.ShowTime())
					{
						const dword t = cpu.GetTime(ev2_countdown);

						if (time != t)
						{
							time = t;

							text[TIME_TEXT_MIN_OFFSET+0] = '0' + t / 60;
							text[TIME_TEXT_SEC_OFFSET+0] = '0' + t % 60 / 10;
							text[TIME_TEXT_SEC_OFFSET+1] = '0' + t % 60 % 10;

							Api::User::eventCallback( Api::User::EVENT_DISPLAY_TIMER, text );
						}
					}
				}

				Mmc3::Sync( event, controllers );
			}

		}
	}
}
