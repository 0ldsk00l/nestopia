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

#ifndef NST_IO_WAVE_H
#define NST_IO_WAVE_H

#pragma once

#include "language/resource.h"
#include "NstString.hpp"
#include <windows.h>

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

namespace Nestopia
{
	namespace Io
	{
		class Wave
		{
		public:

			enum Mode
			{
				MODE_READ = MMIO_ALLOCBUF|MMIO_READ,
				MODE_WRITE = MMIO_ALLOCBUF|MMIO_READWRITE|MMIO_CREATE|MMIO_EXCLUSIVE
			};

			explicit Wave(Mode);
			~Wave();

			enum Exception
			{
				ERR_OPEN = IDS_WAVE_ERR_OPEN,
				ERR_WRITE = IDS_WAVE_ERR_WRITE,
				ERR_READ = IDS_WAVE_ERR_READ,
				ERR_FINALIZE = IDS_WAVE_ERR_FINALIZE
			};

			uint Open(const GenericString,WAVEFORMATEX&);
			uint Open(const void*,uint,WAVEFORMATEX&);
			void Write(const void*,uint);
			void Read(void*);
			void Close();

		private:

			uint Open(WAVEFORMATEX&);
			void Abort();

			void CreateChunk(MMCKINFO&,FOURCC,uint,uint,uint) const;
			void AscendChunk(MMCKINFO&) const;
			void DescendChunk(MMCKINFO&,const MMCKINFO*,uint) const;

			template<typename T> void WriteChunk(const T&,int) const;
			template<typename T> void ReadChunk(T&,int) const;

			HMMIO handle;
			const Mode mode;
			MMCKINFO chunkRiff;
			MMCKINFO chunkData;
			MMIOINFO info;
			Path fileName;

		public:

			bool IsOpen() const
			{
				return handle;
			}

			const Path& GetName() const
			{
				return fileName;
			}
		};
	}
}

#endif
