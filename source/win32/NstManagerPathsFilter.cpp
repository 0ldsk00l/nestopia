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

#include "NstResourceString.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPathsFilter.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Paths::Filter::Filter(const File::Types types)
		{
			NST_ASSERT( types.Word() );

			(*this) << Resource::String(IDS_FILES_SUPPORTED);

			uint offset = Length();

			if (types( File::XML     )) (*this) << ", *.xml";
			if (types( File::INES    )) (*this) << ", *.nes";
			if (types( File::UNIF    )) (*this) << ", *.unf";
			if (types( File::FDS     )) (*this) << ", *.fds";
			if (types( File::NSF     )) (*this) << ", *.nsf";
			if (types( File::ROM     )) (*this) << ", *.rom";
			if (types( File::BATTERY )) (*this) << ", *.sav";
			if (types( File::TAPE    )) (*this) << ", *.tp";
			if (types( File::STATE   )) (*this) << ", *.nst";
			if (types( File::SLOTS   )) (*this) << ", *.ns1..9";
			if (types( File::MOVIE   )) (*this) << ", *.nsv";
			if (types( File::IPS     )) (*this) << ", *.ips";
			if (types( File::UPS     )) (*this) << ", *.ups";
			if (types( File::PALETTE )) (*this) << ", *.pal";
			if (types( File::WAVE    )) (*this) << ", *.wav";
			if (types( File::AVI     )) (*this) << ", *.avi";
			if (types( File::ARCHIVE )) (*this) << ", *.zip, *.rar, *.7z";

			if (Length() <= MAX_FILE_DIALOG_WIDTH)
			{
				At(offset + 0) = ' ';
				At(offset + 1) = '(';
				(*this) << ')';

				offset = Length();
			}
			else
			{
				ShrinkTo( offset );
			}

			if (types( File::XML     )) (*this) << ";*.xml";
			if (types( File::INES    )) (*this) << ";*.nes";
			if (types( File::UNIF    )) (*this) << ";*.unf;*.unif";
			if (types( File::FDS     )) (*this) << ";*.fds";
			if (types( File::NSF     )) (*this) << ";*.nsf";
			if (types( File::ROM     )) (*this) << ";*.rom";
			if (types( File::BATTERY )) (*this) << ";*.sav";
			if (types( File::TAPE    )) (*this) << ";*.tp";
			if (types( File::STATE   )) (*this) << ";*.nst";
			if (types( File::MOVIE   )) (*this) << ";*.nsv";
			if (types( File::IPS     )) (*this) << ";*.ips";
			if (types( File::UPS     )) (*this) << ";*.ups";
			if (types( File::SLOTS   )) (*this) << ";*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9";
			if (types( File::PALETTE )) (*this) << ";*.pal";
			if (types( File::WAVE    )) (*this) << ";*.wav";
			if (types( File::AVI     )) (*this) << ";*.avi";
			if (types( File::ARCHIVE )) (*this) << ";*.zip;*.rar;*.7z";

			At(offset) = '\t';

			if (types( File::XML      )) (*this) << '\t' << Resource::String( IDS_FILES_XML     ) << " (*.xml)\t*.xml";
			if (types( File::INES     )) (*this) << '\t' << Resource::String( IDS_FILES_INES    ) << " (*.nes)\t*.nes";
			if (types( File::UNIF     )) (*this) << '\t' << Resource::String( IDS_FILES_UNIF    ) << " (*.unf,*.unif)\t*.unf;*.unif";
			if (types( File::FDS      )) (*this) << '\t' << Resource::String( IDS_FILES_FDS     ) << " (*.fds)\t*.fds";
			if (types( File::NSF      )) (*this) << '\t' << Resource::String( IDS_FILES_NSF     ) << " (*.nsf)\t*.nsf";
			if (types( File::ROM      )) (*this) << '\t' << Resource::String( IDS_FILES_ROM     ) << " (*.rom)\t*.rom";
			if (types( File::BATTERY  )) (*this) << '\t' << Resource::String( IDS_FILES_BATTERY ) << " (*.sav)\t*.sav";
			if (types( File::TAPE     )) (*this) << '\t' << Resource::String( IDS_FILES_TAPE    ) << " (*.tp)\t*.tp";

			switch (types( File::STATE|File::SLOTS ))
			{
				case File::STATE:             (*this) << '\t' << Resource::String( IDS_FILES_STATE       ) << " (*.nst)\t*.nst"; break;
				case File::SLOTS:             (*this) << '\t' << Resource::String( IDS_FILES_STATE_SLOTS ) << " (*.ns1..9)\t*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"; break;
				case File::STATE|File::SLOTS: (*this) << '\t' << Resource::String( IDS_FILES_STATE       ) << " (*.nst,*.ns1..9)\t*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"; break;
			}

			if (types( File::MOVIE    )) (*this) << '\t' << Resource::String( IDS_FILES_MOVIE   ) << " (*.nsv)\t*.nsv";
			if (types( File::IPS      )) (*this) << '\t' << Resource::String( IDS_FILES_PATCHES ) << " (*.ips)\t*.ips";
			if (types( File::UPS      )) (*this) << '\t' << Resource::String( IDS_FILES_PATCHES ) << " (*.ups)\t*.ups";
			if (types( File::PALETTE  )) (*this) << '\t' << Resource::String( IDS_FILES_PALETTE ) << " (*.pal)\t*.pal";
			if (types( File::WAVE     )) (*this) << '\t' << Resource::String( IDS_FILES_WAVE    ) << " (*.wav)\t*.wav";
			if (types( File::AVI      )) (*this) << '\t' << Resource::String( IDS_FILES_AVI     ) << " (*.avi)\t*.avi";
			if (types( File::ARCHIVE  )) (*this) << '\t' << Resource::String( IDS_FILES_ARCHIVE ) << " (*.zip,*.rar,*.7z)\t*.zip;*.rar;*.7z";

			(*this) << '\t' << Resource::String( IDS_FILES_ALL ) << " (*.*)\t*.*\t";
		}
	}
}
