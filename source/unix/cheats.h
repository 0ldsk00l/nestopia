#ifndef _CHEATS_H_
#define _CHEATS_H_

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiCheats.hpp"
#include "core/NstStream.hpp"
#include "core/NstXml.hpp"

using namespace Nes::Api;

void cheats_init();
void cheats_code_gg_add(const wchar_t *data);

#endif
