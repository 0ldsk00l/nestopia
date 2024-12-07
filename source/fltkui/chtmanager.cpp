/*
Copyright (c) 2012-2024 R. Danbrook
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "jg/jg.h"

#include "chtmanager.h"

CheatManager::CheatManager(JGManager& jgm) : jgm(jgm) {
}

std::vector<NstCheat>& CheatManager::get_list() {
    return chtlist;
}

void CheatManager::clear() {
    chtlist.clear();
    jgm.cheat_clear();
}

void CheatManager::refresh() {
    // First clear the active cheats
    jgm.cheat_clear();

    // Apply any enabled cheats
    for (auto& cht : chtlist) {
        if (cht.enabled) {
            if (cht.gg.size()) {
                jgm.cheat_set(cht.gg.c_str());
            }
            else if (cht.par.size()) {
                jgm.cheat_set(cht.par.c_str());
            }
            else if (cht.raw.size()) {
                jgm.cheat_set(cht.raw.c_str());
            }
        }
    }
}

void CheatManager::load(const char *cheatpath) {
    Xml xml;

    clear();

    std::ifstream cheatfile(cheatpath, std::ifstream::in|std::ifstream::binary);

    if (cheatfile.is_open()) {
        xml.Read(cheatfile);

        if (xml.GetRoot().IsType(L"cheats")) {

            Xml::Node root(xml.GetRoot());
            Xml::Node node(root.GetFirstChild());

            for (int i = 0; i < root.NumChildren(L"cheat"); i++) {
                std::string ggcode{};
                std::string parcode{};
                std::string rawcode{};

                if (node.GetChild(L"genie")) { // Game Genie
                    std::wstring ws = node.GetChild(L"genie").GetValue();
                    ggcode = std::string(ws.begin(), ws.end());
                }

                else if (node.GetChild(L"rocky")) { // Pro Action Rocky
                    std::wstring ws = node.GetChild(L"rocky").GetValue();
                    parcode = std::string(ws.begin(), ws.end());
                }

                else if (node.GetChild(L"address")) { // Raw
                    std::wstring ws = node.GetChild(L"address").GetValue();
                    rawcode = std::string(ws.begin(), ws.end());

                    if (node.GetChild(L"value")) {
                        std::wstring vws = node.GetChild(L"value").GetValue();
                        rawcode += ' ' + std::string(vws.begin(), vws.end());
                    }

                    if (node.GetChild(L"compare")) {
                        std::wstring cws = node.GetChild(L"compare").GetValue();
                        rawcode += ' ' + std::string(cws.begin(), cws.end());
                    }
                }

                NstCheat cht = {
                    node.GetAttribute(L"enabled").IsValue(L"1"),
                    ggcode,
                    parcode,
                    rawcode,
                    node.GetChild(L"description").GetValue()
                };

                chtlist.push_back(cht);
                node = node.GetNextSibling();
            }
        }

        cheatfile.close();
        refresh();
    }
}

void CheatManager::save(const char *cheatpath) {
    // Save the cheat list
    std::ofstream cheatfile(cheatpath, std::ifstream::out|std::ifstream::binary);

    if (cheatfile.is_open()) {
        saveroot = (savexml.GetRoot());

        saveroot = savexml.Create( L"cheats" );
        saveroot.AddAttribute( L"version", L"1.0" );

        for (auto& cht : chtlist) {
            Xml::Node node(saveroot.AddChild(L"cheat"));
            node.AddAttribute(L"enabled", cht.enabled ? L"1" : L"0");

            if (cht.gg.size()) {
                std::wstring ggcode = std::wstring(cht.gg.begin(), cht.gg.end());
                node.AddChild(L"genie", ggcode.c_str());
            }

            if (cht.par.size()) {
                std::wstring parcode = std::wstring(cht.par.begin(), cht.par.end());
                node.AddChild(L"rocky", parcode.c_str());
            }

            if (cht.raw.size()) {
                std::vector<std::string> tokens;
                std::stringstream rawcode(cht.raw);
                std::string token;

                while (std::getline(rawcode, token, ' ')) {
                    tokens.push_back(token);
                }

                if ((tokens.size() == 2) || (tokens.size() == 3)) {
                    std::wstring ws = std::wstring(tokens[0].begin(), tokens[0].end());
                    node.AddChild(L"address", ws.c_str());

                    ws = std::wstring(tokens[1].begin(), tokens[1].end());
                    node.AddChild(L"value", ws.c_str());

                    if (tokens.size() == 3) {
                        ws = std::wstring(tokens[2].begin(), tokens[2].end());
                        node.AddChild(L"compare", ws.c_str());
                    }
                }
            }

            if (cht.description.size() > 0) {
                node.AddChild(L"description", cht.description.c_str());
            }
        }

        savexml.Write(saveroot, cheatfile);
        cheatfile.close();
    }
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
