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

#include "NstObjectPod.hpp"
#include "NstIoWave.hpp"

namespace Nestopia
{
	namespace Io
	{
		Wave::Wave(Mode m)
		: handle(NULL), mode(m) {}

		Wave::~Wave()
		{
			try
			{
				Close();
			}
			catch (Exception)
			{
			}
		}

		void Wave::CreateChunk(MMCKINFO& chunk,FOURCC id,uint size=0,uint type=0,uint flags=0) const
		{
			chunk.ckid = id;
			chunk.cksize = size;
			chunk.dwDataOffset = 0;
			chunk.dwFlags = 0;
			chunk.fccType = type;

			if (::mmioCreateChunk( handle, &chunk, flags ) != MMSYSERR_NOERROR)
				throw ERR_OPEN;
		}

		template<typename T>
		void Wave::WriteChunk(const T& t,const int size=sizeof(T)) const
		{
			if (::mmioWrite( handle, reinterpret_cast<const char*>(&t), size ) != size)
				throw ERR_OPEN;
		}

		template<typename T>
		void Wave::ReadChunk(T& t,const int size=sizeof(T)) const
		{
			if (::mmioRead( handle, reinterpret_cast<char*>(&t), size ) != size)
				throw ERR_OPEN;
		}

		void Wave::AscendChunk(MMCKINFO& chunk) const
		{
			if (::mmioAscend( handle, &chunk, 0 ) != MMSYSERR_NOERROR)
				throw ERR_OPEN;
		}

		void Wave::DescendChunk(MMCKINFO& chunk,const MMCKINFO* parent=NULL,uint flag=0) const
		{
			if (::mmioDescend( handle, &chunk, parent, flag ) != MMSYSERR_NOERROR)
				throw ERR_OPEN;
		}

		uint Wave::Open(const void* data,uint size,WAVEFORMATEX& waveFormat)
		{
			Close();
			fileName.Clear();

			Object::Pod<MMIOINFO> info;
			info.fccIOProc = FOURCC_MEM;
			info.pchBuffer = static_cast<HPSTR>(const_cast<void*>(data));
			info.cchBuffer = size;

			if (NULL == (handle=::mmioOpen( NULL, &info, mode )))
				throw ERR_OPEN;

			return Open( waveFormat );
		}

		uint Wave::Open(const GenericString name,WAVEFORMATEX& waveFormat)
		{
			Close();
			fileName = name;

			if (NULL == (handle=::mmioOpen( fileName.Ptr(), NULL, mode )))
				throw ERR_OPEN;

			return Open( waveFormat );
		}

		uint Wave::Open(WAVEFORMATEX& waveFormat)
		{
			NST_COMPILE_ASSERT( sizeof(WAVEFORMATEX) >= sizeof(PCMWAVEFORMAT) );

			try
			{
				if (mode == MODE_WRITE)
				{
					NST_ASSERT( waveFormat.wFormatTag == WAVE_FORMAT_PCM );

					CreateChunk( chunkRiff, 0, 0, mmioFOURCC('W','A','V','E'), MMIO_CREATERIFF );

					MMCKINFO chunk;

					CreateChunk( chunk, mmioFOURCC('f','m','t',' '), sizeof(PCMWAVEFORMAT) );
					WriteChunk( waveFormat, sizeof(PCMWAVEFORMAT) );
					AscendChunk( chunk );

					CreateChunk( chunk, mmioFOURCC('f','a','c','t') );
					WriteChunk( DWORD(-1) );
					AscendChunk( chunk );

					CreateChunk( chunkData, mmioFOURCC('d','a','t','a') );
				}
				else
				{
					DescendChunk( chunkRiff );

					if (chunkRiff.ckid != FOURCC_RIFF || chunkRiff.fccType != mmioFOURCC('W','A','V','E'))
						throw ERR_OPEN;

					MMCKINFO chunk;

					chunk.ckid = mmioFOURCC('f','m','t',' ');
					DescendChunk( chunk, &chunkRiff, MMIO_FINDCHUNK );

					if (chunk.cksize != sizeof(PCMWAVEFORMAT))
						throw ERR_OPEN;

					PCMWAVEFORMAT pcm;

					ReadChunk( pcm );

					if
					(
						pcm.wf.wFormatTag != WAVE_FORMAT_PCM ||
						pcm.wBitsPerSample == 0 ||
						pcm.wBitsPerSample % 8 ||
						pcm.wf.nSamplesPerSec == 0 ||
						pcm.wf.nChannels < 1 || pcm.wf.nChannels > 2 ||
						pcm.wf.nBlockAlign != pcm.wBitsPerSample / 8 * pcm.wf.nChannels ||
						pcm.wf.nAvgBytesPerSec != pcm.wf.nSamplesPerSec * pcm.wf.nBlockAlign
					)
						throw ERR_OPEN;

					std::memcpy( &waveFormat, &pcm, sizeof(PCMWAVEFORMAT) );

					AscendChunk( chunk );

					if (::mmioSeek( handle, chunkRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET ) == -1)
						throw ERR_OPEN;

					chunkData.ckid = mmioFOURCC('d','a','t','a');
					DescendChunk( chunkData, &chunkRiff, MMIO_FINDCHUNK );
				}

				if (::mmioGetInfo( handle, &info, 0 ) != MMSYSERR_NOERROR)
					throw ERR_OPEN;

				return chunkData.cksize;
			}
			catch (Exception)
			{
				Abort();
				throw ERR_OPEN;
			}
		}

		void Wave::Write(const void* const data,const uint length)
		{
			NST_ASSERT( handle );

			const uchar* input = static_cast<const uchar*>(data);
			const uchar* const end = input + length;

			while (input != end)
			{
				if (info.pchNext == info.pchEndWrite)
				{
					info.dwFlags |= MMIO_DIRTY;

					if (::mmioAdvance( handle, &info, MMIO_WRITE ) != MMSYSERR_NOERROR)
					{
						Abort();
						throw ERR_WRITE;
					}
				}

				*info.pchNext++ = *input++;
			}
		}

		void Wave::Read(void* const data)
		{
			NST_ASSERT( handle && data );

			uchar* output = static_cast<uchar*>(data);
			const uchar* const end = output + chunkData.cksize;

			while (output != end)
			{
				if (info.pchNext == info.pchEndRead)
				{
					if (::mmioAdvance( handle, &info, MMIO_READ ) != MMSYSERR_NOERROR || info.pchNext == info.pchEndRead)
					{
						Abort();
						throw ERR_READ;
					}
				}

				*output++ = *info.pchNext++;
			}
		}

		void Wave::Close()
		{
			if (handle)
			{
				if (mode == MODE_WRITE)
				{
					try
					{
						info.dwFlags |= MMIO_DIRTY;

						if (::mmioSetInfo( handle, &info, 0 ) != MMSYSERR_NOERROR)
							throw ERR_WRITE;

						AscendChunk( chunkData );
						AscendChunk( chunkRiff );

						::mmioSeek( handle, 0, SEEK_SET );

						if (::mmioDescend( handle, &chunkRiff, NULL, 0 ) != MMSYSERR_NOERROR)
							throw ERR_WRITE;

						MMCKINFO chunkFact;

						chunkFact.ckid = mmioFOURCC('f','a','c','t');
						chunkFact.cksize = 0;

						if (::mmioDescend( handle, &chunkFact, &chunkRiff, MMIO_FINDCHUNK ) == MMSYSERR_NOERROR)
						{
							WriteChunk( DWORD(0) );
							AscendChunk( chunkFact );
						}

						AscendChunk( chunkRiff );
					}
					catch (Exception)
					{
						Abort();
						throw ERR_FINALIZE;
					}
				}

				::mmioClose( handle, 0 );
				handle = NULL;
			}
		}

		void Wave::Abort()
		{
			NST_ASSERT( handle && fileName.Length() );

			::mmioClose( handle, 0 );
			handle = NULL;
		}
	}
}
