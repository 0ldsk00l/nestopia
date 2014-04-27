#ifndef _CHEATS_H_
#define _CHEATS_H_

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiCheats.hpp"
#include "core/api/NstApiDipSwitches.hpp"
#include "core/NstStream.hpp"
#include "core/NstXml.hpp"

using namespace Nes::Api;

typedef Nes::Core::Xml Xml;

void cheats_init();
void cheats_code_gg_add(const wchar_t *data);
void cheats_code_par_add(const wchar_t *data);
void cheats_code_raw_add(Xml::Node node);

// DIP Switches
void dip_handle();

#endif
