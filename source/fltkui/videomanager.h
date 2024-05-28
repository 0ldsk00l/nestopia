#pragma once

#include "setmanager.h"
#include "jgmanager.h"

class VideoManager {
public:
    VideoManager() = delete;
    VideoManager(JGManager& jgm, SettingManager& setmgr);
    ~VideoManager();

    void get_dimensions(int *w, int *h);
    void get_scaled_coords(int x, int y, int *xcoord, int *ycoord);

    void rehash();
    void resize(int w, int h);
    void set_aspect();

    void ogl_init();
    void ogl_deinit();
    void ogl_render();

    static void text_print(const char *text, int xpos, int ypos, int seconds, bool bg);
    static void text_print_time(const char *timebuf, bool drawtime);
    static void text_draw(const char *text, int xpos, int ypos, bool bg);
    static void text_match(const char *text, int *xpos, int *ypos, int strpos);

private:
    JGManager &jgm;
    SettingManager &setmgr;

    double aspect{1.0};

    // Triangle and Texture vertices
    float vertices[16]{
        -1.0, -1.0, // Vertex 1 (X, Y) Left Bottom
        -1.0, 1.0,  // Vertex 2 (X, Y) Left Top
        1.0, -1.0,  // Vertex 3 (X, Y) Right Bottom
        1.0, 1.0,   // Vertex 4 (X, Y) Right Top
        0.0, 0.0,   // Texture 2 (X, Y) Left Top
        0.0, 1.0,   // Texture 1 (X, Y) Left Bottom
        1.0, 0.0,   // Texture 4 (X, Y) Right Top
        1.0, 1.0,   // Texture 3 (X, Y) Right Bottom
    };

    unsigned int gl_texture_id{0};

    // Dimensions
    struct _dimensions {
        int ww; int wh;
        float rw; float rh;
        float xo; float yo;
        float dpiscale;
    } dimensions;
};
