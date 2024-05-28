/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2024 R. Danbrook
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

#include <cstdint>
#include <cstring>

#include <FL/gl.h>

#include "videomanager.h"

#include "font.h"
#include "png.h"

namespace {

struct osdtext {
    int xpos;
    int ypos;
    char textbuf[32];
    char timebuf[6];
    int drawtext;
    bool drawtime;
    bool bg;
} osdtext;

jg_videoinfo_t *vidinfo;
uint32_t *videobuf;

}

VideoManager::VideoManager(JGManager& jgm, SettingManager& setmgr)
        : jgm(jgm), setmgr(setmgr) {
    // Initialize video
    vidinfo = jg_get_videoinfo();

    videobuf = (uint32_t*)calloc(1, vidinfo->hmax * vidinfo->wmax * sizeof(uint32_t));
    vidinfo->buf = (void*)&videobuf[0];

    set_aspect();

    int scale = setmgr.get_setting("v_scale")->val;
    dimensions.ww = (aspect * vidinfo->h * scale) + 0.5;
    dimensions.wh = (vidinfo->h * scale) + 0.5;
    dimensions.rw = dimensions.ww;
    dimensions.rh = dimensions.wh;
    dimensions.dpiscale = 1.0;
}

VideoManager::~VideoManager() {
    ogl_deinit();
    if (videobuf) {
        free(videobuf);
    }
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
        default: break;
    }
}

void VideoManager::rehash() {
    GLuint filter = setmgr.get_setting("v_linearfilter")->val ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    set_aspect();
    dimensions.rw = dimensions.ww;
    dimensions.rh = dimensions.wh;

    // Check which dimension to optimize
    if (dimensions.rh * aspect > dimensions.rw)
        dimensions.rh = dimensions.rw / aspect + 0.5;
    else if (dimensions.rw / aspect > dimensions.rh)
        dimensions.rw = dimensions.rh * aspect + 0.5;

    // Store X and Y offsets
    dimensions.xo = (dimensions.ww - dimensions.rw) / 2;
    dimensions.yo = (dimensions.wh - dimensions.rh) / 2;
}

void VideoManager::ogl_init() {
    // Generate texture for raw game output
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &gl_texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

    // The full sized source image before any clipping
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        vidinfo->wmax, vidinfo->hmax, 0, GL_BGRA, GL_UNSIGNED_BYTE,
        videobuf);

    GLuint filter = setmgr.get_setting("v_linearfilter")->val ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}

void VideoManager::ogl_deinit() {
    // Deinitialize OpenGL
    if (gl_texture_id) {
        glDeleteTextures(1, &gl_texture_id);
    }
}

void VideoManager::ogl_render() {
    // OSD Text
    if (osdtext.drawtext) {
        text_draw(osdtext.textbuf, osdtext.xpos, osdtext.ypos, osdtext.bg);
        osdtext.drawtext--;
    }

    if (osdtext.drawtime) {
        text_draw(osdtext.timebuf, 208, 218, false);
    }

    //jgrf_video_gl_refresh(); // Check for changes
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

void VideoManager::get_dimensions(int *w, int *h) {
    *w = dimensions.rw;
    *h = dimensions.rh;
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

void VideoManager::resize(int w, int h) {
    dimensions.ww = w;
    dimensions.wh = h;
    rehash();
}

/*void video_screenshot_flip(unsigned char *pixels, int width, int height, int bytes) {
    // Flip the pixels
    int rowsize = width * bytes;
    unsigned char *row = (unsigned char*)malloc(rowsize);
    unsigned char *low = pixels;
    unsigned char *high = &pixels[(height - 1) * rowsize];

    for (; low < high; low += rowsize, high -= rowsize) {
        memcpy(row, low, rowsize);
        memcpy(low, high, rowsize);
        memcpy(high, row, rowsize);
    }
    free(row);
}

void video_screenshot(const char* filename) {
    // Take a screenshot in .png format
    unsigned char *pixels;
    pixels = (unsigned char*)malloc(sizeof(unsigned char) * rendersize.w * rendersize.h * 4);

    // Read the pixels and flip them vertically
    glReadPixels(0, 0, rendersize.w, rendersize.h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    video_screenshot_flip(pixels, rendersize.w, rendersize.h, 4);

    if (filename == NULL) {
        // Set the filename
        char sshotpath[512];
        snprintf(sshotpath, sizeof(sshotpath), "%sscreenshots/%s-%ld-%d.png", nstpaths.nstdir, nstpaths.gamename, time(NULL), rand() % 899 + 100);

        // Save the file
        lodepng_encode32_file(sshotpath, (const unsigned char*)pixels, rendersize.w, rendersize.h);
        fprintf(stderr, "Screenshot: %s\n", sshotpath);
    }
    else {
        lodepng_encode32_file(filename, (const unsigned char*)pixels, rendersize.w, rendersize.h);
    }

    free(pixels);
}*/

void VideoManager::text_print(const char *text, int xpos, int ypos, int seconds, bool bg) {
    snprintf(osdtext.textbuf, sizeof(osdtext.textbuf), "%s", text);
    osdtext.xpos = xpos;
    osdtext.ypos = ypos;
    osdtext.drawtext = seconds * 60; // FIXME frametime
    osdtext.bg = bg;
}

void VideoManager::text_print_time(const char *timebuf, bool drawtime) {
    snprintf(osdtext.timebuf, sizeof(osdtext.timebuf), "%s", timebuf);
    osdtext.drawtime = drawtime;
}

void VideoManager::text_draw(const char *text, int xpos, int ypos, bool bg) {
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

void VideoManager::text_match(const char *text, int *xpos, int *ypos, int strpos) {
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
