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

#include "NstInpDevice.hpp"
#include "NstInpFamilyKeyboard.hpp"
#include "../NstCpu.hpp"
#include "../NstHook.hpp"
#include "../NstFile.hpp"
#include "../api/NstApiTapeRecorder.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class FamilyKeyboard::DataRecorder
			{
			public:

				explicit DataRecorder(Cpu&);
				~DataRecorder();

				Result Record();
				Result Play();

				void SaveState(State::Saver&,dword) const;
				void LoadState(State::Loader&);

			private:

				NST_NO_INLINE void Start();
				NST_NO_INLINE Result Stop(bool);

				NES_DECL_HOOK( Tape );

				enum
				{
					MAX_LENGTH = SIZE_4096K,
					TAPE_CLOCK = 32000
				};

				enum Status
				{
					STOPPED,
					PLAYING,
					RECORDING
				};

				qaword cycles;
				Cpu& cpu;
				dword multiplier;
				dword clock;
				Status status;
				Vector<byte> stream;
				dword pos;
				uint in;
				uint out;
				File file;

			public:

				bool IsStopped() const
				{
					return status == STOPPED;
				}

				bool IsRecording() const
				{
					return status == RECORDING;
				}

				bool IsPlaying() const
				{
					return status == PLAYING;
				}

				bool Playable() const
				{
					return stream.Size();
				}

				Result Stop()
				{
					return Stop( false );
				}

				void Reset()
				{
					clock = 0;
					Stop( false );
				}

				void Poke(uint data)
				{
					out = data;
				}

				uint Peek() const
				{
					return in;
				}

				NST_SINGLE_CALL void EndFrame()
				{
					if (!clock)
						return;

					if (multiplier)
					{
						const qaword frame = qaword(cpu.GetFrameCycles()) * multiplier;
						NST_VERIFY( cycles >= frame );

						if (cycles > frame)
							cycles -= frame;
						else
							cycles = 0;
					}
					else
					{
						clock = 0;
						cpu.RemoveHook( Hook(this,&DataRecorder::Hook_Tape) );
					}
				}
			};

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			FamilyKeyboard::FamilyKeyboard(Cpu& c,bool connectDataRecorder)
			:
			Device       (c,Api::Input::FAMILYKEYBOARD),
			dataRecorder (connectDataRecorder ? new DataRecorder(c) : NULL)
			{
				FamilyKeyboard::Reset();
			}

			FamilyKeyboard::DataRecorder::DataRecorder(Cpu& c)
			: cycles(0), cpu(c), multiplier(0), clock(0), status(STOPPED), pos(0), in(0), out(0)
			{
				file.Load( File::TAPE, stream, MAX_LENGTH );
			}

			FamilyKeyboard::~FamilyKeyboard()
			{
				delete dataRecorder;
			}

			FamilyKeyboard::DataRecorder::~DataRecorder()
			{
				Stop( true );

				if (stream.Size())
					file.Save( File::TAPE, stream.Begin(), stream.Size() );
			}

			void FamilyKeyboard::Reset()
			{
				scan = 0;
				mode = 0;

				if (dataRecorder)
					dataRecorder->Reset();
			}

			void FamilyKeyboard::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'F','B'>::R(0,0,id) );
				saver.Begin( AsciiId<'K','B','D'>::V ).Write8( mode | (scan << 1) ).End();

				if (dataRecorder)
					dataRecorder->SaveState( saver, AsciiId<'D','T','R'>::V );

				saver.End();
			}

			void FamilyKeyboard::DataRecorder::SaveState(State::Saver& state,const dword baseChunk) const
			{
				if (stream.Size() || status != STOPPED)
				{
					state.Begin( baseChunk );

					if (status == PLAYING)
					{
						state.Begin( AsciiId<'P','L','Y'>::V ).Write32( pos ).Write8( in ).Write32( cycles ).Write32( multiplier ).End();
					}
					else if (status == RECORDING)
					{
						state.Begin( AsciiId<'R','E','C'>::V ).Write8( out ).Write32( cycles ).Write32( multiplier ).End();
					}

					if (stream.Size())
						state.Begin( AsciiId<'D','A','T'>::V ).Write32( stream.Size() ).Compress( stream.Begin(), stream.Size() ).End();

					state.End();
				}
			}

			void FamilyKeyboard::LoadState(State::Loader& loader,const dword id)
			{
				if (dataRecorder)
					dataRecorder->Stop();

				if (id == AsciiId<'F','B'>::V)
				{
					while (const dword chunk = loader.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'K','B','D'>::V:
							{
								const uint data = loader.Read8();

								mode = data & 0x1;
								scan = data >> 1 & 0xF;

								if (scan > 9)
									scan = 0;

								break;
							}

							case AsciiId<'D','T','R'>::V:

								NST_VERIFY( dataRecorder );

								if (dataRecorder)
									dataRecorder->LoadState( loader );

								break;
						}

						loader.End();
					}
				}
			}

			void FamilyKeyboard::DataRecorder::LoadState(State::Loader& state)
			{
				Stop( true );

				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'P','L','Y'>::V:

							NST_VERIFY( status == STOPPED );

							if (status == STOPPED)
							{
								status = PLAYING;
								pos = state.Read32();
								in = state.Read8() & 0x2;

								cycles = state.Read32();

								if (const dword multiplier = state.Read32())
									cycles = cycles * (cpu.GetClockDivider() * TAPE_CLOCK) / multiplier;
								else
									cycles = 0;
							}
							break;

						case AsciiId<'R','E','C'>::V:

							NST_VERIFY( status == STOPPED );

							if (status == STOPPED)
							{
								status = RECORDING;
								out = state.Read8();

								cycles = state.Read32();

								if (const dword multiplier = state.Read32())
									cycles = cycles * (cpu.GetClockDivider() * TAPE_CLOCK) / multiplier;
								else
									cycles = 0;
							}
							break;

						case AsciiId<'D','A','T'>::V:
						{
							const dword size = state.Read32();
							NST_VERIFY( size > 0 && size <= MAX_LENGTH );

							if (size > 0 && size <= MAX_LENGTH)
							{
								stream.Resize( size );
								state.Uncompress( stream.Begin(), size );
							}

							break;
						}
					}

					state.End();
				}

				if (status == PLAYING)
				{
					NST_VERIFY( pos < stream.Size() );

					if (pos < stream.Size())
					{
						Start();
					}
					else
					{
						status = STOPPED;
						cycles = 0;
						pos = 0;
						in = 0;
					}
				}
				else if (status == RECORDING)
				{
					Start();
				}
			}

			Result FamilyKeyboard::PlayTape()
			{
				return dataRecorder ? dataRecorder->Play() : RESULT_ERR_NOT_READY;
			}

			Result FamilyKeyboard::RecordTape()
			{
				return dataRecorder ? dataRecorder->Record() : RESULT_ERR_NOT_READY;
			}

			Result FamilyKeyboard::StopTape()
			{
				return dataRecorder ? dataRecorder->Stop() : RESULT_NOP;
			}

			bool FamilyKeyboard::IsTapeRecording() const
			{
				return dataRecorder ? dataRecorder->IsRecording() : false;
			}

			bool FamilyKeyboard::IsTapePlaying() const
			{
				return dataRecorder ? dataRecorder->IsPlaying() : false;
			}

			bool FamilyKeyboard::IsTapeStopped() const
			{
				return dataRecorder ? dataRecorder->IsStopped() : false;
			}

			bool FamilyKeyboard::IsTapePlayable() const
			{
				return dataRecorder ? dataRecorder->Playable() : false;
			}

			Result FamilyKeyboard::DataRecorder::Record()
			{
				if (status == RECORDING)
					return RESULT_NOP;

				if (status == PLAYING)
					return RESULT_ERR_NOT_READY;

				status = RECORDING;
				stream.Destroy();

				Start();

				return RESULT_OK;
			}

			Result FamilyKeyboard::DataRecorder::Play()
			{
				if (status == PLAYING)
					return RESULT_NOP;

				if (status == RECORDING || !Playable())
					return RESULT_ERR_NOT_READY;

				status = PLAYING;

				Start();

				return RESULT_OK;
			}

			NST_NO_INLINE void FamilyKeyboard::DataRecorder::Start()
			{
				clock = cpu.GetClockBase();
				multiplier = cpu.GetClockDivider() * TAPE_CLOCK;

				cpu.AddHook( Hook(this,&DataRecorder::Hook_Tape) );

				Api::TapeRecorder::eventCallback( status == PLAYING ? Api::TapeRecorder::EVENT_PLAYING : Api::TapeRecorder::EVENT_RECORDING );
			}

			NST_NO_INLINE Result FamilyKeyboard::DataRecorder::Stop(const bool removeHook)
			{
				if (removeHook)
					cpu.RemoveHook( Hook(this,&DataRecorder::Hook_Tape) );

				if (status == STOPPED)
					return RESULT_NOP;

				status = STOPPED;
				cycles = 0;
				multiplier = 0;
				in = 0;
				out = 0;
				pos = 0;

				Api::TapeRecorder::eventCallback( Api::TapeRecorder::EVENT_STOPPED );

				return RESULT_OK;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void FamilyKeyboard::EndFrame()
			{
				if (dataRecorder)
					dataRecorder->EndFrame();
			}

			void FamilyKeyboard::Poke(const uint data)
			{
				if (dataRecorder)
					dataRecorder->Poke( data );

				if (data & COMMAND_KEY)
				{
					const uint out = data & COMMAND_SCAN;

					if (mode && !out && ++scan > 9)
						scan = 0;

					mode = out >> 1;

					if (data & COMMAND_RESET)
						scan = 0;
				}
			}

			uint FamilyKeyboard::Peek(uint port)
			{
				if (port == 0)
				{
					return dataRecorder ? dataRecorder->Peek() : 0;
				}
				else if (input && scan < 9)
				{
					Controllers::FamilyKeyboard::callback( input->familyKeyboard, scan, mode );
					return ~uint(input->familyKeyboard.parts[scan]) & 0x1E;
				}
				else
				{
					return 0x1E;
				}
			}

			NES_HOOK(FamilyKeyboard::DataRecorder,Tape)
			{
				for (const qaword next = qaword(cpu.GetCycles()) * multiplier; cycles < next; cycles += clock)
				{
					if (status == PLAYING)
					{
						if (pos < stream.Size())
						{
							const uint data = stream[pos++];

							if (data >= 0x8C)
							{
								in = 0x2;
							}
							else if (data <= 0x74)
							{
								in = 0x0;
							}
						}
						else
						{
							Stop( false );
							break;
						}
					}
					else
					{
						NST_ASSERT( status == RECORDING );

						if (stream.Size() < MAX_LENGTH)
						{
							stream.Append( (out & 0x7) == 0x7 ? 0x90 : 0x70 );
						}
						else
						{
							Stop( false );
							break;
						}
					}
				}
			}
		}
	}
}
