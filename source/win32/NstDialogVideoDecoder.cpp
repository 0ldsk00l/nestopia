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

#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogVideoDecoder.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_DECODER_GY_VALUE == IDC_VIDEO_DECODER_RY_VALUE + 1 &&
			IDC_VIDEO_DECODER_BY_VALUE == IDC_VIDEO_DECODER_RY_VALUE + 2
		);

		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_DECODER_GY_NUM == IDC_VIDEO_DECODER_RY_NUM + 1 &&
			IDC_VIDEO_DECODER_BY_NUM == IDC_VIDEO_DECODER_RY_NUM + 2
		);

		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_DECODER_GY_GAIN == IDC_VIDEO_DECODER_RY_GAIN + 1 &&
			IDC_VIDEO_DECODER_BY_GAIN == IDC_VIDEO_DECODER_RY_GAIN + 2
		);

		struct VideoDecoder::Handlers
		{
			static const MsgHandler::Entry<VideoDecoder> messages[];
			static const MsgHandler::Entry<VideoDecoder> commands[];
		};

		const MsgHandler::Entry<VideoDecoder> VideoDecoder::Handlers::messages[] =
		{
			{ WM_INITDIALOG,  &VideoDecoder::OnInitDialog },
			{ WM_HSCROLL,     &VideoDecoder::OnHScroll    }
		};

		const MsgHandler::Entry<VideoDecoder> VideoDecoder::Handlers::commands[] =
		{
			{ IDC_VIDEO_DECODER_RY_GAIN,      &VideoDecoder::OnCmdGain        },
			{ IDC_VIDEO_DECODER_GY_GAIN,      &VideoDecoder::OnCmdGain        },
			{ IDC_VIDEO_DECODER_BY_GAIN,      &VideoDecoder::OnCmdGain        },
			{ IDC_VIDEO_DECODER_BOOST_YELLOW, &VideoDecoder::OnCmdBoostYellow },
			{ IDC_VIDEO_DECODER_CANONICAL ,   &VideoDecoder::OnCmdPreset      },
			{ IDC_VIDEO_DECODER_CONSUMER,     &VideoDecoder::OnCmdPreset      },
			{ IDC_VIDEO_DECODER_ALTERNATIVE,  &VideoDecoder::OnCmdPreset      },
			{ IDOK,                           &VideoDecoder::OnCmdOk          }
		};

		VideoDecoder::VideoDecoder(Nes::Video e)
		:
		dialog (IDD_VIDEO_DECODER,this,Handlers::messages,Handlers::commands),
		nes    (e),
		final  (e.GetDecoder())
		{
		}

		VideoDecoder::~VideoDecoder()
		{
			nes.SetDecoder( final );
		}

		void VideoDecoder::Load(const Configuration& cfg,Nes::Video nes)
		{
			Configuration::ConstSection decoder( cfg["video"]["decoder"] );

			Nes::Video::Decoder nesDecoder( Nes::Video::DECODER_CANONICAL );

			nesDecoder.boostYellow = decoder[ "yellow-boost" ].Yes();

			for (uint i=0; i < 3; ++i)
			{
				static const char types[3][3] = {"ry","gy","by"};

				Configuration::ConstSection axis( decoder[types[i]] );

				GenericString string( axis[ "angle" ].Str() );

				if (string.Length())
					string >> nesDecoder.axes[i].angle;

				string = axis[ "gain" ].Str();

				if (string.Length())
				{
					nesDecoder.axes[i].gain = std::atof( String::Heap<char>(string).Ptr() );
					nesDecoder.axes[i].gain = NST_CLAMP(nesDecoder.axes[i].gain,0.0f,2.0f);
				}
			}

			nes.SetDecoder( nesDecoder );
		}

		void VideoDecoder::Save(Configuration& cfg,const Nes::Video nes)
		{
			Configuration::Section decoder( cfg["video"]["decoder"] );

			const Nes::Video::Decoder& nesDecoder = nes.GetDecoder();

			decoder[ "yellow-boost" ].YesNo() = nesDecoder.boostYellow;

			for (uint i=0; i < 3; ++i)
			{
				static const char types[3][3] = {"ry","gy","by"};

				Configuration::Section axis( decoder[types[i]] );

				axis[ "angle" ].Int() = nesDecoder.axes[i].angle;
				axis[ "gain"  ].Str() = RealString( nesDecoder.axes[i].gain, 3, true );
			}
		}

		ibool VideoDecoder::OnInitDialog(Param&)
		{
			for (uint i=0; i < 3; ++i)
			{
				dialog.Slider( IDC_VIDEO_DECODER_RY_VALUE+i ).SetRange( 0, 60 );
				dialog.Edit( IDC_VIDEO_DECODER_RY_GAIN+i ).Limit( 5 );
			}

			Update();

			return true;
		}

		ibool VideoDecoder::OnHScroll(Param& param)
		{
			const uint i = param.Slider().GetId() - IDC_VIDEO_DECODER_RY_VALUE;

			if (i < 3)
			{
				Nes::Video::Decoder decoder( nes.GetDecoder() );

				static const ushort offsets[3] = {60, 200, 330};
				uint angle = param.Slider().Scroll() + offsets[i];

				if (angle >= 360)
					angle -= 360;

				if (decoder.axes[i].angle != angle)
				{
					decoder.axes[i].angle = angle;
					nes.SetDecoder( decoder );
					dialog.Edit( IDC_VIDEO_DECODER_RY_NUM+i ) << angle;
					Application::Instance::GetMainWindow().Redraw();
				}
			}

			return true;
		}

		ibool VideoDecoder::OnCmdGain(Param& param)
		{
			if (param.Edit().Changed())
			{
				String::Heap<char> string;
				dialog.Edit( param.Edit().GetId() ) >> string;

				float gain = std::atof( string.Ptr() );
				gain = NST_CLAMP(gain,0.0f,2.0f);

				Nes::Video::Decoder decoder( nes.GetDecoder() );
				decoder.axes[param.Edit().GetId() - IDC_VIDEO_DECODER_RY_GAIN].gain = gain;

				if (nes.SetDecoder( decoder ) != Nes::RESULT_NOP)
					Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoDecoder::OnCmdBoostYellow(Param& param)
		{
			if (param.Button().Clicked())
			{
				Nes::Video::Decoder decoder( nes.GetDecoder() );
				decoder.boostYellow = dialog.CheckBox(IDC_VIDEO_DECODER_BOOST_YELLOW).Checked();
				nes.SetDecoder( decoder );
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoDecoder::OnCmdPreset(Param& param)
		{
			if (param.Button().Clicked())
			{
				Nes::Video::DecoderPreset preset;

				switch (param.Button().GetId())
				{
					case IDC_VIDEO_DECODER_CONSUMER:    preset = Nes::Video::DECODER_CONSUMER;    break;
					case IDC_VIDEO_DECODER_ALTERNATIVE: preset = Nes::Video::DECODER_ALTERNATIVE; break;
					default:                            preset = Nes::Video::DECODER_CANONICAL;   break;
				}

				nes.SetDecoder( preset );
				Update();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoDecoder::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				final = nes.GetDecoder();
				dialog.Close();
			}

			return true;
		}

		void VideoDecoder::Update() const
		{
			Nes::Video::Decoder decoder( nes.GetDecoder() );

			for (uint i=0; i < 3; ++i)
			{
				int angle = decoder.axes[i].angle;

				dialog.Edit( IDC_VIDEO_DECODER_RY_NUM+i ) << uint(angle);
				static const short offsets[3] = {60, 200, 330};

				angle -= offsets[i];

				if (angle < 0)
					angle += 360;

				dialog.Slider( IDC_VIDEO_DECODER_RY_VALUE+i ).Position() = uint(angle);
				dialog.Edit( IDC_VIDEO_DECODER_RY_GAIN+i ).Text() << RealString( decoder.axes[i].gain, 3, true ).Ptr();
			}

			dialog.CheckBox( IDC_VIDEO_DECODER_BOOST_YELLOW ).Check( decoder.boostYellow );
		}
	}
}

