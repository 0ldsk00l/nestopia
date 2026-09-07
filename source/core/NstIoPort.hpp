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

#ifndef NST_IO_PORT_H
#define NST_IO_PORT_H

namespace Nes
{
	namespace Core
	{
		namespace Io
		{

			class Port
			{
				typedef void* Component;
				typedef Data (NST_REGCALL *Reader)(Component,Address);
				typedef void (NST_REGCALL *Writer)(Component,Address,Data);

				Component component;
				Reader reader;
				Writer writer;

			public:

				Port() {}

				Port(Component c,Reader r,Writer w)
				:
				component ( c ),
				reader    ( r ),
				writer    ( w )
				{}

				void Set(Component c,Reader r,Writer w)
				{
					component = c;
					reader    = r;
					writer    = w;
				}

				void Set(Reader r)
				{
					reader = r;
				}

				void Set(Writer w)
				{
					writer = w;
				}

				void Set(Reader r,Writer w)
				{
					reader = r;
					writer = w;
				}

				Data Peek(Address address) const
				{
					return reader( component, address );
				}

				void Poke(Address address,Data data) const
				{
					writer( component, address, data );
				}

				bool operator == (const Port& p) const
				{
					return component == p.component && reader == p.reader && writer == p.writer;
				}
			};

			#define NES_DECL_PEEK(a_)                                                        \
                                                                                             \
				NST_SINGLE_CALL Data NST_FASTCALL Peek_M_##a_(Address);                      \
				static Data NST_REGCALL Peek_##a_(void*,Address)

			#define NES_DECL_POKE(a_)                                                        \
                                                                                             \
				NST_SINGLE_CALL void NST_FASTCALL Poke_M_##a_(Address,Data);                 \
				static void NST_REGCALL Poke_##a_(void*,Address,Data)

			#define NES_PEEK(o_,a_)                                                          \
                                                                                             \
				Data NST_REGCALL o_::Peek_##a_(void* p_,Address i_)                          \
				{                                                                            \
					return static_cast<o_*>(p_)->Peek_M_##a_(i_);                            \
				}                                                                            \
                                                                                             \
				NST_SINGLE_CALL Data NST_FASTCALL o_::Peek_M_##a_(Address)

			#define NES_PEEK_A(o_,a_)                                                        \
                                                                                             \
				Data NST_REGCALL o_::Peek_##a_(void* p_,Address i_)                          \
				{                                                                            \
					return static_cast<o_*>(p_)->Peek_M_##a_(i_);                            \
				}                                                                            \
                                                                                             \
				NST_SINGLE_CALL Data NST_FASTCALL o_::Peek_M_##a_(Address address)

			#define NES_POKE(o_,a_)                                                          \
                                                                                             \
				void NST_REGCALL o_::Poke_##a_(void* p_,Address i_,Data j_)                  \
				{                                                                            \
					static_cast<o_*>(p_)->Poke_M_##a_(i_,j_);                                \
				}                                                                            \
                                                                                             \
				NST_SINGLE_CALL void NST_FASTCALL o_::Poke_M_##a_(Address,Data)

			#define NES_POKE_A(o_,a_)                                                        \
                                                                                             \
				void NST_REGCALL o_::Poke_##a_(void* p_,Address i_,Data j_)                  \
				{                                                                            \
					static_cast<o_*>(p_)->Poke_M_##a_(i_,j_);                                \
				}                                                                            \
                                                                                             \
				NST_SINGLE_CALL void NST_FASTCALL o_::Poke_M_##a_(Address address,Data)

			#define NES_POKE_D(o_,a_)                                                        \
                                                                                             \
				void NST_REGCALL o_::Poke_##a_(void* p_,Address i_,Data j_)                  \
				{                                                                            \
					static_cast<o_*>(p_)->Poke_M_##a_(i_,j_);                                \
				}                                                                            \
                                                                                             \
				NST_SINGLE_CALL void NST_FASTCALL o_::Poke_M_##a_(Address,Data data)

			#define NES_POKE_AD(o_,a_)                                                       \
                                                                                             \
				void NST_REGCALL o_::Poke_##a_(void* p_,Address i_,Data j_)                  \
				{                                                                            \
					static_cast<o_*>(p_)->Poke_M_##a_(i_,j_);                                \
				}                                                                            \
                                                                                             \
				NST_SINGLE_CALL void NST_FASTCALL o_::Poke_M_##a_(Address address,Data data)

			#define NES_DO_POKE(a_,p_,d_) Poke_##a_(this,p_,d_)
			#define NES_DO_PEEK(a_,p_)    Peek_##a_(this,p_)

		}
	}
}

#endif
