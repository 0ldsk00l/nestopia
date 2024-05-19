#pragma once

#include <string>
#include <vector>

#include "core/api/NstApiDipSwitches.hpp"
#include "core/NstStream.hpp"
#include "core/NstXml.hpp"

#include "jgmanager.h"

typedef Nes::Core::Xml Xml;

typedef struct NstCheat {
    bool enabled{false};
    std::string gg;
    std::string par;
    std::string raw;
    std::wstring description;
} NstCheat;

// DIP Switches
void nst_dip_handle(const char *dippath);

class CheatManager {
public:
    CheatManager() = delete;
    CheatManager(JGManager& jgm);
    ~CheatManager() {}

    void clear();
    void refresh();
    void load(const char *cheatpath);
    void save(const char *cheatpath);

    std::vector<NstCheat>& get_list();

private:
    std::vector<NstCheat> chtlist;
    Xml savexml;
    Xml::Node saveroot;

    JGManager& jgm;
};
