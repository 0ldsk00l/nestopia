#pragma once

#include "jgmanager.h"
#include "setmanager.h"

constexpr size_t BUFSIZE = 4800;

class AudioManager {
public:
    AudioManager() = delete;
    AudioManager(JGManager& jgm, SettingManager& setmgr);
    ~AudioManager();

    void set_speed(int speed);
    void pause(bool p);
    void mute(bool m);

    inline int16_t dequeue();
    static void queue(size_t in_size);
    static void null_queue(size_t in_size);

    void rehash();

private:
    JGManager &jgm;
    SettingManager &setmgr;
};
