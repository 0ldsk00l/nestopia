/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2016 R. Danbrook
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

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "cheats.h"

extern Emulator emulator;
extern nstpaths_t nstpaths;

void cheats_init() {
	// Initialize cheat engine
	Cheats cheats(emulator);
	Xml xml;
	
	cheats.ClearCodes();
	
	std::ifstream cheatfile(nstpaths.cheatpath, std::ifstream::in|std::ifstream::binary);
	
	if (cheatfile.is_open()) {
		xml.Read(cheatfile);
		
		if (xml.GetRoot().IsType(L"cheats")) {
			
			Xml::Node root(xml.GetRoot());
			Xml::Node node(root.GetFirstChild());
			
			for (int i = 0; i < root.NumChildren(L"cheat"); i++) {
				
				if (node.GetAttribute(L"enabled").IsValue(L"1")) {
					
					if (node.GetChild(L"genie")) { // Game Genie
						cheats_code_gg_add(node.GetChild(L"genie").GetValue());
					}
					
					else if (node.GetChild(L"rocky")) { // Pro Action Rocky
						cheats_code_par_add(node.GetChild(L"rocky").GetValue());
					}
					
					else if (node.GetChild(L"address")) { // Raw
						cheats_code_raw_add(node);
					}
					
					//fprintf(stderr, "Cheat: %ls\n", node.GetChild(L"description").GetValue());
				}
				node = node.GetNextSibling();
			}
		}
		cheatfile.close();
	}
}

/*void cheats_list() {
	// List the active cheats
	Cheats cheats(emulator);
	Cheats::Code code;
	
	char gg[9];
	
	for (int i = 0; i < cheats.NumCodes(); i++) {
		cheats.GetCode(i, code);
		cheats.GameGenieEncode(code, gg);
	
		fprintf(stderr, "Cheat: %s\n", gg);
	}
}*/

void cheats_code_gg_add(const wchar_t *data) {
	// Add a Game Genie code
	Cheats cheats(emulator);
	Cheats::Code code;
	
	char gg[9];
	wcstombs(gg, data, sizeof(gg));
	
	cheats.GameGenieDecode(gg, code);
	cheats.SetCode(code);
}

void cheats_code_par_add(const wchar_t *data) {
	// Add a Pro Action Rocky code
	Cheats cheats(emulator);
	Cheats::Code code;
	
	char par[9];
	wcstombs(par, data, sizeof(par));
	
	cheats.ProActionRockyDecode(par, code);
	cheats.SetCode(code);
}

void cheats_code_raw_add(Xml::Node node) {
	// Add a Raw code
	Cheats cheats(emulator);
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

// DIP Switches
void dip_handle() {
	// Handle the DIP switch file
	DipSwitches dipswitches(emulator);
	Xml xml;
	
	char dippath[512];
	snprintf(dippath, sizeof(dippath), "%s%s.dip", nstpaths.savedir, nstpaths.gamename);
	
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
		char buf[2];
		
		int numdips = dipswitches.NumDips();
	
		if (numdips > 0) {
			for (int i = 0; i < numdips; i++) {
				Xml::Node node(root.AddChild(L"dip"));
				
				mbstowcs(wbuf, dipswitches.GetDipName(i), sizeof(wbuf));
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
	}
}
