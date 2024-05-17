#pragma once

#include <string>
#include <vector>

void fltkui_archive_load_file(const char *filename, std::string& arcname, std::vector<uint8_t>& game);
bool fltkui_archive_select(const char *filename, std::string& arcname);
