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

#include <algorithm>
#include "NstApplicationInstance.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogSound.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_SOUND_8_BIT         == IDC_SOUND_SAMPLE_RATE + 1 &&
			IDC_SOUND_16_BIT        == IDC_SOUND_SAMPLE_RATE + 2 &&
			IDC_SOUND_LATENCY       == IDC_SOUND_SAMPLE_RATE + 3 &&
			IDC_SOUND_LATENCY_ONE   == IDC_SOUND_SAMPLE_RATE + 4 &&
			IDC_SOUND_LATENCY_TEN   == IDC_SOUND_SAMPLE_RATE + 5 &&
			IDC_SOUND_MONO          == IDC_SOUND_SAMPLE_RATE + 6 &&
			IDC_SOUND_STEREO        == IDC_SOUND_SAMPLE_RATE + 7 &&
			IDC_SOUND_POOL_SYSTEM   == IDC_SOUND_SAMPLE_RATE + 8 &&
			IDC_SOUND_POOL_HARDWARE == IDC_SOUND_SAMPLE_RATE + 9 &&
			IDC_SOUND_ADJUST_PITCH  == IDC_SOUND_SAMPLE_RATE + 10
		);

		const Sound::ChannelLut Sound::channelLut[NUM_CHANNELS] =
		{
			{ NULL,       "master",   IDC_SOUND_MASTER_SLIDER,   IDC_SOUND_MASTER_VALUE,   IDC_SOUND_MASTER_TEXT,   0                            },
			{ "apu",      "square-1", IDC_SOUND_SQUARE1_SLIDER,  IDC_SOUND_SQUARE1_VALUE,  IDC_SOUND_SQUARE1_TEXT,  Nes::Sound::CHANNEL_SQUARE1  },
			{ "apu",      "square-2", IDC_SOUND_SQUARE2_SLIDER,  IDC_SOUND_SQUARE2_VALUE,  IDC_SOUND_SQUARE2_TEXT,  Nes::Sound::CHANNEL_SQUARE2  },
			{ "apu",      "triangle", IDC_SOUND_TRIANGLE_SLIDER, IDC_SOUND_TRIANGLE_VALUE, IDC_SOUND_TRIANGLE_TEXT, Nes::Sound::CHANNEL_TRIANGLE },
			{ "apu",      "noise",    IDC_SOUND_NOISE_SLIDER,    IDC_SOUND_NOISE_VALUE,    IDC_SOUND_NOISE_TEXT,    Nes::Sound::CHANNEL_NOISE    },
			{ "apu",      "dpcm",     IDC_SOUND_DPCM_SLIDER,     IDC_SOUND_DPCM_VALUE,     IDC_SOUND_DPCM_TEXT,     Nes::Sound::CHANNEL_DPCM     },
			{ "external", "fds",      IDC_SOUND_FDS_SLIDER,      IDC_SOUND_FDS_VALUE,      IDC_SOUND_FDS_TEXT,      Nes::Sound::CHANNEL_FDS      },
			{ "external", "mmc5",     IDC_SOUND_MMC5_SLIDER,     IDC_SOUND_MMC5_VALUE,     IDC_SOUND_MMC5_TEXT,     Nes::Sound::CHANNEL_MMC5     },
			{ "external", "vrc6",     IDC_SOUND_VRC6_SLIDER,     IDC_SOUND_VRC6_VALUE,     IDC_SOUND_VRC6_TEXT,     Nes::Sound::CHANNEL_VRC6     },
			{ "external", "vrc7",     IDC_SOUND_VRC7_SLIDER,     IDC_SOUND_VRC7_VALUE,     IDC_SOUND_VRC7_TEXT,     Nes::Sound::CHANNEL_VRC7     },
			{ "external", "n163",     IDC_SOUND_N163_SLIDER,     IDC_SOUND_N163_VALUE,     IDC_SOUND_N163_TEXT,     Nes::Sound::CHANNEL_N163     },
			{ "external", "s5b",      IDC_SOUND_S5B_SLIDER,      IDC_SOUND_S5B_VALUE,      IDC_SOUND_S5B_TEXT,      Nes::Sound::CHANNEL_S5B      }
		};

		struct Sound::Handlers
		{
			static const MsgHandler::Entry<Sound> messages[];
			static const MsgHandler::Entry<Sound> commands[];
		};

		const MsgHandler::Entry<Sound> Sound::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Sound::OnInitDialog },
			{ WM_VSCROLL,    &Sound::OnVScroll    }
		};

		const MsgHandler::Entry<Sound> Sound::Handlers::commands[] =
		{
			{ IDC_SOUND_DEVICE,        &Sound::OnCmdDevice       },
			{ IDC_SOUND_DEFAULT,       &Sound::OnCmdDefault      },
			{ IDC_SOUND_RESET_SLIDERS, &Sound::OnCmdResetSliders },
			{ IDOK,                    &Sound::OnCmdOk           }
		};

		Sound::Sound(Managers::Emulator& e,const Adapters& a,const Managers::Paths& paths,const Configuration& cfg)
		:
		adapters ( a ),
		nes      ( e ),
		dialog   ( IDD_SOUND, this, Handlers::messages, Handlers::commands ),
		recorder ( paths )
		{
			Configuration::ConstSection sound( cfg["sound"] );

			settings.adapter = UINT_MAX;

			if (adapters.size())
			{
				const GenericString device( sound["device"].Str() );

				if (device != L"none")
				{
					const Adapters::const_iterator it( std::find( adapters.begin(), adapters.end(), System::Guid(device) ));

					if (it != adapters.end())
						settings.adapter = it - adapters.begin();
					else
						settings.adapter = GetDefaultAdapter();
				}
			}

			switch (uint rate = sound["sample-rate"].Int())
			{
				case 11025:
				case 22050:
				case 44100:
				case 48000:
				case 88200:
				case 96000:

					nes.SetSampleRate( rate );
					break;
			}

			switch (uint bits = sound["sample-bits"].Int())
			{
				case 8:
				case 16:

					nes.SetSampleBits( bits );
					break;
			}

			nes.SetAutoTranspose( sound["adjust-pitch"].Yes() );
			nes.SetSpeaker( sound["speakers"].Str() == L"stereo" ? Nes::Sound::SPEAKER_STEREO : Nes::Sound::SPEAKER_MONO );
			settings.pool = (sound["memory-pool"].Str() == L"hardware" ? DirectSound::POOL_HARDWARE : DirectSound::POOL_SYSTEM);

			settings.latency = sound["buffers"].Int( DEFAULT_LATENCY );

			if (settings.latency > LATENCY_MAX)
				settings.latency = DEFAULT_LATENCY;

			Configuration::ConstSection volumes( sound["volumes"] );

			for (uint i=0; i < NUM_CHANNELS; ++i)
			{
				uint volume;

				if (channelLut[i].cfgCategory)
					volume = volumes[channelLut[i].cfgCategory][channelLut[i].cfgChannel].Int( DEFAULT_VOLUME );
				else
					volume = volumes[channelLut[i].cfgChannel].Int( DEFAULT_VOLUME );

				if (volume > VOLUME_MAX)
					volume = DEFAULT_VOLUME;

				settings.volumes[i] = volume;

				if (channelLut[i].channel)
					nes.SetVolume( channelLut[i].channel, GetVolume( channelLut[i].channel ) );
			}
		}

		Sound::~Sound()
		{
		}

		void Sound::Save(Configuration& cfg) const
		{
			Configuration::Section sound( cfg["sound"] );

			if (settings.adapter < adapters.size())
				sound["device"].Str() = adapters[settings.adapter].guid.GetString();
			else
				sound["device"].Str() = "none";

			sound[ "sample-rate"  ].Int() = nes.GetSampleRate();
			sound[ "sample-bits"  ].Int() = nes.GetSampleBits();
			sound[ "buffers"      ].Int() = settings.latency;
			sound[ "speakers"     ].Str() = (nes.GetSpeaker() == Nes::Sound::SPEAKER_STEREO ? "stereo" : "mono");
			sound[ "adjust-pitch" ].YesNo() = nes.IsAutoTransposing();
			sound[ "memory-pool"  ].Str() = (settings.pool == DirectSound::POOL_SYSTEM ? "system" : "hardware");

			Configuration::Section volumes( sound["volumes"] );

			for (uint i=0; i < NUM_CHANNELS; ++i)
			{
				if (channelLut[i].cfgCategory)
					volumes[channelLut[i].cfgCategory][channelLut[i].cfgChannel].Int() = settings.volumes[i];
				else
					volumes[channelLut[i].cfgChannel].Int() = settings.volumes[i];
			}
		}

		uint Sound::GetVolume(const uint channel) const
		{
			if (settings.adapter != UINT_MAX)
			{
				for (uint i=1; i < NUM_CHANNELS; ++i)
				{
					if (channelLut[i].channel == channel)
					{
						const uint volume = (settings.volumes[i] * settings.volumes[0] + DEFAULT_VOLUME/2) / DEFAULT_VOLUME;
						return NST_MIN(volume,VOLUME_MAX);
					}
				}
			}

			return 0;
		}

		uint Sound::GetDefaultAdapter() const
		{
			const Adapters::const_iterator it( std::find( adapters.begin(), adapters.end(), System::Guid() ) );
			return it != adapters.end() ? it - adapters.begin() : 0;
		}

		ibool Sound::OnInitDialog(Param&)
		{
			if (adapters.size())
			{
				{
					const Control::ComboBox comboBox( dialog.ComboBox(IDC_SOUND_DEVICE) );

					comboBox.Add( Resource::String(IDS_TEXT_NONE) );

					for (Adapters::const_iterator it(adapters.begin()), end(adapters.end()); it != end; ++it)
						comboBox.Add( it->name.Ptr() );

					comboBox[settings.adapter+1].Select();
				}

				{
					static const wchar_t rates[][6] =
					{
						L"11025",
						L"22050",
						L"44100",
						L"48000",
						L"88200",
						L"96000"
					};

					uint index;

					switch (nes.GetSampleRate())
					{
						case 11025: index = 0; break;
						case 22050: index = 1; break;
						case 48000: index = 3; break;
						case 88200: index = 4; break;
						case 96000: index = 5; break;
						default:    index = 2; break;
					}

					const Control::ComboBox comboBox( dialog.ComboBox(IDC_SOUND_SAMPLE_RATE) );

					comboBox.Add( rates, sizeof(array(rates)) );
					comboBox[index].Select();
				}

				dialog.RadioButton( nes.GetSampleBits() == 8 ? IDC_SOUND_8_BIT : IDC_SOUND_16_BIT ).Check();
				dialog.RadioButton( nes.GetSpeaker() == Nes::Sound::SPEAKER_STEREO ? IDC_SOUND_STEREO : IDC_SOUND_MONO ).Check();
				dialog.RadioButton( settings.pool == DirectSound::POOL_HARDWARE ? IDC_SOUND_POOL_HARDWARE : IDC_SOUND_POOL_SYSTEM ).Check();

				for (uint i=0; i < NUM_CHANNELS; ++i)
				{
					const Control::Slider control( dialog.Slider(channelLut[i].ctrlSlider) );
					control.SetRange( 0, VOLUME_MAX );
					control.Position() = VOLUME_MAX - settings.volumes[i];
					dialog.Edit( channelLut[i].ctrlValue ) << uint(settings.volumes[i]);
				}

				{
					const Control::Slider control( dialog.Slider(IDC_SOUND_LATENCY) );
					control.SetRange( 1, LATENCY_MAX );
					control.Position() = settings.latency;
				}

				dialog.CheckBox( IDC_SOUND_ADJUST_PITCH ).Check( nes.IsAutoTransposing() );

				if (settings.adapter != UINT_MAX)
					UpdateVolumeReset();
				else
					Enable( false );
			}
			else
			{
				dialog.Control( IDC_SOUND_DEVICE ).Disable();
				dialog.Control( IDC_SOUND_DEFAULT ).Disable();

				Enable( false );
			}

			return true;
		}

		void Sound::Enable(const bool state) const
		{
			for (uint i=IDC_SOUND_SAMPLE_RATE; i <= IDC_SOUND_ADJUST_PITCH; ++i)
				dialog.Control( i ).Enable( state );

			for (uint i=0; i < NUM_CHANNELS; ++i)
			{
				dialog.Control( channelLut[i].ctrlSlider ).Enable( state );
				dialog.Control( channelLut[i].ctrlValue ).Enable( state );
				dialog.Control( channelLut[i].ctrlText ).Enable( state );
			}

			if (state)
				UpdateVolumeReset();
			else
				dialog.Control( IDC_SOUND_RESET_SLIDERS ).Disable();
		}

		void Sound::ResetVolumeSliders() const
		{
			for (uint i=0; i < NUM_CHANNELS; ++i)
			{
				dialog.Slider( channelLut[i].ctrlSlider ).Position() = VOLUME_MAX-DEFAULT_VOLUME;
				dialog.Edit( channelLut[i].ctrlValue ) << uint(DEFAULT_VOLUME);
			}

			dialog.Control( IDC_SOUND_RESET_SLIDERS ).Disable();
		}

		void Sound::UpdateVolumeReset() const
		{
			uint reset;

			for (reset=0; reset < NUM_CHANNELS; ++reset)
			{
				if (dialog.Slider( channelLut[reset].ctrlSlider ).Position() != VOLUME_MAX-DEFAULT_VOLUME)
					break;
			}

			dialog.Control( IDC_SOUND_RESET_SLIDERS ).Enable( reset != NUM_CHANNELS );
		}

		ibool Sound::OnVScroll(Param& param)
		{
			for (uint i=0, id=param.Slider().GetId(); i < NUM_CHANNELS; ++i)
			{
				if (id == channelLut[i].ctrlSlider)
				{
					dialog.Edit( channelLut[i].ctrlValue ) << (VOLUME_MAX - param.Slider().Scroll());
					UpdateVolumeReset();
					break;
				}
			}

			return true;
		}

		ibool Sound::OnCmdDevice(Param&)
		{
			Enable( dialog.ComboBox( IDC_SOUND_DEVICE ).Selection().GetIndex() != 0 );
			return true;
		}

		ibool Sound::OnCmdResetSliders(Param& param)
		{
			if (param.Button().Clicked())
				ResetVolumeSliders();

			return true;
		}

		ibool Sound::OnCmdDefault(Param& param)
		{
			NST_VERIFY( adapters.size() );

			if (param.Button().Clicked())
			{
				Enable( true );

				dialog.ComboBox( IDC_SOUND_DEVICE )[GetDefaultAdapter()+1].Select();
				dialog.ComboBox( IDC_SOUND_SAMPLE_RATE )[1].Select();

				dialog.RadioButton( IDC_SOUND_16_BIT ).Check();
				dialog.RadioButton( IDC_SOUND_8_BIT ).Uncheck();

				dialog.RadioButton( IDC_SOUND_MONO ).Check();
				dialog.RadioButton( IDC_SOUND_STEREO ).Uncheck();

				dialog.RadioButton( IDC_SOUND_POOL_HARDWARE ).Uncheck();
				dialog.RadioButton( IDC_SOUND_POOL_SYSTEM ).Check();

				dialog.CheckBox( IDC_SOUND_ADJUST_PITCH ).Uncheck();
				dialog.Slider( IDC_SOUND_LATENCY ).Position() = DEFAULT_LATENCY;

				ResetVolumeSliders();
			}

			return true;
		}

		ibool Sound::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				if (adapters.size())
				{
					settings.adapter = dialog.ComboBox( IDC_SOUND_DEVICE ).Selection().GetIndex() - 1U;
					settings.latency = dialog.Slider( IDC_SOUND_LATENCY ).Position();
					settings.pool = (dialog.RadioButton( IDC_SOUND_POOL_HARDWARE ).Checked() ? DirectSound::POOL_HARDWARE : DirectSound::POOL_SYSTEM);

					static const uint rates[] = {11025,22050,44100,48000,88200,96000};

					nes.SetSampleRate( rates[dialog.ComboBox( IDC_SOUND_SAMPLE_RATE ).Selection().GetIndex()] );
					nes.SetSampleBits( dialog.RadioButton( IDC_SOUND_8_BIT ).Checked() ? 8 : 16 );
					nes.SetSpeaker( dialog.RadioButton( IDC_SOUND_STEREO ).Checked() ? Nes::Sound::SPEAKER_STEREO : Nes::Sound::SPEAKER_MONO );
					nes.SetAutoTranspose( dialog.CheckBox( IDC_SOUND_ADJUST_PITCH ).Checked() );

					for (uint i=0; i < NUM_CHANNELS; ++i)
						settings.volumes[i] = VOLUME_MAX - dialog.Slider( channelLut[i].ctrlSlider ).Position();

					for (uint i=1; i < NUM_CHANNELS; ++i)
						nes.SetVolume( channelLut[i].channel, GetVolume( channelLut[i].channel ) );
				}

				dialog.Close();
			}

			return true;
		}

		struct Sound::Recorder::Handlers
		{
			static const MsgHandler::Entry<Recorder> messages[];
			static const MsgHandler::Entry<Recorder> commands[];
		};

		const MsgHandler::Entry<Sound::Recorder> Sound::Recorder::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Recorder::OnInitDialog }
		};

		const MsgHandler::Entry<Sound::Recorder> Sound::Recorder::Handlers::commands[] =
		{
			{ IDC_SOUND_CAPTURE_CLEAR,  &Recorder::OnCmdClear  },
			{ IDC_SOUND_CAPTURE_BROWSE, &Recorder::OnCmdBrowse },
			{ IDOK,                     &Recorder::OnCmdOk     }
		};

		Sound::Recorder::Recorder(const Managers::Paths& p)
		: dialog(IDD_SOUND_RECORDER,this,Handlers::messages,Handlers::commands), paths(p) {}

		Sound::Recorder::~Recorder()
		{
		}

		const Path Sound::Recorder::WaveFile() const
		{
			return Application::Instance::GetFullPath( waveFile );
		}

		ibool Sound::Recorder::OnInitDialog(Param&)
		{
			dialog.Edit( IDC_SOUND_CAPTURE_FILE ) << waveFile.Ptr();
			dialog.Edit( IDC_SOUND_CAPTURE_FILE ).Limit( MAX_PATH );

			return true;
		}

		ibool Sound::Recorder::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit(IDC_SOUND_CAPTURE_FILE).Clear();

			return true;
		}

		ibool Sound::Recorder::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				Path tmp;
				dialog.Edit(IDC_SOUND_CAPTURE_FILE) >> tmp;
				dialog.Edit(IDC_SOUND_CAPTURE_FILE).Try() << paths.BrowseSave( Managers::Paths::File::WAVE, Managers::Paths::SUGGEST, Application::Instance::GetFullPath(tmp) ).Ptr();
			}

			return true;
		}

		ibool Sound::Recorder::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Edit(IDC_SOUND_CAPTURE_FILE) >> waveFile;
				paths.FixFile( Managers::Paths::File::WAVE, waveFile );
				dialog.Close();
			}

			return true;
		}
	}
}
