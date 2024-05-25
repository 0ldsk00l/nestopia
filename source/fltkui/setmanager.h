#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "jgmanager.h"

// Extend the jg_settingflag value to use the top bit for frontend-only settings
constexpr unsigned FLAG_FRONTEND = 0x8000000;

class SettingManager {
public:
    SettingManager();
    ~SettingManager() {}

    void read(JGManager& jgm);
    void write(JGManager& jgm);

    std::vector<jg_setting_t*>* get_settings();
    jg_setting_t* get_setting(std::string name);

    std::string& get_input(std::string name, std::string def);
    void set_input(std::string name, std::string def, std::string val);

private:

    std::string confpath;

    std::vector<jg_setting_t*> settings;
};
