/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2024 R. Danbrook
 * Copyright (C) 2020-2024 Rupert Carmichael
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <filesystem>
#include <sstream>
#include <fstream>

#include "videomanager.h"
#include "logdriver.h"

#include "font.h"
#include "lodepng.h"

namespace {

jg_videoinfo_t *vidinfo;
uint32_t *videobuf;

struct osdtext {
    int xpos;
    int ypos;
    char textbuf[32];
    char timebuf[6];
    int drawtext;
    bool drawtime;
    bool bg;
} osdtext;

// Dimensions
struct _dimensions {
    int ww; int wh;
    float rw; float rh;
    float xo; float yo;
    float dpiscale;
} dimensions;

} // namespace

VideoRenderer::VideoRenderer(SettingManager& setmgr)
        : setmgr(setmgr) {
}

VideoRendererLegacy::VideoRendererLegacy(SettingManager& setmgr)
        : VideoRenderer(setmgr) {
    // Generate texture for raw game output
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &gl_texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

    // The full sized source image before any clipping
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        vidinfo->wmax, vidinfo->hmax, 0, GL_BGRA, GL_UNSIGNED_BYTE,
        videobuf);

    GLuint filter = setmgr.get_setting("v_postproc")->val ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    // Ignore the empty alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

VideoRendererLegacy::~VideoRendererLegacy() {
    if (gl_texture_id) {
        glDeleteTextures(1, &gl_texture_id);
    }
}

VideoRendererModern::VideoRendererModern(SettingManager& setmgr, const std::string ver)
        : VideoRenderer(setmgr), glslver(ver) {
    // Create Vertex Array Objects
    glGenVertexArrays(1, &vao[0]);
    glGenVertexArrays(1, &vao[1]);

    // Create Vertex Buffer Objects
    glGenBuffers(1, &vbo[0]);
    glGenBuffers(1, &vbo[1]);

    // Bind buffers for vertex buffer objects
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
        vertices, GL_STATIC_DRAW);

    GLfloat vertices_out[] = {
        -1.0, -1.0, // Vertex 1 (X, Y) Left Bottom
        -1.0, 1.0,  // Vertex 2 (X, Y) Left Top
        1.0, -1.0,  // Vertex 3 (X, Y) Right Bottom
        1.0, 1.0,   // Vertex 4 (X, Y) Right Top
        0.0, 1.0,   // Texture 1 (X, Y) Left Bottom
        0.0, 0.0,   // Texture 2 (X, Y) Left Top
        1.0, 1.0,   // Texture 3 (X, Y) Right Bottom
        1.0, 0.0,   // Texture 4 (X, Y) Right Top
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_out),
        vertices_out, GL_STATIC_DRAW);

    // Bind vertex array and specify layout for first pass
    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    // Generate texture for raw game output
    glGenTextures(1, &tex[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);

    // The full sized source image before any clipping
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        vidinfo->wmax, vidinfo->hmax, 0, GL_BGRA, GL_UNSIGNED_BYTE,
        videobuf);

    // Create framebuffer
    glGenFramebuffers(1, &framebuf);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf);

    // Create texture to hold colour buffer
    glGenTextures(1, &tex[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[1]);

    // The framebuffer texture that is being rendered to offscreen, after clip
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        vidinfo->w, vidinfo->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        tex[1], 0);

    // Ignore the empty alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    shader_setup();

    ogl_refresh();
}

VideoRendererModern::~VideoRendererModern() {
    if (gl_texture_id) {
        glDeleteTextures(1, &gl_texture_id);
    }

    if (framebuf) {
        glDeleteFramebuffers(1, &framebuf);
    }

    for (size_t i = 0; i < NUMPASSES; ++i) {
        if (shaderprog[i]) glDeleteProgram(shaderprog[i]);
        if (tex[i]) glDeleteTextures(1, &tex[i]);
        if (vao[i]) glDeleteVertexArrays(1, &vao[i]);
        if (vbo[i]) glDeleteBuffers(1, &vbo[i]);
    }
}

void VideoRendererLegacy::rehash(bool reset_shaders) {
    if (reset_shaders) {
        GLuint filter = setmgr.get_setting("v_postproc")->val ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    }
}

void VideoRendererLegacy::ogl_refresh() {
    float top = (float)vidinfo->y / vidinfo->hmax;
    float bottom = 1.0 + top -
        ((vidinfo->hmax - (float)vidinfo->h) / vidinfo->hmax);
    float left = (float)vidinfo->x / vidinfo->wmax;
    float right = 1.0 + left -
        ((vidinfo->wmax -(float)vidinfo->w) / vidinfo->wmax);

    // Check if any vertices have changed since last time
    if (vertices[9] != top || vertices[11] != bottom
        || vertices[8] != left || vertices[12] != right) {
        vertices[9] = vertices[13] = top;
        vertices[11] = vertices[15] = bottom;
        vertices[8] = vertices[10] = left;
        vertices[12] = vertices[14] = right;
    }
}

void VideoRendererLegacy::ogl_render() {
    // OSD Text
    if (osdtext.drawtext) {
        text_draw(osdtext.textbuf, osdtext.xpos, osdtext.ypos, osdtext.bg);
        osdtext.drawtext--;
    }

    if (osdtext.drawtime) {
        text_draw(osdtext.timebuf, 208, 218, false);
    }

    ogl_refresh(); // Check for changes

    glPixelStorei(GL_UNPACK_ROW_LENGTH, vidinfo->p);

    // Viewport set to size of the output
    glViewport(dimensions.xo, dimensions.yo, dimensions.rw, dimensions.rh);

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

    // Render if there is new pixel data, do Black Frame Insertion otherwise
    glTexSubImage2D(GL_TEXTURE_2D,
            0,
            0, // xoffset
            0, // yoffset
            vidinfo->w + vidinfo->x, // width
            vidinfo->h + vidinfo->y, // height
            GL_BGRA, // format
            GL_UNSIGNED_BYTE, // type
        videobuf);

    glBegin(GL_QUADS);
        glTexCoord2f(vertices[10], vertices[11]);
        glVertex2f(vertices[0], vertices[1]); // Bottom Left

        glTexCoord2f(vertices[8], vertices[9]);
        glVertex2f(vertices[2], vertices[3]); // Top Left

        glTexCoord2f(vertices[12], vertices[13]);
        glVertex2f(vertices[6], vertices[7]); // Top Right

        glTexCoord2f(vertices[14], vertices[15]);
        glVertex2f(vertices[4], vertices[5]); // Bottom Right
    glEnd();
}

void VideoRendererModern::rehash(bool reset_shaders) {
    if (reset_shaders) {
        shader_setup();
    }

    // Update uniforms for post-processing
    glUniform4f(glGetUniformLocation(shaderprog[1], "sourceSize"),
        (float)vidinfo->w, (float)vidinfo->h,
        1.0/(float)vidinfo->w, 1.0/(float)vidinfo->h);
    glUniform4f(glGetUniformLocation(shaderprog[1], "targetSize"),
        dimensions.rw, dimensions.rh,
        1.0/dimensions.rw, 1.0/dimensions.rh);
}

GLuint VideoRendererModern::shader_create(const std::string& vs, const std::string& fs) {
    // If the binary is run from the source directory, shader path is PWD
    std::string shaderpath{};
    if (std::filesystem::exists(std::filesystem::path{"shaders/default.vs"})) {
        shaderpath = std::string(std::getenv("PWD")) + "/shaders/";
    }
    else {
        shaderpath = std::string(NST_DATADIR) + "/shaders/";
    }

    auto shader_load = [this](const std::string filename) -> std::string {
        std::ifstream shader_file(filename);
        if (shader_file.is_open()) {
            std::stringstream buffer;
            buffer << shader_file.rdbuf();
            shader_file.close();
            return glslver + buffer.str();
        }
        return {};
    };

    std::string vssrc = shader_load(shaderpath + vs);
    const GLchar *vsrc = vssrc.c_str();

    std::string fssrc = shader_load(shaderpath + fs);
    const GLchar *fsrc = fssrc.c_str();

    // Create and compile the vertex shader
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vsrc, NULL);
    glCompileShader(vshader);

    // Test if the shader compiled
    GLint err;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &err);
    if (err == GL_FALSE) {
        char shaderlog[1024];
        glGetShaderInfoLog(vshader, 1024, NULL, shaderlog);
        LogDriver::log(LogLevel::Warn, "Vertex shader: " + std::string(shaderlog));
    }

    // Create and compile the fragment shader
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fsrc, NULL);
    glCompileShader(fshader);

    // Test if the fragment shader compiled
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &err);
    if (err == GL_FALSE) {
        char shaderlog[1024];
        glGetShaderInfoLog(fshader, 1024, NULL, shaderlog);
        LogDriver::log(LogLevel::Warn, "Fragment shader: " + std::string(shaderlog));
    }

    // Create the shader program
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vshader);
    glAttachShader(prog, fshader);
    glLinkProgram(prog);

    // Clean up fragment and vertex shaders
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    // Return the successfully linked shader program
    return prog;
}

void VideoRendererModern::shader_setup(void) {
    for (size_t i = 0; i < NUMPASSES; ++i) {
        if (shaderprog[i]) {
            glDeleteProgram(shaderprog[i]);
        }
        texfilter[i] = GL_NEAREST;
    }

    // Create the shader program for the first pass (clipping)
    shaderprog[0] = shader_create("default.vs", "default.fs");

    GLint posattrib = glGetAttribLocation(shaderprog[0], "position");
    glEnableVertexAttribArray(posattrib);
    glVertexAttribPointer(posattrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint texattrib = glGetAttribLocation(shaderprog[0], "vtxCoord");
    glEnableVertexAttribArray(texattrib);
    glVertexAttribPointer(texattrib, 2, GL_FLOAT, GL_FALSE,
                          0, (void*)(8 * sizeof(GLfloat)));

    // Set up uniform for input texture
    glUseProgram(shaderprog[0]);
    glUniform1i(glGetUniformLocation(shaderprog[0], "source"), 0);

    // Set up the post-processing shader
    switch (setmgr.get_setting("v_postproc")->val) {
        default: case 0: { // Nearest Neighbour
            shaderprog[1] = shader_create("default.vs", "default.fs");
            break;
        }
        case 1: { // Linear
            shaderprog[1] = shader_create("default.vs", "default.fs");
            texfilter[1] = GL_LINEAR;
            break;
        }
        case 2: { // Sharp Bilinear
            shaderprog[1] = shader_create("default.vs", "sharp-bilinear.fs");
            texfilter[0] = GL_LINEAR;
            texfilter[1] = GL_LINEAR;
            break;
        }
        case 3: { // CRTea
            shaderprog[1] = shader_create("default.vs", "crtea.fs");
            break;
        }
        case 4: { // MMPX
            shaderprog[1] = shader_create("default.vs", "mmpx.fs");
            break;
        }
        case 5: { // Omniscale
            shaderprog[1] = shader_create("default.vs", "omniscale.fs");
            break;
        }
    }

    // Set texture parameters for input texture
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texfilter[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texfilter[0]);

    // Bind vertex array and specify layout for second pass
    glBindVertexArray(vao[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

    GLint posattrib_out = glGetAttribLocation(shaderprog[1], "position");
    glEnableVertexAttribArray(posattrib_out);
    glVertexAttribPointer(posattrib_out, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint texattrib_out = glGetAttribLocation(shaderprog[1], "vtxCoord");
    glEnableVertexAttribArray(texattrib_out);
    glVertexAttribPointer(texattrib_out, 2, GL_FLOAT, GL_FALSE,
                          0, (void*)(8 * sizeof(GLfloat)));

    // Set up uniforms for post-processing texture
    glUseProgram(shaderprog[1]);

    glUniform1i(glGetUniformLocation(shaderprog[1], "source"), 0);
    glUniform4f(glGetUniformLocation(shaderprog[1], "sourceSize"),
                                     (float)vidinfo->w, (float)vidinfo->h,
                                     1.0/(float)vidinfo->w, 1.0/(float)vidinfo->h);
    glUniform4f(glGetUniformLocation(shaderprog[1], "targetSize"),
                                     dimensions.rw, dimensions.rh,
                                     1.0/dimensions.rw, 1.0/dimensions.rh);

    // Settings for CRT
    glUniform1i(glGetUniformLocation(shaderprog[1], "masktype"),
                setmgr.get_setting("s_crtmasktype")->val);
    glUniform1f(glGetUniformLocation(shaderprog[1], "maskstr"),
                setmgr.get_setting("s_crtmaskstr")->val / 10.0);
    glUniform1f(glGetUniformLocation(shaderprog[1], "scanstr"),
                setmgr.get_setting("s_crtscanstr")->val / 10.0);
    glUniform1f(glGetUniformLocation(shaderprog[1], "sharpness"),
                float(setmgr.get_setting("s_crtsharp")->val));
    glUniform1f(glGetUniformLocation(shaderprog[1], "curve"),
                setmgr.get_setting("s_crtcurve")->val / 100.0);
    glUniform1f(glGetUniformLocation(shaderprog[1], "corner"),
                setmgr.get_setting("s_crtcorner")->val ?
                float(setmgr.get_setting("s_crtcorner")->val) : -3.0);
    glUniform1f(glGetUniformLocation(shaderprog[1], "tcurve"),
                setmgr.get_setting("s_crttcurve")->val / 10.0);

    // Set parameters for output texture
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texfilter[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texfilter[1]);
}

void VideoRendererModern::ogl_render() {
    // OSD Text
    if (osdtext.drawtext) {
        text_draw(osdtext.textbuf, osdtext.xpos, osdtext.ypos, osdtext.bg);
        osdtext.drawtext--;
    }

    if (osdtext.drawtime) {
        text_draw(osdtext.timebuf, 208, 218, false);
    }

    ogl_refresh(); // Check for changes

    // Viewport set to size of the input pixel array
    glViewport(0, 0, vidinfo->w, vidinfo->h);

    // Make sure first pass shader program is active
    glUseProgram(shaderprog[0]);

    // Bind user-created framebuffer and draw scene onto it
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf);
    glBindVertexArray(vao[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);

    // Render if there is new pixel data, do Black Frame Insertion otherwise
    glTexSubImage2D(GL_TEXTURE_2D,
            0,
            0, // xoffset
            0, // yoffset
            vidinfo->w + vidinfo->x, // width
            vidinfo->h + vidinfo->y, // height
            GL_BGRA, // format
            GL_UNSIGNED_BYTE, // type
        videobuf);

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw a rectangle from the 2 triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Now deal with the actual image to be output
    // Viewport adjusted for output
    glViewport(dimensions.xo, dimensions.yo, dimensions.rw, dimensions.rh);

    // Make sure second pass shader program is active
    glUseProgram(shaderprog[1]);

    // Bind default framebuffer and draw contents of user framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(vao[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[1]);

    // Clear the screen to black again
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw framebuffer contents
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void VideoRendererModern::ogl_refresh() {
    float top = (float)vidinfo->y / vidinfo->hmax;
    float bottom = 1.0 + top -
        ((vidinfo->hmax - (float)vidinfo->h) / vidinfo->hmax);
    float left = (float)vidinfo->x / vidinfo->wmax;
    float right = 1.0 + left -
        ((vidinfo->wmax -(float)vidinfo->w) / vidinfo->wmax);

    // Check if any vertices have changed since last time
    if (vertices[9] != top || vertices[11] != bottom
        || vertices[8] != left || vertices[12] != right) {
        vertices[9] = vertices[13] = top;
        vertices[11] = vertices[15] = bottom;
        vertices[8] = vertices[10] = left;
        vertices[12] = vertices[14] = right;
    }
    else {
        return;
    }

    // Bind the VAO/VBO for the offscreen texture, update with new vertex data
    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Resize the offscreen texture
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        vidinfo->wmax, vidinfo->hmax, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    // Set row length
    glUseProgram(shaderprog[0]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, vidinfo->p);

    // Resize the output texture
    glUseProgram(shaderprog[1]);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        vidinfo->w, vidinfo->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    // Update uniforms for post-processing
    glUniform4f(glGetUniformLocation(shaderprog[1], "sourceSize"),
        (float)vidinfo->w, (float)vidinfo->h,
        1.0/(float)vidinfo->w, 1.0/(float)vidinfo->h);
    glUniform4f(glGetUniformLocation(shaderprog[1], "targetSize"),
        dimensions.rw, dimensions.rh,
        1.0/dimensions.rw, 1.0/dimensions.rh);
}

void VideoRenderer::text_print(const char *text, int xpos, int ypos, int seconds, bool bg) {
    snprintf(osdtext.textbuf, sizeof(osdtext.textbuf), "%s", text);
    osdtext.xpos = xpos;
    osdtext.ypos = ypos;
    osdtext.drawtext = seconds * 60; // FIXME frametime
    osdtext.bg = bg;
}

void VideoRenderer::text_print_time(const char *timebuf, bool drawtime) {
    snprintf(osdtext.timebuf, sizeof(osdtext.timebuf), "%s", timebuf);
    osdtext.drawtime = drawtime;
}

void VideoRenderer::text_draw(const char *text, int xpos, int ypos, bool bg) {
    // Draw text on screen
    uint32_t w = 0xc0c0c0c0; // "White", actually Grey
    uint32_t b = 0x00000000; // Black
    uint32_t g = 0x00358570; // Nestopia UE Green
    uint32_t d = 0x00255f65; // Nestopia UE Dark Green

    int numchars = strlen(text);

    int letterypos;
    int letterxpos;
    int letternum = 0;

    if (bg) { // Draw background borders
        for (int i = 0; i < numchars * 8; i++) { // Rows above and below
            videobuf[(xpos + i) + ((ypos - 1) * vidinfo->wmax)] = g;
            videobuf[(xpos + i) + ((ypos + 8) * vidinfo->wmax)] = g;
        }
        for (int i = 0; i < 8; i++) { // Columns on both sides
            videobuf[(xpos - 1) + ((ypos + i) * vidinfo->wmax)] = g;
            videobuf[(xpos + (numchars * 8)) + ((ypos + i) * vidinfo->wmax)] = g;
        }
    }

    // FIXME this code is terrible
    for (int tpos = 0; tpos < (8 * numchars); tpos+=8) {
        text_match(text, &letterxpos, &letterypos, letternum);
        for (int row = 0; row < 8; row++) { // Draw Rows
            for (int col = 0; col < 8; col++) { // Draw Columns
                switch (nesfont[row + letterypos][col + letterxpos]) {
                    case '.':
                        videobuf[xpos + ((ypos + row) * vidinfo->wmax) + (col + tpos)] = w;
                        break;

                    case '+':
                        videobuf[xpos + ((ypos + row) * vidinfo->wmax) + (col + tpos)] = g;
                        break;

                    default:
                        if (bg) { videobuf[xpos + ((ypos + row) * vidinfo->wmax) + (col + tpos)] = d; }
                        break;
                }
            }
        }
        letternum++;
    }
}

void VideoRenderer::text_match(const char *text, int *xpos, int *ypos, int strpos) {
    // Match letters to draw on screen
    switch (text[strpos]) {
        case ' ': *xpos = 0; *ypos = 0; break;
        case '!': *xpos = 8; *ypos = 0; break;
        case '"': *xpos = 16; *ypos = 0; break;
        case '#': *xpos = 24; *ypos = 0; break;
        case '$': *xpos = 32; *ypos = 0; break;
        case '%': *xpos = 40; *ypos = 0; break;
        case '&': *xpos = 48; *ypos = 0; break;
        case '\'': *xpos = 56; *ypos = 0; break;
        case '(': *xpos = 64; *ypos = 0; break;
        case ')': *xpos = 72; *ypos = 0; break;
        case '*': *xpos = 80; *ypos = 0; break;
        case '+': *xpos = 88; *ypos = 0; break;
        case ',': *xpos = 96; *ypos = 0; break;
        case '-': *xpos = 104; *ypos = 0; break;
        case '.': *xpos = 112; *ypos = 0; break;
        case '/': *xpos = 120; *ypos = 0; break;
        case '0': *xpos = 0; *ypos = 8; break;
        case '1': *xpos = 8; *ypos = 8; break;
        case '2': *xpos = 16; *ypos = 8; break;
        case '3': *xpos = 24; *ypos = 8; break;
        case '4': *xpos = 32; *ypos = 8; break;
        case '5': *xpos = 40; *ypos = 8; break;
        case '6': *xpos = 48; *ypos = 8; break;
        case '7': *xpos = 56; *ypos = 8; break;
        case '8': *xpos = 64; *ypos = 8; break;
        case '9': *xpos = 72; *ypos = 8; break;
        case ':': *xpos = 80; *ypos = 8; break;
        case ';': *xpos = 88; *ypos = 8; break;
        case '<': *xpos = 96; *ypos = 8; break;
        case '=': *xpos = 104; *ypos = 8; break;
        case '>': *xpos = 112; *ypos = 8; break;
        case '?': *xpos = 120; *ypos = 8; break;
        case '@': *xpos = 0; *ypos = 16; break;
        case 'A': *xpos = 8; *ypos = 16; break;
        case 'B': *xpos = 16; *ypos = 16; break;
        case 'C': *xpos = 24; *ypos = 16; break;
        case 'D': *xpos = 32; *ypos = 16; break;
        case 'E': *xpos = 40; *ypos = 16; break;
        case 'F': *xpos = 48; *ypos = 16; break;
        case 'G': *xpos = 56; *ypos = 16; break;
        case 'H': *xpos = 64; *ypos = 16; break;
        case 'I': *xpos = 72; *ypos = 16; break;
        case 'J': *xpos = 80; *ypos = 16; break;
        case 'K': *xpos = 88; *ypos = 16; break;
        case 'L': *xpos = 96; *ypos = 16; break;
        case 'M': *xpos = 104; *ypos = 16; break;
        case 'N': *xpos = 112; *ypos = 16; break;
        case 'O': *xpos = 120; *ypos = 16; break;
        case 'P': *xpos = 0; *ypos = 24; break;
        case 'Q': *xpos = 8; *ypos = 24; break;
        case 'R': *xpos = 16; *ypos = 24; break;
        case 'S': *xpos = 24; *ypos = 24; break;
        case 'T': *xpos = 32; *ypos = 24; break;
        case 'U': *xpos = 40; *ypos = 24; break;
        case 'V': *xpos = 48; *ypos = 24; break;
        case 'W': *xpos = 56; *ypos = 24; break;
        case 'X': *xpos = 64; *ypos = 24; break;
        case 'Y': *xpos = 72; *ypos = 24; break;
        case 'Z': *xpos = 80; *ypos = 24; break;
        case '[': *xpos = 88; *ypos = 24; break;
        case '\\': *xpos = 96; *ypos = 24; break;
        case ']': *xpos = 104; *ypos = 24; break;
        case '^': *xpos = 112; *ypos = 24; break;
        case '_': *xpos = 120; *ypos = 24; break;
        case '`': *xpos = 0; *ypos = 32; break;
        case 'a': *xpos = 8; *ypos = 32; break;
        case 'b': *xpos = 16; *ypos = 32; break;
        case 'c': *xpos = 24; *ypos = 32; break;
        case 'd': *xpos = 32; *ypos = 32; break;
        case 'e': *xpos = 40; *ypos = 32; break;
        case 'f': *xpos = 48; *ypos = 32; break;
        case 'g': *xpos = 56; *ypos = 32; break;
        case 'h': *xpos = 64; *ypos = 32; break;
        case 'i': *xpos = 72; *ypos = 32; break;
        case 'j': *xpos = 80; *ypos = 32; break;
        case 'k': *xpos = 88; *ypos = 32; break;
        case 'l': *xpos = 96; *ypos = 32; break;
        case 'm': *xpos = 104; *ypos = 32; break;
        case 'n': *xpos = 112; *ypos = 32; break;
        case 'o': *xpos = 120; *ypos = 32; break;
        case 'p': *xpos = 0; *ypos = 40; break;
        case 'q': *xpos = 8; *ypos = 40; break;
        case 'r': *xpos = 16; *ypos = 40; break;
        case 's': *xpos = 24; *ypos = 40; break;
        case 't': *xpos = 32; *ypos = 40; break;
        case 'u': *xpos = 40; *ypos = 40; break;
        case 'v': *xpos = 48; *ypos = 40; break;
        case 'w': *xpos = 56; *ypos = 40; break;
        case 'x': *xpos = 64; *ypos = 40; break;
        case 'y': *xpos = 72; *ypos = 40; break;
        case 'z': *xpos = 80; *ypos = 40; break;
        case '{': *xpos = 88; *ypos = 40; break;
        case '|': *xpos = 96; *ypos = 40; break;
        case '}': *xpos = 104; *ypos = 40; break;
        case '~': *xpos = 112; *ypos = 40; break;
        //case ' ': *xpos = 120; *ypos = 40; break; // Triangle
        default: *xpos = 0; *ypos = 0; break;
    }
}

void VideoRenderer::get_pixeldata(std::vector<uint8_t>& pixels) {
    int w = dimensions.rw;
    int h = dimensions.rh;

    // Remove any on-screen text before grabbing pixel data
    int drawtext = osdtext.drawtext;
    bool drawtime = osdtext.drawtime;
    osdtext.drawtext = osdtext.drawtime = 0;
    ogl_render();
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Put any text back on screen
    osdtext.drawtext = drawtext;
    osdtext.drawtime = drawtime;


    // Flip the image
    for (int line = 0; line != h / 2; ++line) {
        std::swap_ranges(pixels.begin() + sizeof(uint32_t) * w * line,
                         pixels.begin() + sizeof(uint32_t) * w * (line + 1),
                         pixels.begin() + sizeof(uint32_t) * w * (h - line - 1));
    }
}

VideoManager::VideoManager(JGManager& jgm, SettingManager& setmgr)
        : jgm(jgm), setmgr(setmgr) {
    // Initialize video
    vidinfo = jg_get_videoinfo();

    videobuf = (uint32_t*)calloc(1, vidinfo->hmax * vidinfo->wmax * sizeof(uint32_t));
    vidinfo->buf = (void*)&videobuf[0];
}

VideoManager::~VideoManager() {
    if (videobuf) {
        free(videobuf);
    }
}

void VideoManager::renderer_init() {
    if (setmgr.get_setting("v_renderer")->val) {
        renderer = new VideoRendererLegacy(setmgr);
        LogDriver::log(LogLevel::Debug, "Renderer: Legacy OpenGL");
    }
    else {
        const std::string ver_core{"#version 140\n"};
        const std::string ver_es{"#version 300 es\n"};

        // Build a test vertex shader to check version compatibility
        auto shadertest = [](const std::string ver) -> bool {
            GLuint nullshader = glCreateShader(GL_VERTEX_SHADER);
            std::string nullsrc = ver + "void main() {}\n";
            const char *c_str = nullsrc.c_str();

            glShaderSource(nullshader, 1, &c_str, NULL);
            glCompileShader(nullshader);

            GLint err;
            glGetShaderiv(nullshader, GL_COMPILE_STATUS, &err);
            glDeleteShader(nullshader);

            return err != GL_FALSE;
        };

        if (shadertest(ver_core)) {
            renderer = new VideoRendererModern(setmgr, ver_core);
            LogDriver::log(LogLevel::Debug, "Renderer: OpenGL 3.1");
        }
        else if (shadertest(ver_es)) {
            renderer = new VideoRendererModern(setmgr, ver_es);
            LogDriver::log(LogLevel::Debug, "Renderer: OpenGL ES 3.0");
        }
        else {
            renderer = new VideoRendererLegacy(setmgr);
            LogDriver::log(LogLevel::Warn, "Renderer: Legacy OpenGL (Fallback)");
        }
    }
}

void VideoManager::renderer_deinit() {
    if (renderer) {
        delete renderer;
        renderer = nullptr;
    }
}

void VideoManager::render() {
    renderer->ogl_render();
}

void VideoManager::get_dimensions(int *w, int *h) {
    *w = dimensions.rw;
    *h = dimensions.rh;
}

void VideoManager::set_dimensions() {
    // Try to guess the correct video parameters if no game is loaded
    if (!jgm.is_loaded()) {
        int t = jgm.get_setting("overscan_t")->val;
        int b = jgm.get_setting("overscan_b")->val;
        int l = jgm.get_setting("overscan_l")->val;
        int r = jgm.get_setting("overscan_r")->val;

        vidinfo->w = vidinfo->w - (l + r);
        vidinfo->h = vidinfo->h - (t + b);
        vidinfo->x = l;
        vidinfo->y = t;
        vidinfo->aspect = (vidinfo->w * 8.0 / 7.0) / vidinfo->h; // NTSC
    }

    set_aspect();

    int scale = setmgr.get_setting("v_scale")->val;
    dimensions.ww = (aspect * vidinfo->h * scale) + 0.5;
    dimensions.wh = (vidinfo->h * scale) + 0.5;
    dimensions.rw = dimensions.ww;
    dimensions.rh = dimensions.wh;
    dimensions.dpiscale = 1.0;
}

void VideoManager::set_dpiscale(float dpiscale) {
    dimensions.dpiscale = dpiscale;
}

// FIXME maybe use std::tuple here
void VideoManager::get_scaled_coords(int x, int y, int *xcoord, int *ycoord) {
    float xscale = dimensions.rw / (vidinfo->aspect * vidinfo->h) / dimensions.dpiscale;
    float yscale = dimensions.rh / vidinfo->h / dimensions.dpiscale;
    float xo = dimensions.xo / dimensions.dpiscale;
    float yo = dimensions.yo / dimensions.dpiscale;
    *xcoord = (x - xo) / ((vidinfo->aspect * vidinfo->h * xscale)/(float)vidinfo->w);
    *ycoord = ((y - yo) / yscale) + vidinfo->y;
}

void VideoManager::rehash(bool reset_shaders) {
    renderer->rehash(reset_shaders);

    set_aspect();
    dimensions.rw = dimensions.ww;
    dimensions.rh = dimensions.wh;

    // Check which dimension to optimize
    if (dimensions.rh * aspect > dimensions.rw) {
        dimensions.rh = dimensions.rw / aspect + 0.5;
    }
    else if (dimensions.rw / aspect > dimensions.rh) {
        dimensions.rw = dimensions.rh * aspect + 0.5;
    }

    // Store X and Y offsets
    dimensions.xo = (dimensions.ww - dimensions.rw) / 2;
    dimensions.yo = (dimensions.wh - dimensions.rh) / 2;
}

void VideoManager::resize(int w, int h) {
    if (!renderer) {
        return;
    }
    dimensions.ww = w * dimensions.dpiscale;
    dimensions.wh = h * dimensions.dpiscale;
    rehash();
}

void VideoManager::set_aspect() {
    switch (setmgr.get_setting("v_aspect")->val) {
        case 0:
            aspect = vidinfo->aspect;
            break;
        case 1:
            if (jgm.get_setting("ntsc_filter")->val) {
                aspect = 301/(double)vidinfo->h;
            }
            else {
                aspect = vidinfo->w/(double)vidinfo->h;
            }
            break;
        case 2:
            aspect = 4.0/3.0;
            break;
        case 3:
            aspect = 5.0/4.0;
            break;
        default: break;
    }
}

void VideoManager::screenshot(std::string& filename) {
    int w = dimensions.rw;
    int h = dimensions.rh;
    std::vector<uint8_t> pixels(sizeof(uint32_t) * w * h);
    renderer->get_pixeldata(pixels);
    lodepng_encode32_file(filename.c_str(), pixels.data(), w, h);
}
