#pragma once

class UiAdapter {
public:
    UiAdapter() {}
    ~UiAdapter() {}

    void fullscreen();
    void fastforward(bool ff);
    void pause();
    void screenshot();

    void show_msgbox(bool show);

private:

};
