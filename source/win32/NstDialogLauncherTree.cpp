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

#include "resource/resource.h"
#include "NstResourceString.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	namespace Window
	{
		Launcher::Tree::Tree()
		:
		selection (0),
		imageList (16,16,IDB_LAUNCHERTREE_OPEN,IDB_LAUNCHERTREE_CLOSED)
		{}

		void Launcher::Tree::operator = (const Control::TreeView& treeCtrl)
		{
			ctrl = treeCtrl;

			ctrl.SetImageList( imageList );

			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_ALL     ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_XML     ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_NES     ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_UNIF    ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_FDS     ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_NSF     ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_PATCH   ) );
			ctrl.Add( Resource::String( IDS_LAUNCHER_SECTION_ARCHIVE ) );

			ctrl[selection].Select();
		}

		void Launcher::Tree::Close()
		{
			selection = ctrl.Selection().GetIndex();
		}

		void Launcher::Tree::SetColors(const uint bg,const uint fg,const Updater redraw) const
		{
			ctrl.SetBackgroundColor( bg );
			ctrl.SetTextColor( fg );

			if (redraw)
				ctrl.Redraw();
		}

		uint Launcher::Tree::GetType(HTREEITEM const hItem) const
		{
			switch (ctrl.GetIndex( hItem ))
			{
				case 1: return List::Files::Entry::XML;
				case 2: return List::Files::Entry::NES;
				case 3: return List::Files::Entry::UNF;
				case 4: return List::Files::Entry::FDS;
				case 5: return List::Files::Entry::NSF;
				case 6: return List::Files::Entry::PATCH;
				case 7: return List::Files::Entry::ARCHIVE;
			}

			return List::Files::Entry::ALL;
		}
	}
}
