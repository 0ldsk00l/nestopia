/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2014 R. Danbrook
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

typedef Nes::Core::Xml Xml;

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
			Xml::Node node(xml.GetRoot().GetFirstChild());
			
			for (int i = 0; i < root.NumChildren(L"cheat"); i++) {
				
				if (node.GetAttribute(L"enabled").IsValue(L"1")) {
					
					if (node.GetChild(L"genie")) { // Game Genie
						cheats_code_gg_add(node.GetChild(L"genie").GetValue());
					}
					
					else if (node.GetChild(L"rocky")) { // Pro Action Rocky
					}
					
					else if (node.GetChild(L"address")) { // Raw
					}
					
					fprintf(stderr, "Cheat: %ls\n", node.GetChild(L"description").GetValue());
				}
				node = node.GetNextSibling();
			}
		}
		cheatfile.close();
	}
	
	//printf("Numcodes: %d\n", cheats.NumCodes());
}

void cheats_code_gg_add(const wchar_t *data) {
	// Add a Game Genie code
	Cheats cheats(emulator);
	Cheats::Code code;
	
	char gg[9];
	wcstombs(gg, data, sizeof(gg));
	
	cheats.GameGenieDecode(gg, code);
	cheats.SetCode(code);
}
