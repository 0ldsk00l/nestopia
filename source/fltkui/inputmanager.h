#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>

#include <SDL.h>

#include "jgmanager.h"
#include "setmanager.h"

class InputManager {
public:
    InputManager() = delete;
    InputManager(JGManager& jgm, SettingManager& setmgr);
    ~InputManager();

    void reassign();

    void event(SDL_Event& evt); // Joystick
    void event(int key, bool pressed); // Keyboard
    void event(int x, int y); // Mouse

    void ui_events();

    std::vector<jg_inputinfo_t> get_inputinfo();

    std::string get_inputdef(std::string device, std::string def);
    void set_inputdef(int val);
    void clear_inputdef();

    void set_inputcfg(std::string device, std::string def, int defnum);

    bool get_lightgun() { return lightgun; }

    bool get_cfg_running() { return cfg_running; }
    void set_cfg_running(bool running);

private:

    void assign();
    void unassign();
    void remap_kb();
    void remap_js();

    void set_inputdef(SDL_Event& evt);

    JGManager &jgm;
    SettingManager &setmgr;

    std::unordered_map<int, int16_t*> jxmap;
    std::unordered_map<int, uint8_t*> jamap;
    std::unordered_map<int, uint8_t*> jbmap;
    std::unordered_map<int, uint8_t*> jhmap;
    std::unordered_map<int, uint8_t*> kbmap;
    std::unordered_map<int, int32_t*> msmap;

    bool lightgun{false};

    bool cfg_running{false};
    std::string cfg_name;
    std::string cfg_def;
    int cfg_defnum;
};
