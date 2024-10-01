#pragma once

#include <string>

#include <epoxy/gl.h>

#include "setmanager.h"
#include "jgmanager.h"

class VideoRenderer {
public:
    VideoRenderer() = delete;
    VideoRenderer(SettingManager& setmgr);
    virtual ~VideoRenderer() {};

    virtual void ogl_render() = 0;
    virtual void ogl_refresh() = 0;

    virtual void rehash(bool reset_shaders = false) = 0;

    static void text_print(const char *text, int xpos, int ypos, int seconds, bool bg);

    void get_pixeldata(std::vector<uint8_t>& pixeldata);

protected:
    static void text_draw(const char *text, int xpos, int ypos, bool bg);
    static void text_print_time(const char *timebuf, bool drawtime);
    static void text_match(const char *text, int *xpos, int *ypos, int strpos);

    SettingManager &setmgr;

    GLuint gl_texture_id{0};

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
};

class VideoRendererLegacy : public VideoRenderer {
public:
    VideoRendererLegacy() = delete;
    VideoRendererLegacy(SettingManager& setmgr);
    ~VideoRendererLegacy() override;

    void ogl_render() override;
    void ogl_refresh() override;

    void rehash(bool reset_shaders = false) override;
};

class VideoRendererModern : public VideoRenderer {
public:
    VideoRendererModern() = delete;
    VideoRendererModern(SettingManager& setmgr, const std::string ver);
    ~VideoRendererModern() override;

    void ogl_render() override;
    void ogl_refresh() override;

    void rehash(bool reset_shaders = false) override;

private:
    GLuint shader_create(const std::string& vs, const std::string& fs);
    void shader_setup(void);

    static constexpr size_t NUMPASSES = 2;

    std::string glslver{};

    GLuint vao[NUMPASSES];
    GLuint vbo[NUMPASSES];
    GLuint shaderprog[NUMPASSES];
    GLuint tex[NUMPASSES];
    GLuint texfilter[NUMPASSES];
    GLuint framebuf; // Framebuffer for rendering offscreen
};

class VideoManager {
public:
    VideoManager() = delete;
    VideoManager(JGManager& jgm, SettingManager& setmgr);
    ~VideoManager();

    void get_dimensions(int *w, int *h);
    void set_dimensions();
    void set_dpiscale(float dpiscale);

    void get_scaled_coords(int x, int y, int *xcoord, int *ycoord);

    void rehash(bool reset_shaders = false);
    void resize(int w, int h);
    void set_aspect();

    void renderer_init();
    void renderer_deinit();
    void render();

    void screenshot(std::string& sspath);

private:
    JGManager &jgm;
    SettingManager &setmgr;

    VideoRenderer *renderer{nullptr};

    double aspect{1.0};
};
