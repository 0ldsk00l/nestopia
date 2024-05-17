/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2018 R. Danbrook
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "cheats.h"

static Xml savexml;
static Xml::Node saveroot;

std::vector<NstCheat> chtlist;

//extern Emulator emulator;

void nst_cheats_init(const char *cheatpath) {
	// Initialize cheat engine
	/*Cheats cheats(emulator);
	Xml xml;

	cheats.ClearCodes();

	std::ifstream cheatfile(cheatpath, std::ifstream::in|std::ifstream::binary);

	if (cheatfile.is_open()) {
		xml.Read(cheatfile);

		if (xml.GetRoot().IsType(L"cheats")) {

			Xml::Node root(xml.GetRoot());
			Xml::Node node(root.GetFirstChild());

			for (int i = 0; i < root.NumChildren(L"cheat"); i++) {

				if (node.GetAttribute(L"enabled").IsValue(L"1")) {

					if (node.GetChild(L"genie")) { // Game Genie
						nst_cheats_code_gg_add(node.GetChild(L"genie").GetValue());
					}

					else if (node.GetChild(L"rocky")) { // Pro Action Rocky
						nst_cheats_code_par_add(node.GetChild(L"rocky").GetValue());
					}

					else if (node.GetChild(L"address")) { // Raw
							Cheats::Code code;
							code.useCompare = false;

							code.address = node.GetChild(L"address").GetUnsignedValue();
							if (node.GetChild(L"value")) {
								code.value = node.GetChild(L"value").GetUnsignedValue();
							}
							if (node.GetChild(L"compare")) {
								code.compare = node.GetChild(L"compare").GetUnsignedValue();
								code.useCompare = true;
							}
							cheats.SetCode(code);
					}

					//fprintf(stderr, "Cheat: %ls\n", node.GetChild(L"description").GetValue());
				}
				NstCheat cht = {
					node.GetAttribute(L"enabled").IsValue(L"1"),
					node.GetChild(L"genie").GetValue(),
					node.GetChild(L"rocky").GetValue(),
					node.GetChild(L"address").GetUnsignedValue(),
					node.GetChild(L"value").GetUnsignedValue(),
					node.GetChild(L"compare").GetUnsignedValue(),
					node.GetChild(L"description").GetValue()
				};
				chtlist.push_back(cht);
				node = node.GetNextSibling();
			}
		}
		cheatfile.close();
	}*/
}

void nst_cheats_save(const char *cheatpath) {
	// Save the cheat list
	/*std::ofstream cheatfile(cheatpath, std::ifstream::out|std::ifstream::binary);

	if (cheatfile.is_open()) {
		saveroot = (savexml.GetRoot());

		saveroot = savexml.Create( L"cheats" );
		saveroot.AddAttribute( L"version", L"1.0" );

		char buf[9];
		wchar_t wbuf[9];

		for (int i = 0; i < chtlist.size(); i++) {
			Xml::Node node(saveroot.AddChild(L"cheat"));
			node.AddAttribute(L"enabled", chtlist[i].enabled ? L"1" : L"0");

			if (chtlist[i].gg.size() > 0) {
				node.AddChild(L"genie", chtlist[i].gg.c_str());
			}

			if (chtlist[i].par.size() > 0) {
				node.AddChild(L"rocky", chtlist[i].par.c_str());
			}

			if (chtlist[i].address != 0) {
				snprintf(buf, sizeof(buf), "0x%04X", chtlist[i].address);
				mbstowcs(wbuf, buf, 9);
				node.AddChild(L"address", wbuf);

				snprintf(buf, sizeof(buf), "0x%02x", chtlist[i].value);
				mbstowcs(wbuf, buf, 9);
				node.AddChild(L"value", wbuf);

				snprintf(buf, sizeof(buf), "0x%02x", chtlist[i].compare);
				mbstowcs(wbuf, buf, 9);
				node.AddChild(L"compare", wbuf);
			}

			if (chtlist[i].description.size() > 0) {
				node.AddChild(L"description", chtlist[i].description.c_str());
			}
		}

		savexml.Write(saveroot, cheatfile);
		cheatfile.close();
	}*/
}

void nst_cheats_code_gg_add(const std::wstring data) {
	// Add a Game Genie code
	/*Cheats cheats(emulator);
	Cheats::Code code;

	char gg[9];
	snprintf(gg, sizeof(gg), "%ls", data.c_str());

	cheats.GameGenieDecode(gg, code);
	cheats.SetCode(code);*/
}

void nst_cheats_code_par_add(const std::wstring data) {
	// Add a Pro Action Rocky code
	/*Cheats cheats(emulator);
	Cheats::Code code;

	char par[9];
	snprintf(par, sizeof(par), "%ls", data.c_str());

	cheats.ProActionRockyDecode(par, code);
	cheats.SetCode(code);*/
}

void nst_cheats_refresh() {
	/*Cheats cheats(emulator);
	cheats.ClearCodes();

	for (int i = 0; i < chtlist.size(); i++) {
		if (chtlist[i].enabled) {
			if (chtlist[i].gg.size()) {
				nst_cheats_code_gg_add(chtlist[i].gg);
			}
			else if (chtlist[i].par.size()) {
				nst_cheats_code_par_add(chtlist[i].par);
			}
			else if (chtlist[i].address) {
				Cheats::Code code;
				code.useCompare = false;
				code.address = chtlist[i].address;
				code.value = chtlist[i].value;
				code.compare = chtlist[i].compare;
				code.useCompare = code.compare != 0;
				cheats.SetCode(code);
			}
		}
	}*/
}

// DIP Switches
void nst_dip_handle(const char *dippath) {
	// Handle the DIP switch file
	/*DipSwitches dipswitches(emulator);
	Xml xml;

	std::ifstream dipfile(dippath, std::ifstream::in|std::ifstream::binary);

	if (dipfile.is_open()) {
		xml.Read(dipfile);

		if (xml.GetRoot().IsType(L"dipswitches")) {
			Xml::Node root(xml.GetRoot());
			Xml::Node node(root.GetFirstChild());

			for (int i = 0; i < root.NumChildren(L"dip"); i++) {

				if (node.GetChild(L"value")) {
					dipswitches.SetValue(i, node.GetChild(L"value").GetUnsignedValue());
				}
				node = node.GetNextSibling();
			}
		}
		dipfile.close();
	}
	else {
		Xml::Node root(xml.GetRoot());

		root = xml.Create(L"dipswitches");
		root.AddAttribute(L"version", L"1.0");

		wchar_t wbuf[32];
		char buf[32];

		int numdips = dipswitches.NumDips();

		if (numdips > 0) {
			for (int i = 0; i < numdips; i++) {
				Xml::Node node(root.AddChild(L"dip"));

				snprintf(buf, sizeof(buf), "%s", dipswitches.GetDipName(i));
				mbstowcs(wbuf, buf, sizeof(buf));
				node.AddChild(L"description", wbuf);

				snprintf(buf, sizeof(buf), "%d", dipswitches.GetValue(i));
				mbstowcs(wbuf, buf, sizeof(buf));
				node.AddChild(L"value", wbuf);
			}
		}

		std::ofstream dipout(dippath, std::ifstream::out|std::ifstream::binary);

		if (dipout.is_open()) {
			xml.Write(root, dipout);
		}

		dipout.close();
	}*/
}
