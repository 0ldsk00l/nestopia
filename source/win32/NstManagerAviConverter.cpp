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

#include "NstIoFile.hpp"
#include "NstIoStream.hpp"
#include "NstObjectPod.hpp"
#include "NstWindowUser.hpp"
#include "NstManager.hpp"
#include "NstManagerAviConverter.hpp"
#include "NstManagerMovie.hpp"

#if NST_MSVC
#pragma comment(lib,"vfw32")
#endif

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <vfw.h>

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

namespace Nestopia
{
	namespace Managers
	{
		AviConverter::AviConverter(Emulator& e)
		: emulator(e), on(e.IsOn())
		{
			Nes::Video::Output::lockCallback.Get( nesVideoLockFunc, nesVideoLockData );
			Nes::Video::Output::unlockCallback.Get( nesVideoUnlockFunc, nesVideoUnlockData );
			Nes::Sound::Output::lockCallback.Get( nesSoundLockFunc, nesSoundLockData );
			Nes::Sound::Output::unlockCallback.Get( nesSoundUnlockFunc, nesSoundUnlockData );
			Nes::Movie::eventCallback.Get( nesMovieEventFunc, nesMovieEventData );

			Nes::Video::Output::lockCallback.Unset();
			Nes::Video::Output::unlockCallback.Unset();
			Nes::Sound::Output::lockCallback.Unset();
			Nes::Sound::Output::unlockCallback.Unset();
			Nes::Movie::eventCallback.Unset();

			Nes::Video(emulator).GetRenderState( renderState );

			if (!on)
				emulator.Power( true );

			if (emulator.IsOn())
				emulator.SaveState( saveState, false, Emulator::QUIETLY );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		uint AviConverter::Record(const Path& moviePath,const Path& aviPath) const
		{
			if (moviePath.Empty() || aviPath.Empty() || !emulator.IsOn())
				return 0;

			try
			{
				class Movie
				{
					Emulator& emulator;
					Io::Stream::In stream;

				public:

					explicit Movie(Emulator& e,const Path& path)
					: emulator(e), stream(path) {}

					uint Play()
					{
						const Nes::Result result = Nes::Movie(emulator).Play( stream );
						return NES_FAILED(result) ? Emulator::ResultToString( result ) : 0;
					}

					~Movie()
					{
						Nes::Movie(emulator).Stop();
					}
				};

				struct BitmapInfo : Object::Pod<BITMAPINFOHEADER>
				{
					BitmapInfo(const uint width,const uint height)
					{
						NST_ASSERT( width && height );

						biSize        = sizeof(BITMAPINFOHEADER);
						biWidth       = width;
						biHeight      = height << uint(width / height >= 2);
						biPlanes      = 1;
						biBitCount    = VIDEO_BPP;
						biCompression = BI_RGB;
						biSizeImage   = VIDEO_BPP/8 * biWidth * biHeight;
					}
				};

				struct CompressVars : Object::Pod<COMPVARS>
				{
					CompressVars()
					{
						cbSize = sizeof(COMPVARS);
					}

					~CompressVars()
					{
						::ICCompressorFree( this );
					}

					BOOL Choose(BitmapInfo& bitmapInfo)
					{
						char title[] = "Choose Video Codec";

						return ::ICCompressorChoose
						(
							Application::Instance::GetMainWindow(),
							ICMF_CHOOSE_DATARATE|ICMF_CHOOSE_KEYFRAME,
							&bitmapInfo,
							NULL,
							this,
							title
						);
					}
				};

				class File
				{
					PAVIFILE file;
					mutable wcstring name;

				public:

					explicit File(wcstring n)
					: name(n)
					{
						NST_ASSERT( name && *name );

						::AVIFileInit();

						if (::AVIFileOpen( &file, name, OF_WRITE|OF_CREATE, NULL ) != AVIERR_OK)
							file = NULL;
					}

					operator PAVIFILE() const
					{
						return file;
					}

					void SetSuccess() const
					{
						name = NULL;
					}

					~File()
					{
						if (file)
						{
							::AVIFileRelease( file );

							if (name)
								::DeleteFile( name );
						}

						::AVIFileExit();
					}
				};

				class Stream
				{
					PAVISTREAM stream;

				public:

					Stream()
					: stream(NULL) {}

					Stream(const File* avi,AVISTREAMINFO& info)
					{
						NST_ASSERT( !avi || bool(*avi) );

						if (!avi || ::AVIFileCreateStream( *avi, &stream, &info ) != AVIERR_OK)
							stream = NULL;
					}

					Stream(const File& avi,AVISTREAMINFO& info)
					{
						NST_ASSERT( bool(avi) );

						if (::AVIFileCreateStream( avi, &stream, &info ) != AVIERR_OK)
							stream = NULL;
					}

					Stream(const Stream& base,AVICOMPRESSOPTIONS& options)
					{
						if (::AVIMakeCompressedStream( &stream, base, &options, NULL ) != AVIERR_OK)
							stream = NULL;
					}

					operator PAVISTREAM() const
					{
						return stream;
					}

					bool SetFormat(void* info,uint size) const
					{
						NST_ASSERT( stream && info && size );

						return ::AVIStreamSetFormat( stream, 0, info, size ) == AVIERR_OK;
					}

					~Stream()
					{
						if (stream)
							::AVIStreamRelease( stream );
					}
				};

				struct CompressOptions : Object::Pod<AVICOMPRESSOPTIONS>
				{
					CompressOptions(const CompressVars& compressVars)
					{
						fccType          = streamtypeVIDEO;
						fccHandler       = compressVars.fccHandler;
						dwKeyFrameEvery  = compressVars.lKey;
						dwQuality        = compressVars.lQ;
						dwBytesPerSecond = compressVars.lDataRate;

						if (compressVars.lDataRate > 0)
							dwFlags |= AVICOMPRESSF_DATARATE;

						if (compressVars.lKey > 0)
							dwFlags |= AVICOMPRESSF_KEYFRAMES;
					}
				};

				struct VideoInfo : Object::Pod<AVISTREAMINFO>
				{
					VideoInfo(Emulator& emulator,const BitmapInfo& bitmapInfo,const CompressVars& compressVars)
					{
						fccType               = streamtypeVIDEO;
						fccHandler            = compressVars.fccHandler;
						dwScale               = 1;
						dwRate                = emulator.GetDefaultSpeed();
						dwQuality             = compressVars.lQ;
						dwSuggestedBufferSize = bitmapInfo.biSizeImage;
						rcFrame.right         = bitmapInfo.biWidth;
						rcFrame.bottom        = bitmapInfo.biHeight;
					}
				};

				struct WaveFormat : Object::Pod<WAVEFORMATEX>
				{
					WaveFormat(const Nes::Sound nes)
					{
						wFormatTag      = WAVE_FORMAT_PCM;
						nSamplesPerSec  = nes.GetSampleRate();
						wBitsPerSample  = nes.GetSampleBits();
						nChannels       = 1 + (nes.GetSpeaker() == Nes::Sound::SPEAKER_STEREO);
						nBlockAlign     = wBitsPerSample / 8 * nChannels;
						nAvgBytesPerSec = nSamplesPerSec * nBlockAlign;
					}
				};

				struct SoundInfo : Object::Pod<AVISTREAMINFO>
				{
					SoundInfo(const WaveFormat& waveFormat)
					{
						fccType         = streamtypeAUDIO;
						fccHandler      = 1;
						dwScale         = waveFormat.nBlockAlign;
						dwInitialFrames = 1;
						dwRate          = waveFormat.nAvgBytesPerSec;
						dwQuality       = DWORD(-1);
						dwSampleSize    = waveFormat.nBlockAlign;
					}
				};

				struct Buffer
				{
					uchar* const ptr;
					const uint size;

					Buffer(uint n) : ptr(n ? new uchar [n] : NULL), size(n) {}
					~Buffer() { delete [] ptr; }
				};

				Movie movie( emulator, moviePath );

				if (const uint ids = movie.Play())
					return ids;

				BitmapInfo bitmapInfo( renderState.width, renderState.height );
				CompressVars compressVars;

				if (!compressVars.Choose( bitmapInfo ))
					return 0;

				Application::Instance::Waiter wait;

				const File avi( aviPath.Ptr() );

				if (!avi)
					return IDS_AVI_WRITE_ERR;

				VideoInfo videoInfo( emulator, bitmapInfo, compressVars );
				const Stream video( avi, videoInfo );

				if (!video)
					return IDS_AVI_WRITE_ERR;

				CompressOptions compressOptions( compressVars );
				const Stream compressor( video, compressOptions );

				if (!compressor)
					return IDS_AVI_WRITE_ERR;

				if (!compressor.SetFormat( &bitmapInfo, bitmapInfo.biSize + bitmapInfo.biClrUsed * sizeof(RGBQUAD) ))
					return IDS_AVI_WRITE_ERR;

				Window::User::Inform( IDS_AVI_WRITE_INFO );
				Application::Instance::Locker locker;

				WaveFormat waveFormat( emulator );
				SoundInfo soundInfo( waveFormat );
				const Stream sound( Nes::Sound(emulator).IsAudible() ? &avi : NULL, soundInfo );

				if (Nes::Sound(emulator).IsAudible())
				{
					if (!sound)
						return IDS_AVI_WRITE_ERR;

					if (!sound.SetFormat( &waveFormat, sizeof(WAVEFORMATEX) ))
						return IDS_AVI_WRITE_ERR;
				}

				Buffer pixels( bitmapInfo.biSizeImage );
				Buffer samples( waveFormat.nAvgBytesPerSec / videoInfo.dwRate );

				// picture will be stored upside down, adjust the pitch
				Nes::Video::Output videoOutput( pixels.ptr + (VIDEO_BPP/8 * bitmapInfo.biWidth * (renderState.height-1)), -int(VIDEO_BPP/8 * bitmapInfo.biWidth) );
				Nes::Sound::Output soundOutput( samples.ptr, samples.size / waveFormat.nBlockAlign );

				{
					Nes::Video::RenderState tmp;
					Nes::Video(emulator).GetRenderState( tmp );

					tmp.bits.count = VIDEO_BPP;
					tmp.bits.mask.r = VIDEO_R_MASK;
					tmp.bits.mask.g = VIDEO_G_MASK;
					tmp.bits.mask.b = VIDEO_B_MASK;

					Nes::Video(emulator).SetRenderState( tmp );
				}

				for (uint frame=0, sample=0, size=0; Nes::Movie(emulator).IsPlaying() && size < MAX_FILE_SIZE; ++frame, sample += soundOutput.length[0])
				{
					::Sleep( 0 );

					if (locker.CheckInput( VK_ESCAPE ))
						return IDS_AVI_WRITE_ABORT;

					if (NES_FAILED(emulator.Nes::Emulator::Execute( &videoOutput, sound ? &soundOutput : NULL, NULL )))
						return IDS_AVI_WRITE_ERR;

					if (bitmapInfo.biHeight == renderState.height * 2)
					{
						const uint pitch = -videoOutput.pitch;

						for (uchar *src=pixels.ptr + (pitch * (renderState.height-1)), *dst = pixels.ptr + (bitmapInfo.biSizeImage - pitch); ; src -= pitch)
						{
							std::memcpy( dst, src, pitch );
							dst -= pitch;

							if (dst == pixels.ptr)
								break;

							std::memcpy( dst, src, pitch );
							dst -= pitch;
						}
					}

					long written[2] = {0,0};

					if
					(
						(::AVIStreamWrite( compressor, frame, 1, pixels.ptr, pixels.size, AVIIF_KEYFRAME, NULL, written+0 ) != AVIERR_OK) ||
						(sound && ::AVIStreamWrite( sound, sample, soundOutput.length[0], samples.ptr, samples.size, 0, NULL, written+1 ) != AVIERR_OK)
					)
						return IDS_AVI_WRITE_ERR;

					size += (written[0] > 0 ? written[0] : 0) + (written[1] > 0 ? written[1] : 0);
				}

				avi.SetSuccess();

				return IDS_AVI_WRITE_FINISHED;
			}
			catch (Io::File::Exception id)
			{
				return id;
			}
			catch (...)
			{
				return IDS_ERR_GENERIC;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		AviConverter::~AviConverter()
		{
			if (saveState.Size())
				emulator.LoadState( saveState, Emulator::QUIETLY );

			if (!on)
				emulator.Power( false );

			Nes::Video::Output::lockCallback.Set( nesVideoLockFunc, nesVideoLockData );
			Nes::Video::Output::unlockCallback.Set( nesVideoUnlockFunc, nesVideoUnlockData );
			Nes::Sound::Output::lockCallback.Set( nesSoundLockFunc, nesSoundLockData );
			Nes::Sound::Output::unlockCallback.Set( nesSoundUnlockFunc, nesSoundUnlockData );
			Nes::Movie::eventCallback.Set( nesMovieEventFunc, nesMovieEventData );

			Nes::Video(emulator).SetRenderState( renderState );

			Application::Instance::GetMainWindow().Redraw();
		}
	}
}
