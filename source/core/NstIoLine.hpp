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

#ifndef NST_IO_LINE_H
#define NST_IO_LINE_H

namespace Nes
{
	namespace Core
	{
		namespace Io
		{

			class Line : public ImplicitBool<Line>
			{
				typedef void* Component;
				typedef void (NST_REGCALL *Toggler)(Component,Address,Cycle);

				Component component;
				Toggler toggler;

			public:

				Line() {}

				Line(Component c,Toggler t)
				:
				component ( c ),
				toggler   ( t )
				{}

				void Set(Component c,Toggler t)
				{
					component = c;
					toggler   = t;
				}

				void Unset()
				{
					component = NULL;
					toggler = NULL;
				}

				bool operator ! () const
				{
					return component == NULL;
				}

				void Toggle(Address address,Cycle cycle) const
				{
					toggler( component, address, cycle );
				}
			};

			#define NES_DECL_LINE(a_)                                                              \
                                                                                                   \
				NST_FORCE_INLINE void NST_FASTCALL Line_M_##a_(Address,Cycle);                     \
				static NST_NO_INLINE void NST_REGCALL Line_##a_(void*,Address,Cycle)

			#define NES_LINE(o_,a_)                                                                \
                                                                                                   \
				NST_NO_INLINE void NST_REGCALL o_::Line_##a_(void* p_,Address a_,Cycle c_)         \
				{                                                                                  \
					static_cast<o_*>(p_)->Line_M_##a_(a_,c_);                                      \
				}                                                                                  \
                                                                                                   \
				NST_FORCE_INLINE void NST_FASTCALL o_::Line_M_##a_(Address address,Cycle cycle)

			#define NES_LINE_T(t_,o_,a_)                                                           \
                                                                                                   \
				t_ NST_NO_INLINE void NST_REGCALL o_::Line_##a_(void* p_,Address a_,Cycle c_)      \
				{                                                                                  \
					static_cast<o_*>(p_)->Line_M_##a_(a_,c_);                                      \
				}                                                                                  \
                                                                                                   \
				t_ NST_FORCE_INLINE void NST_FASTCALL o_::Line_M_##a_(Address address,Cycle cycle)

		}
	}
}

#endif
