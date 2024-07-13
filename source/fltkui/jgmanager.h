#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "jg/jg.h"

class JGManager {
public:
    JGManager();
    ~JGManager();

    void exec_frame();

    void reset(int hard);

    void rehash();

    void unload_game();
    void load_game(const char *filename, std::vector<uint8_t>& game);
    bool is_loaded();

    int state_load(std::string &filename);
    int state_save(std::string &filename);
    int state_qload(int slot);
    int state_qsave(int slot);

    void media_select();
    void media_insert();

    void cheat_clear();
    void cheat_set(const char *code);

    static int get_frametime();

    std::string& get_basepath();
    std::string& get_gamename();

    std::vector<jg_setting_t*>* get_settings();
    jg_setting_t* get_setting(std::string name);

    jg_coreinfo_t* get_coreinfo();
    jg_inputinfo_t* get_inputinfo(int port);
    jg_audioinfo_t* get_audioinfo();

    void set_audio_cb(jg_cb_audio_t cb);

    void data_push(uint32_t type, int port, const void *p, size_t sz);

    void setup_audio();
    void setup_video();

private:
    void set_paths();

    jg_fileinfo_t gameinfo;
    jg_pathinfo_t pathinfo;

    bool loaded{false};

    std::string basepath;
    std::string corepath;
    std::string savepath;

    std::string gamepath;
    std::string gamename;
    std::string gamefname;

    std::vector<jg_setting_t*> settings;
};
