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

#include "NstWindowParam.hpp"
#include "NstResourceString.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogImageInfo.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "../core/api/NstApiFds.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	namespace Window
	{
		ImageInfo::ImageInfo(Managers::Emulator& e)
		:
		dialog   (IDD_IMAGE_INFO,WM_INITDIALOG,this,&ImageInfo::OnInitDialog),
		emulator (e)
		{
		}

		ibool ImageInfo::OnInitDialog(Param&)
		{
			struct Table
			{
				static void Tab(HeapString* const strings,const uint count,const bool fixed)
				{
					if (fixed)
					{
						uint maxTab = 0;

						for (uint i=0; i < count; ++i)
							maxTab = NST_MAX(maxTab,strings[i].Length());

						maxTab += 2;

						for (uint i=0; i < count; ++i)
						{
							if (uint pos = strings[i].Length())
							{
								strings[i].Resize( maxTab );
								strings[i][pos] = ':';

								while (++pos < maxTab)
									strings[i][pos] = ' ';
							}
						}
					}
					else for (uint i=0; i < count; ++i)
					{
						if (strings[i].Length())
							strings[i] << ": ";
					}
				}

				static void Output(HeapString& output,HeapString* const strings,const uint count)
				{
					for (uint i=0; i < count; ++i)
					{
						if (strings[i].Length())
						{
							output << strings[i];

							if (i < count-1)
								output << "\r\n";
						}
					}
				}
			};

			HeapString text;

			const bool fixedFont = dialog.Edit(IDC_IMAGE_INFO_EDIT).FixedFont();

			if (emulator.IsCart())
			{
				typedef Nes::Cartridge::Profile Profile;

				enum
				{
					SP_H = Profile::Board::SOLDERPAD_H,
					SP_V = Profile::Board::SOLDERPAD_V
				};

				const Profile& profile = *Nes::Cartridge(emulator).GetProfile();

				HeapString types[] =
				{
					!profile.game.title.empty() ?                    Resource::String( IDS_TEXT_NAME        ) : HeapString(),
					!profile.game.altTitle.empty() ?                 Resource::String( IDS_TEXT_ALTNAME     ) : HeapString(),
					!profile.game.publisher.empty() ?                Resource::String( IDS_TEXT_PUBLISHER   ) : HeapString(),
					!profile.game.developer.empty() ?                Resource::String( IDS_TEXT_DEVELOPER   ) : HeapString(),
					!profile.game.region.empty() ?                   Resource::String( IDS_TEXT_REGION      ) : HeapString(),
					profile.game.players ?                           Resource::String( IDS_TEXT_PLAYERS     ) : HeapString(),
                                                                     Resource::String( IDS_TEXT_FILE        ),
                                                                     Resource::String( IDS_TEXT_DIRECTORY   ),
                                                                     Resource::String( IDS_TEXT_SOFTPATCHED ),
                                                                                       "CRC",
                                                                                       "SHA-1",
                                                                     Resource::String( IDS_TEXT_SYSTEM      ),
                                                                     Resource::String( IDS_TEXT_BOARD       ),
					profile.board.GetPrg() ?                                           "PRG-ROM"              : HeapString(),
					profile.board.GetChr() ?                                           "CHR-ROM"              : HeapString(),
					profile.board.GetVram() ?                                          "V-RAM"                : HeapString(),
					profile.board.GetWram() ?                                          "W-RAM"                : HeapString(),
					!profile.board.chips.empty() ?                   Resource::String( IDS_TEXT_CHIPS       ) : HeapString(),
					profile.board.solderPads & (SP_H|SP_V) ?         Resource::String( IDS_TEXT_SOLDERPAD   ) : HeapString(),
                                                                     Resource::String( IDS_TEXT_BATTERY     ),
					profile.board.HasBattery() ?                     Resource::String( IDS_TEXT_FILE        ) : HeapString(),
					profile.board.HasBattery() ?                     Resource::String( IDS_TEXT_DIRECTORY   ) : HeapString(),
                                                                     Resource::String( IDS_TEXT_DUMP        )
				};

				Table::Tab( types, sizeof(array(types)), fixedFont );

				if (!profile.game.title.empty())
					types[0] << profile.game.title.c_str();

				if (!profile.game.title.empty())
					types[1] << profile.game.altTitle.c_str();

				if (!profile.game.publisher.empty())
					types[2] << profile.game.publisher.c_str();

				if (!profile.game.developer.empty())
					types[3] << profile.game.developer.c_str();

				if (!profile.game.region.empty())
					types[4] << profile.game.region.c_str();

				if (profile.game.players)
					types[5] << profile.game.players;

				types[6] << emulator.GetImagePath().File();
				types[7] << emulator.GetImagePath().Directory();

				types[8] << Resource::String( profile.patched ? IDS_TEXT_YES : IDS_TEXT_NO );;

				{
					char sha1[41] = {0};
					char crc32[9] = {0};

					profile.hash.Get( sha1, crc32 );

					types[9] << crc32;
					types[10] << sha1;
				}

				types[11] << Resource::String
				(
					profile.system.type == Profile::System::VS_UNISYSTEM  ? IDS_TEXT_VSUNISYSTEM  :
					profile.system.type == Profile::System::VS_DUALSYSTEM ? IDS_TEXT_VSDUALSYSTEM :
					profile.system.type == Profile::System::PLAYCHOICE_10 ? IDS_TEXT_PLAYCHOICE10 :
					profile.system.type == Profile::System::NES_PAL       ? IDS_TEXT_NES_PAL      :
					profile.system.type == Profile::System::NES_PAL_A     ? IDS_TEXT_NES_PAL_A    :
					profile.system.type == Profile::System::NES_PAL_B     ? IDS_TEXT_NES_PAL_B    :
					profile.system.type == Profile::System::FAMICOM       ? IDS_TEXT_FAMICOM      :
					profile.system.type == Profile::System::DENDY         ? IDS_TEXT_DENDY        :
																			IDS_TEXT_NES_NTSC
				);

				types[12] << profile.board.type.c_str();

				if (profile.board.mapper && profile.board.mapper != Profile::Board::NO_MAPPER)
					types[12] << ", " << Resource::String( IDS_TEXT_MAPPER ) << ' ' << profile.board.mapper;

				if (const uint prg = profile.board.GetPrg())
					types[13] << (prg % 1024 ? prg : prg / 1024) << (prg % 1024 ? " bytes" : "k");

				if (const uint chr = profile.board.GetChr())
					types[14] << (chr % 1024 ? chr : chr / 1024) << (chr % 1024 ? " bytes" : "k");

				if (const uint vram = profile.board.GetVram())
					types[15] << (vram % 1024 ? vram : vram / 1024) << (vram % 1024 ? " bytes" : "k");

				if (const uint wram = profile.board.GetWram())
					types[16] << (wram % 1024 ? wram : wram / 1024) << (wram % 1024 ? " bytes" : "k");

				for (uint i=0, n=profile.board.chips.size(); i < n; ++i)
				{
					if (i)
						types[17] << ", ";

					types[17] << profile.board.chips[i].type.c_str();
				}

				if (profile.board.solderPads & (SP_H|SP_V))
				{
					types[18] << "H:"  << ((profile.board.solderPads & SP_H) ? '1' : '0')
                              << " V:" << ((profile.board.solderPads & SP_V) ? '1' : '0');
				}

				types[19] << Resource::String( profile.board.HasBattery() ? IDS_TEXT_YES : IDS_TEXT_NO );

				if (profile.board.HasBattery())
				{
					types[20] << emulator.GetSavePath().File();
					types[21] << emulator.GetSavePath().Directory();
				}

				types[22] << Resource::String
				(
					profile.dump.state == Profile::Dump::OK  ? IDS_TEXT_OK :
					profile.dump.state == Profile::Dump::BAD ? IDS_TEXT_BAD :
                                                               IDS_TEXT_UNKNOWN
				);

				Table::Output( text, types, sizeof(array(types)) );
			}
			else if (emulator.IsFds())
			{
				const Nes::Fds fds(emulator);

				std::vector<HeapString> types( 3 + NST_MAX(fds.GetNumSides()/2,1) );

				types[0] = Resource::String( IDS_TEXT_FILE      );
				types[1] = Resource::String( IDS_TEXT_DIRECTORY );
				types[2] = Resource::String( IDS_TEXT_HEADER    );

				for (uint i=3, n=types.size(); i < n; ++i)
					types[i] = Resource::String( IDS_TEXT_DISK ).Invoke( i-3+1 );

				Table::Tab( &types.front(), types.size(), fixedFont );

				types[0] << emulator.GetImagePath().File();
				types[1] << emulator.GetImagePath().Directory();
				types[2] << Resource::String( fds.HasHeader() ? IDS_TEXT_YES : IDS_TEXT_NO );

				for (uint i=0, n=fds.GetNumSides(); i < n; ++i)
				{
					Nes::Fds::DiskData data;
					fds.GetDiskData( i, data );

					uint size = 0;

					for (Nes::Fds::DiskData::Files::const_iterator it(data.files.begin()), end(data.files.end()); it != end; ++it)
						size += it->data.size();

					types[3+i/2] << (i % 2 ? ", B: " : "A: ")
                                 << (size / 1024)
                                 << "k "
                                 << Resource::String( IDS_TEXT_IN_FILES ).Invoke( data.files.size() );

					if (!data.raw.empty())
						types[3+i/2] << ", " << Resource::String( IDS_TEXT_TRAILING_DATA ).Invoke( data.raw.size() );
				}

				Table::Output( text, &types.front(), types.size() );
			}
			else if (emulator.IsNsf())
			{
				const Nes::Nsf nsf(emulator);

				HeapString types[] =
				{
											Resource::String( IDS_TEXT_FILE          ),
											Resource::String( IDS_TEXT_DIRECTORY     ),
					*nsf.GetName() ?        Resource::String( IDS_TEXT_NAME          ) : HeapString(),
					*nsf.GetArtist() ?      Resource::String( IDS_TEXT_ARTIST        ) : HeapString(),
					*nsf.GetCopyright() ?   Resource::String( IDS_TEXT_COPYRIGHT     ) : HeapString(),
											Resource::String( IDS_TEXT_REGION        ),
											Resource::String( IDS_TEXT_SONGS         ),
					nsf.GetNumSongs() > 1 ? Resource::String( IDS_TEXT_STARTINGSONG  ) : HeapString(),
											Resource::String( IDS_TEXT_EXTRACHIPS    ),
											Resource::String( IDS_TEXT_BANKSWITCHING ),
											Resource::String( IDS_TEXT_LOADADDRESS   ),
											Resource::String( IDS_TEXT_INITADDRESS   ),
											Resource::String( IDS_TEXT_PLAYADDRESS   )
				};

				Table::Tab( types, sizeof(array(types)), fixedFont );

				types[0] << emulator.GetImagePath().File();
				types[1] << emulator.GetImagePath().Directory();

				if (*nsf.GetName())
					types[2].Import( nsf.GetName() );

				if (*nsf.GetArtist())
					types[3].Import( nsf.GetArtist() );

				if (*nsf.GetCopyright())
					types[4].Import( nsf.GetCopyright() );

				types[5] <<
				(
					nsf.GetMode() == Nes::Nsf::TUNE_MODE_NTSC ? "NTSC"     :
					nsf.GetMode() == Nes::Nsf::TUNE_MODE_PAL  ? "PAL"      :
																"NTSC/PAL"
				);

				types[6] << nsf.GetNumSongs();

				if (nsf.GetNumSongs() > 1)
					types[7] << (nsf.GetStartingSong() + 1);

				if (const uint chips = nsf.GetChips())
				{
					cstring c = "";

					if ( chips & Nes::Nsf::CHIP_MMC5 ) { types[8]      << "MMC5";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_VRC6 ) { types[8] << c << "VRC6";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_VRC7 ) { types[8] << c << "VRC7";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_N163 ) { types[8] << c << "N163";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_S5B  ) { types[8] << c << "Sunsoft5B"; c = "+"; }
					if ( chips & Nes::Nsf::CHIP_FDS  ) { types[8] << c << "FDS";       c = "+"; }
				}
				else
				{
					types[8] << Resource::String( IDS_TEXT_NO );
				}

				types[9] << Resource::String( nsf.UsesBankSwitching() ? IDS_TEXT_YES : IDS_TEXT_NO );

				types[10] << HexString( 16, nsf.GetLoadAddress() );
				types[11] << HexString( 16, nsf.GetInitAddress() );
				types[12] << HexString( 16, nsf.GetPlayAddress() );

				Table::Output( text, types, sizeof(array(types)) );
			}

			if (text.Length())
			{
				dialog.Edit(IDC_IMAGE_INFO_EDIT) << text.Ptr();

				Point size( dialog.Edit(IDC_IMAGE_INFO_EDIT).GetMaxTextSize() );
				Point delta( dialog.Edit(IDC_IMAGE_INFO_EDIT).GetWindow().Size() );

				if (size.x > delta.x)
				{
					size.x = delta.x;
					size.y += ::GetSystemMetrics(SM_CXVSCROLL);
					::ShowScrollBar( dialog.Control(IDC_IMAGE_INFO_EDIT).GetWindow(), SB_HORZ, true );
				}

				if (size != delta)
				{
					dialog.Edit(IDC_IMAGE_INFO_EDIT).GetWindow().Size() = size;
					delta -= size;
					dialog.Control(IDOK).GetWindow().Position() -= delta;
					dialog.Size() -= delta;
				}
			}

			return true;
		}
	}
}
