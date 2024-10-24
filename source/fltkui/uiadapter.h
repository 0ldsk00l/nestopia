#pragma once

class UiAdapter {
public:
    UiAdapter() {}
    ~UiAdapter() {}

    static void fullscreen();
    static void fastforward(bool ff);
    static void pause();
    static void screenshot();
    static void quit();
    static void show_inputmsg(int show);

private:

};
