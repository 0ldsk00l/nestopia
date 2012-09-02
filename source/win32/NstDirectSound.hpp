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

#ifndef NST_DIRECTX_DIRECTSOUND_H
#define NST_DIRECTX_DIRECTSOUND_H

#pragma once

#include <vector>
#include "NstObjectPod.hpp"
#include "NstDirectX.hpp"

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

#include <dsound.h>

namespace Nestopia
{
	namespace DirectX
	{
		class DirectSound
		{
		public:

			explicit DirectSound(HWND);
			~DirectSound();

			enum Channels
			{
				MONO = 1,
				STEREO
			};

			enum Pool
			{
				POOL_HARDWARE,
				POOL_SYSTEM
			};

			struct Adapter : BaseAdapter
			{
				Adapter();

				bool buggy;
			};

			typedef std::vector<Adapter> Adapters;

			cstring Update(uint,uint,uint,Channels,uint,Pool,bool);
			void Destroy();
			void StopStream();

		private:

			class SoundAdapters
			{
				static BOOL CALLBACK Enumerator(LPGUID,LPCTSTR,LPCTSTR,LPVOID);

			public:

				SoundAdapters();

				Adapters list;
			};

			struct Device : ComInterface<IDirectSound8>
			{
				explicit Device(HWND);

				HWND const hWnd;
				ushort id;
				bool priority;
				bool buggy;
			};

			class Buffer
			{
			public:

				Buffer();
				~Buffer();

				cstring Update(IDirectSound8&,bool,uint,uint,Channels,uint,Pool,bool);
				void StartStream();
				void StopStream(IDirectSound8*,bool);
				void Release();

			private:

				cstring Create(IDirectSound8&,bool);

				enum
				{
					DC_OFFSET_8 = 0x80,
					DC_OFFSET_16 = 0x0000
				};

				struct Com : ComInterface<IDirectSoundBuffer8>
				{
					Com();

					uint writePos;
				};

				struct Settings
				{
					Settings();

					uint size;
					Pool pool;
					bool globalFocus;
				};

				Com com;
				Object::Pod<WAVEFORMATEX> waveFormat;
				Settings settings;

			public:

				const WAVEFORMATEX& GetWaveFormat() const
				{
					return waveFormat;
				}

				bool GlobalFocus() const
				{
					return settings.globalFocus;
				}

				NST_FORCE_INLINE bool LockStream(void** data,uint* size)
				{
					DWORD pos;

					if (SUCCEEDED(com->GetCurrentPosition( &pos, NULL )))
					{
						pos = (pos > com.writePos ? pos - com.writePos : pos + settings.size - com.writePos);

						DWORD bytes[2];

						if (SUCCEEDED(com->Lock( com.writePos, pos, data+0, bytes+0, data+1, bytes+1, 0 )))
						{
							com.writePos = (com.writePos + pos) % settings.size;

							size[0] = bytes[0] / waveFormat.nBlockAlign;
							size[1] = bytes[1] / waveFormat.nBlockAlign;

							return true;
						}
					}

					com->Stop();

					NST_DEBUG_MSG("DirectSound::Buffer::Lock() failed!");

					return false;
				}

				NST_FORCE_INLINE void UnlockStream(void** data,uint* size) const
				{
					NST_ASSERT( data && com );
					com->Unlock( data[0], size[0]*waveFormat.nBlockAlign, data[1], size[1]*waveFormat.nBlockAlign );
				}

				bool Streaming() const
				{
					DWORD status;

					return
					(
						com && SUCCEEDED(com->GetStatus( &status )) &&
						(status & (DSBSTATUS_BUFFERLOST|DSBSTATUS_PLAYING|DSBSTATUS_LOOPING)) == (DSBSTATUS_PLAYING|DSBSTATUS_LOOPING)
					);
				}
			};

			Device device;
			Buffer buffer;
			const SoundAdapters adapters;

		public:

			bool Streaming() const
			{
				return buffer.Streaming();
			}

			void StartStream()
			{
				buffer.StartStream();
			}

			NST_FORCE_INLINE bool LockStream(void** data,uint* size)
			{
				return buffer.LockStream( data, size );
			}

			NST_FORCE_INLINE void UnlockStream(void** data,uint* size) const
			{
				buffer.UnlockStream( data, size );
			}

			const Adapters& GetAdapters() const
			{
				return adapters.list;
			}

			const WAVEFORMATEX& GetWaveFormat() const
			{
				return buffer.GetWaveFormat();
			}

			bool GlobalFocus() const
			{
				return buffer.GlobalFocus();
			}
		};
	}
}

#endif
