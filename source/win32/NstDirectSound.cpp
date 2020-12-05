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

#include "NstIoLog.hpp"
#include "NstDirectSound.hpp"
#include <Shlwapi.h>

#if NST_MSVC
#pragma comment(lib,"dsound")
#endif

namespace Nestopia
{
	namespace DirectX
	{
		DirectSound::Device::Device(HWND h)
		: hWnd(h), id(0), priority(false), buggy(false) {}

		DirectSound::Adapter::Adapter()
		: buggy(false) {}

		DirectSound::SoundAdapters::SoundAdapters()
		{
			Io::Log() << "DirectSound: initializing..\r\n";

			if (FAILED(::DirectSoundEnumerate( Enumerator, &list )) || list.empty())
				Enumerator( NULL, L"Primary Sound Driver", NULL, &list );

			bool report = true;

			for (Adapters::iterator it(list.begin()), end(list.end()); it != end; ++it)
			{
				if (::StrStrI( it->name.Ptr(), L"E-DSP Wave" ))
				{
					it->buggy = true;

					if (report)
					{
						report = false;
						Io::Log() << "DirectSound: warning, possibly buggy drivers!! activating stupid-mode..\r\n";
					}
				}
			}
		}

		DirectSound::DirectSound(HWND const hWnd)
		: device( hWnd ) {}

		DirectSound::~DirectSound()
		{
			Destroy();
		}

		void DirectSound::Destroy()
		{
			buffer.Release();
			device.Release();
		}

		BOOL CALLBACK DirectSound::SoundAdapters::Enumerator(LPGUID const guid,LPCTSTR const desc,LPCTSTR,LPVOID const context)
		{
			Io::Log() << "DirectSound: enumerating device - name: "
                      << (desc && *desc ? desc : L"unknown")
                      << ", GUID: "
                      << (guid ? System::Guid( *guid ).GetString() : L"unspecified")
                      << "\r\n";

			ComInterface<IDirectSound8> device;

			if (SUCCEEDED(::DirectSoundCreate8( guid, &device, NULL )))
			{
				static_cast<Adapters*>(context)->resize( static_cast<Adapters*>(context)->size() + 1 );
				Adapter& adapter = static_cast<Adapters*>(context)->back();

				if (guid)
					adapter.guid = *guid;

				adapter.name = desc;
			}
			else
			{
				Io::Log() << "DirectSound: DirectSoundCreate8() failed on this device, continuing enumeration..\r\n";
			}

			return true;
		}

		cstring DirectSound::Update
		(
			const uint deviceId,
			const uint rate,
			const uint bits,
			const Channels channels,
			const uint latency,
			const Pool pool,
			const bool globalFocus
		)
		{
			NST_ASSERT( deviceId < adapters.list.size() );

			if (device.id != deviceId || !device)
			{
				device.id = deviceId;

				Destroy();

				if (FAILED(::DirectSoundCreate8( &adapters.list[device.id].guid, &device, NULL )))
					return "DirectSoundCreate8()";

				Io::Log() << "DirectSound: creating device #" << device.id << "\r\n";

				device.priority = SUCCEEDED(device->SetCooperativeLevel( device.hWnd, DSSCL_PRIORITY ));
				device.buggy = adapters.list[device.id].buggy;

				if (!device.priority)
				{
					Io::Log() << "DirectSound: warning, IDirectSound8::SetCooperativeLevel( DSSCL_PRIORITY ) failed! Retrying with DSSCL_NORMAL..\r\n";

					if (FAILED(device->SetCooperativeLevel( device.hWnd, DSSCL_NORMAL )))
					{
						device.Release();
						return "IDirectSound8::SetCooperativeLevel()";
					}
				}
			}

			if (cstring errMsg = buffer.Update( **device, device.priority, rate, bits, channels, latency, pool, globalFocus ))
			{
				Destroy();
				return errMsg;
			}

			return NULL;
		}

		void DirectSound::StopStream()
		{
			buffer.StopStream( device.buggy ? *device : NULL, device.priority );
		}

		DirectSound::Buffer::Com::Com()
		: writePos(0) {}

		DirectSound::Buffer::Settings::Settings()
		: size(0), pool(POOL_HARDWARE), globalFocus(false) {}

		DirectSound::Buffer::Buffer()
		{
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		}

		DirectSound::Buffer::~Buffer()
		{
			Release();
		}

		void DirectSound::Buffer::Release()
		{
			if (com)
			{
				com->Stop();
				com.Release();
			}
		}

		cstring DirectSound::Buffer::Create(IDirectSound8& device,const bool priority)
		{
			Release();

			Object::Pod<DSBUFFERDESC> desc;

			desc.dwSize = sizeof(desc);
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
			desc.lpwfxFormat = NULL;

			if (priority)
			{
				ComInterface<IDirectSoundBuffer> primary;

				if (FAILED(device.CreateSoundBuffer( &desc, &primary, NULL )) || FAILED(primary->SetFormat( &waveFormat )))
				{
					static bool logged = false;

					if (logged == false)
					{
						logged = true;
						Io::Log() << "DirectSound: warning, couldn't set the sample format for the primary sound buffer!\r\n";
					}
				}
			}

			NST_ASSERT( settings.size % waveFormat.nBlockAlign == 0 );

			desc.Clear();
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;

			if (settings.globalFocus)
				desc.dwFlags |= DSBCAPS_GLOBALFOCUS;

			if (settings.pool == POOL_SYSTEM)
				desc.dwFlags |= DSBCAPS_LOCSOFTWARE;

			desc.dwBufferBytes = settings.size;
			desc.lpwfxFormat = &waveFormat;

			ComInterface<IDirectSoundBuffer> oldCom;

			if (FAILED(device.CreateSoundBuffer( &desc, &oldCom, NULL )))
			{
				if (!(desc.dwFlags & DSBCAPS_LOCSOFTWARE))
				{
					desc.dwFlags |= DSBCAPS_LOCSOFTWARE;

					static bool logged = false;

					if (logged == false)
					{
						logged = true;
						Io::Log() << "DirectSound: warning, couldn't create the sound buffer! Retrying with software buffers..\r\n";
					}

					if (FAILED(device.CreateSoundBuffer( &desc, &oldCom, NULL )))
						return "IDirectSound8::CreateSoundBuffer()";
				}
			}

			if (FAILED(oldCom->QueryInterface( IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&com) )))
				return "IDirectSoundBuffer::QueryInterface()";

			return NULL;
		}

		cstring DirectSound::Buffer::Update
		(
			IDirectSound8& device,
			const bool priority,
			const uint rate,
			const uint bits,
			const Channels channels,
			const uint latency,
			const Pool pool,
			const bool globalFocus
		)
		{
			const uint size = rate * latency / 1000 * (bits / 8 * channels);

			if
			(
				com &&
				waveFormat.nSamplesPerSec == rate &&
				waveFormat.wBitsPerSample == bits &&
				waveFormat.nChannels == channels &&
				settings.size == size &&
				settings.pool == pool &&
				settings.globalFocus == globalFocus
			)
				return NULL;

			waveFormat.nSamplesPerSec = rate;
			waveFormat.wBitsPerSample = bits;
			waveFormat.nChannels = channels;
			waveFormat.nBlockAlign = waveFormat.wBitsPerSample / 8 * waveFormat.nChannels;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

			settings.size = size;
			settings.pool = pool;
			settings.globalFocus = globalFocus;

			return Create( device, priority );
		}

		void DirectSound::Buffer::StartStream()
		{
			if (com)
			{
				DWORD status;

				if (FAILED(com->GetStatus( &status )))
				{
					com.Release();
					return;
				}

				if ((status & (DSBSTATUS_BUFFERLOST|DSBSTATUS_LOOPING|DSBSTATUS_PLAYING)) == (DSBSTATUS_LOOPING|DSBSTATUS_PLAYING))
					return;

				if (status & DSBSTATUS_PLAYING)
					com->Stop();

				if (status & DSBSTATUS_BUFFERLOST)
				{
					const HRESULT hResult = com->Restore();

					if (FAILED(hResult) && hResult != DSERR_BUFFERLOST)
						com.Release();

					return;
				}

				void* data;
				DWORD size;

				if (SUCCEEDED(com->Lock( 0, 0, &data, &size, NULL, NULL, DSBLOCK_ENTIREBUFFER )))
				{
					std::memset( data, waveFormat.wBitsPerSample == 16 ? DC_OFFSET_16 : DC_OFFSET_8, size );
					com->Unlock( data, size, NULL, 0 );
				}

				com.writePos = 0;
				com->SetCurrentPosition( 0 );
				const HRESULT hResult = com->Play( 0, 0, DSBPLAY_LOOPING );

				if (FAILED(hResult) && hResult != DSERR_BUFFERLOST)
					com.Release();
			}
		}

		void DirectSound::Buffer::StopStream(IDirectSound8* const device,const bool priority)
		{
			if (com)
			{
				if (device)
					Create( *device, priority );
				else
					com->Stop();
			}
		}
	}
}
