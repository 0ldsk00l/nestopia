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

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			void Buffer::operator >> (Block& block)
			{
				NST_ASSERT( block.length );

				const uint delta = (dword(pos) + SIZE - start) & MASK;

				block.data = output;
				block.start = start;

				if (block.length > delta)
					block.length = delta;

				start = (start + block.length) & MASK;

				if (start == pos)
				{
					start = 0;
					pos = 0;
				}
			}

			inline Buffer::Block::Block(uint l)
			: length(l) {}

			inline void Buffer::operator << (const Sample sample)
			{
				const uint p = pos;
				pos = (pos + 1) & MASK;
				output[p] = sample;
			}

			inline Buffer::Renderer::Renderer(void* samples,uint length)
			:
			dst (static_cast<iword*>(samples)),
			end (static_cast<const iword*>(samples) + length)
			{}

			inline bool Buffer::Renderer::operator !() const
			{
				return dst == end;
			}

			inline void Buffer::Renderer::operator << (Sample sample)
			{
				*dst++ = sample;
			}

			NST_FORCE_INLINE bool Buffer::Renderer::operator << (const Block& block)
			{
				NST_ASSERT( end - dst >= block.length );

				if (block.length)
				{
					if (block.start + block.length <= SIZE)
					{
						std::memcpy( dst, block.data + block.start, sizeof(iword) * block.length );
					}
					else
					{
						const uint chunk = SIZE - block.start;
						std::memcpy( dst, block.data + block.start, sizeof(iword) * chunk );
						std::memcpy( dst + chunk, block.data, sizeof(iword) * ((block.start + block.length) - SIZE) );
					}

					dst += block.length;
				}

				return dst != end;
			}

		}
	}
}
