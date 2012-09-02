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
#include "NstManagerEmulator.hpp"
#include "NstDialogBarcodeReader.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct BarcodeReader::Handlers
		{
			static const MsgHandler::Entry<BarcodeReader> messages[];
			static const MsgHandler::Entry<BarcodeReader> commands[];
		};

		const MsgHandler::Entry<BarcodeReader> BarcodeReader::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &BarcodeReader::OnInitDialog }
		};

		const MsgHandler::Entry<BarcodeReader> BarcodeReader::Handlers::commands[] =
		{
			{ IDC_BARCODE_TRANSFER, &BarcodeReader::OnCmdTransfer },
			{ IDC_BARCODE_RANDOM,   &BarcodeReader::OnCmdRandom   },
			{ IDC_BARCODE_DIGITS,   &BarcodeReader::OnCmdDigits   }
		};

		BarcodeReader::BarcodeReader(Managers::Emulator& emulator,String::Heap<char>& string)
		:
		dialog        (IDD_BARCODE,this,Handlers::messages,Handlers::commands),
		barcodeReader (emulator),
		code          (string)
		{
		}

		ibool BarcodeReader::OnInitDialog(Param&)
		{
			Control::Edit edit( dialog.Edit(IDC_BARCODE_DIGITS) );

			edit.Limit( Nes::BarcodeReader::MAX_DIGITS );

			if (code.Length())
				edit << code.Ptr();

			if (!barcodeReader.CanTransfer())
			{
				edit.Disable();
				dialog.Control( IDC_BARCODE_RANDOM ).Disable();
				dialog.Control( IDC_BARCODE_TRANSFER ).Disable();
			}

			return true;
		}

		ibool BarcodeReader::OnCmdDigits(Param& param)
		{
			if (param.Edit().Changed())
				dialog.Edit( IDC_BARCODE_DIGITS ) >> code;

			return true;
		}

		ibool BarcodeReader::OnCmdRandom(Param& param)
		{
			if (param.Button().Clicked())
			{
				char string[Nes::BarcodeReader::MAX_DIGITS+1];

				if (barcodeReader.Randomize( string ))
					dialog.Edit( IDC_BARCODE_DIGITS ) << string;
			}

			return true;
		}

		ibool BarcodeReader::OnCmdTransfer(Param& param)
		{
			if (param.Button().Clicked() && barcodeReader.IsDigitsSupported( code.Length() ))
			{
				barcodeReader.Transfer( code.Ptr(), code.Length() );
				dialog.Close();
			}

			return true;
		}
	}
}
