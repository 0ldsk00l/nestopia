#ifndef _CHEATS_H_
#define _CHEATS_H_

#include <string>
#include <vector>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiCheats.hpp"
#include "core/api/NstApiDipSwitches.hpp"
#include "core/NstStream.hpp"
#include "core/NstXml.hpp"

using namespace Nes::Api;

typedef Nes::Core::Xml Xml;

typedef struct NstCheat {
    bool enabled;
    std::wstring gg;
    std::wstring par;
    unsigned short address;
    unsigned char value;
    unsigned char compare;
    std::wstring description;
} NstCheat;

void nst_cheats_init(const char *cheatpath);
void nst_cheats_refresh();
void nst_cheats_code_gg_add(const std::wstring data);
void nst_cheats_code_par_add(const std::wstring data);

// DIP Switches
void nst_dip_handle(const char *dippath);

#endif
