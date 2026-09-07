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

#ifndef NST_SOUND_RENDERER_H
#define NST_SOUND_RENDERER_H

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			typedef idword Sample;

			class Buffer
			{
			public:

				explicit Buffer(uint);
				~Buffer();

				enum
				{
					SIZE = 0x4000,
					MASK = SIZE-1
				};

				struct Block
				{
					inline explicit Block(uint);

					const iword* data;
					uint start;
					uint length;
				};

				void Reset(bool=true);
				void operator >> (Block&);

				class Renderer;

			private:

				uint pos;
				uint start;
				iword* const NST_RESTRICT output;

			public:

				inline void operator << (const Sample);
			};

			class Buffer::Renderer : public ImplicitBool<Buffer::Renderer>
			{
				iword* NST_RESTRICT dst;
				const iword* const end;

			public:

				inline Renderer(void*,uint);

				inline bool operator !() const;
				inline void operator << (Sample);
				NST_FORCE_INLINE bool operator << (const Block&);
			};
		}
	}
}

#endif
