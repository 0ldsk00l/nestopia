/*
zlib License

Copyright (c) 2020-2022 Rupert Carmichael

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef JG_H
#define JG_H

#ifdef __cplusplus
  #include <cstddef>
  #if __cplusplus >= 201103L
    #include <cstdint>
  #else
    #include <stdint.h>
  #endif
extern "C" {
#else
  #include <stddef.h>
  #include <stdint.h>
#endif

// Semantic Versioning
#define JG_VERSION_MAJOR 1
#define JG_VERSION_MINOR 0
#define JG_VERSION_PATCH 0

// Enums
enum jg_datatype {
    JG_DATA_RAW,    /**< Raw data */
    JG_DATA_AUDIO,  /**< Audio samples */
    JG_DATA_VIDEO   /**< Static or streaming visual data */
};

enum jg_hints {
    JG_HINT_AUDIO_INTERNAL  = 0x00000001,   /**< Core internally allocates
                                                audio buffer */
    JG_HINT_VIDEO_INTERNAL  = 0x00000002,   /**< Core internally allocates
                                                video buffer */
    JG_HINT_VIDEO_FLIP_H    = 0x00000004,   /**< Screen flipped horizontally */
    JG_HINT_VIDEO_FLIP_V    = 0x00000008,   /**< Screen flipped vertically */
    JG_HINT_VIDEO_ROTATE_L  = 0x00000010,   /**< Screen rotated 90 degrees
                                                left */
    JG_HINT_VIDEO_ROTATE_R  = 0x00000020,   /**< Screen rotated 90 degrees
                                                right */
    JG_HINT_VIDEO_PRESCALED = 0x00000040,   /**< Video internally scaled by
                                                core */
    JG_HINT_MEDIA_ARCHIVED  = 0x00000080,   /**< Core uses archived/compressed
                                                content */
    JG_HINT_INPUT_AUDIO     = 0x00000100,   /**< Core supports audio input
                                                (microphone) */
    JG_HINT_INPUT_VIDEO     = 0x00000200    /**< Core supports visual input
                                                (camera, scanner) */
};

enum jg_inputtype {
    JG_INPUT_CONTROLLER,    /**< Emulated Controller or Gamepad input type */
    JG_INPUT_GUN,           /**< Emulated Light Gun input type */
    JG_INPUT_KEYBOARD,      /**< Emulated Keyboard input type */
    JG_INPUT_POINTER,       /**< Emulated Pointer input type: mice, trackballs,
                                drawing tablets, light sensors */
    JG_INPUT_SPINNER,       /**< Emulated Spinner input type */
    JG_INPUT_TOUCH,         /**< Emulated Touchscreen or Touchpad input type */
    JG_INPUT_EXTERNAL       /**< Emulated External input type: Coin slots,
                                system/hardware buttons */
};

enum jg_loglevel {
    JG_LOG_DBG,     /**< Debug: verbose logging */
    JG_LOG_INF,     /**< Info: general purpose logging */
    JG_LOG_WRN,     /**< Warning: warnings about non-critical errors */
    JG_LOG_ERR,     /**< Error: critical errors requiring program exit */
    JG_LOG_SCR      /**< Screen: log to on-screen display */
};

enum jg_pixfmt {
    JG_PIXFMT_XRGB8888, /**< 32-bit pixels with bits 24-31 unused, bits 16-23
                            for Red, bits 8-15 for Green, and bits 0-7 for
                            Blue */
    JG_PIXFMT_XBGR8888, /**< 32-bit pixels with bits 24-31 unused, bits 16-23
                            for Blue, bits 8-15 for Green, and bits 0-7 for
                            Red */
    JG_PIXFMT_RGBX5551, /**< 16-bit pixels with bits 11-15 for Red, bits 6-10
                            for Green, bits 1-5 for Blue, and bit 0 unused */
    JG_PIXFMT_RGB565    /**< 16-bit pixels with bits 11-15 for Red, bits 5-10 
                            for Green, and bits 0-4 for Blue */
};

enum jg_sampfmt {
    JG_SAMPFMT_INT16,   /**< 16-bit Signed Integer audio samples */
    JG_SAMPFMT_FLT32    /**< 32-bit Floating Point audio samples */
};

enum jg_settingflag {
    JG_SETTING_RESTART  = 0x01, /**< Restart Required */
    JG_SETTING_INPUT    = 0x02  /**< Setting changes an input device */
};

// Typedefs
typedef struct _jg_coreinfo_t {
    const char *name;       /**< Short Name of the core e.g. "corename" */
    const char *fname;      /**< Full Name of the core e.g. "Core Name" */
    const char *version;    /**< Version e.g. "1.0 WIP" */
    const char *sys;        /**< System being emulated e.g. "sms" */
    uint8_t numinputs;      /**< Number of Ports/Inputs */
    uint32_t hints;         /**< Hints set for this core */
} jg_coreinfo_t;

typedef struct _jg_pathinfo_t {
    const char *base;       /**< Base user directory from which all other paths
                                begin */
    const char *core;       /**< Core asset path: for external support files
                                required for normal core operation, typically
                                distributed with the core */
    const char *user;       /**< User asset path: for external support files
                                not required for normal core operation,
                                typically optional audio/graphics/etc */
    const char *bios;       /**< BIOS directory: where BIOS or firmware files
                                are located */
    const char *save;       /**< Save directory: where SRAM, Memory Card, Real
                                Time Clock, or other non-volatile data is
                                saved */
} jg_pathinfo_t;

typedef struct _jg_fileinfo_t {
    void *data;             /**< Pointer to the file data */
    size_t size;            /**< Size of the file data in bytes */
    uint32_t crc;           /**< CRC32 Checksum */
    const char *md5;        /**< MD5 Checksum */
    const char *path;       /**< Filesystem full path */
    const char *name;       /**< Name of the file (without extension) */
    const char *fname;      /**< Full name of the file (with extension) */
} jg_fileinfo_t;

typedef struct _jg_videoinfo_t {
    enum jg_pixfmt pixfmt;  /**< Pixel Format */
    unsigned wmax;          /**< Maximum Width (X Resolution) */
    unsigned hmax;          /**< Maximum Height (Y Resolution) */
    unsigned w;             /**< Width: Current displayed video width */
    unsigned h;             /**< Height: Current displayed video height */
    unsigned x;             /**< X Offset: Start drawing x pixels into the
                                canvas from the left */
    unsigned y;             /**< Y Offset: Start drawing y pixels into the
                                canvas from the top */
    unsigned p;             /**< Pitch: Row Length, usually equal to Width */
    double aspect;          /**< Aspect Ratio e.g. 4.0/3.0 == 1.333333 */
    void *buf;              /**< Pointer to Video Buffer */
} jg_videoinfo_t;

typedef struct _jg_audioinfo_t {
    enum jg_sampfmt sampfmt;    /**< Sample Format */
    unsigned rate;              /**< Sample Rate in Hz e.g. 48000 */
    unsigned channels;          /**< Number of Channels */
    unsigned spf;               /**< Number of Audio Samples Per Frame */
    void *buf;                  /**< Pointer to Audio Buffer */
} jg_audioinfo_t;

typedef struct _jg_inputinfo_t {
    enum jg_inputtype type;     /**< Type of emulated input device */
    int index;                  /**< Index/port input device is plugged into */
    const char *name;           /**< Short name e.g. "gamepad" */
    const char *fname;          /**< Full name e.g. "Game Pad" */
    const char **defs;          /**< List of input definitions/button names */
    int numaxes;                /**< Number of axes on emulated device */
    int numbuttons;             /**< Number of buttons on emulated device */
} jg_inputinfo_t;

typedef struct _jg_inputstate_t {
    int16_t *axis;      /**< 16-bit signed axis values allocated by frontend */
    uint8_t *button;    /**< Digital button values allocated by frontend */
    int32_t *coord;     /**< 32-bit signed coordinate values allocated by
                            frontend */
    int32_t *rel;       /**< 32-bit signed relative motion values allocated by
                            frontend */
} jg_inputstate_t;

typedef struct _jg_setting_t {
    const char *name;   /**< Name of the setting */
    const char *fname;  /**< Full (Friendly) name of the setting */
    const char *opts;   /**< List of options: This string must be in one of two
                            formats.\n\n For numeric ranges:\n
                            N = Setting\n\n
                            For a list of settings with distinct names:\n
                            0 = Option, 1 = Other Option, 2 = Another Option */
    const char *desc;   /**< Description of the setting (verbose) */
    int val;            /**< Value of the setting */
    int min;            /**< Range minimum */
    int max;            /**< Range maximum */
    uint32_t flags;     /**< Flags to handle special behaviour */
} jg_setting_t;

// Jolly Good API Callbacks

/**
 * Send log data to the frontend
 * @param log level
 * @param format string
 */
typedef void (*jg_cb_log_t)(int, const char *, ...);
void jg_set_cb_log(jg_cb_log_t);

/**
 * Tell the frontend how many samples to read from the audio buffer
 * @param number of samples to read
 */
typedef void (*jg_cb_audio_t)(size_t);
void jg_set_cb_audio(jg_cb_audio_t);

/**
 * Send frame time interval to the frontend
 * @param frame time interval
 */
typedef void (*jg_cb_frametime_t)(double);
void jg_set_cb_frametime(jg_cb_frametime_t);

/**
 * Send Force Feedback data to the frontend
 * @param port
 * @param strength (0.0 to 1.0)
 * @param length of time in frames
 */
typedef void (*jg_cb_rumble_t)(int, float, size_t);
void jg_set_cb_rumble(jg_cb_rumble_t);

// Jolly Good API Calls

/**
 * Initialize the core
 * @return success/fail
 */
int jg_init(void);

/**
 * Deinitialize a core
 */
void jg_deinit(void);

/**
 * Reset the system
 * @param hard or soft reset
 */
void jg_reset(int);

/**
 * Run a single frame of emulation
 */
void jg_exec_frame(void);

/**
 * Load a game
 * @return success/fail
 */
int jg_game_load(void);

/**
 * Unload a game
 * @return success/fail
 */
int jg_game_unload(void);

/**
 * Load a state by path
 * @param filename of the state to load
 */
int jg_state_load(const char *);

/**
 * Load raw state data
 * @param pointer to raw state data
 */
void jg_state_load_raw(const void*);

/**
 * Save a state by path
 * @param filename of the state to save
 */
int jg_state_save(const char *);

/**
 * Save raw state data
 * @return pointer to raw state data
 */
const void* jg_state_save_raw(void);

/**
 * Retrieve the size of state data
 * @return size of state data in bytes
 */
size_t jg_state_size(void);

/**
 * Select next disc, floppy, or other type of media
 */
void jg_media_select(void);

/**
 * Insert or remove media (disc, floppy, etc)
 */
void jg_media_insert(void);

/**
 * Clear (disable) all cheat codes
 */
void jg_cheat_clear(void);

/**
 * Set (enable) a cheat code
 * @param cheat code
 */
void jg_cheat_set(const char *);

/**
 * Rehash settings or other properties which may be modified while running
 */
void jg_rehash(void);

/**
 * Push data into the core
 * @param type of data being pushed
 * @param port number of device, if applicable (default 0)
 * @param pointer to a data buffer or struct containing a data buffer
 * @param size of data or number of samples
 */
void jg_data_push(uint32_t, int, const void*, size_t);

/**
 * Return core information
 * @param system being emulated
 * @return information about the core
 */
jg_coreinfo_t* jg_get_coreinfo(const char *);

/**
 * Return video information
 * @return information about video rendering
 */
jg_videoinfo_t* jg_get_videoinfo(void);

/**
 * Return audio information
 * @return information about audio
 */
jg_audioinfo_t* jg_get_audioinfo(void);

/**
 * Return audio information
 * @param port input device is plugged into
 * @return information about an input device
 */
jg_inputinfo_t* jg_get_inputinfo(int);

/**
 * Return number of emulator-specific settings, get pointer to settings array
 * @param pointer to frontend's variable used to hold the number of settings
 * @return pointer to core's settings array
 */
jg_setting_t* jg_get_settings(size_t*);

/**
 * Set up video parameters after a game is loaded
 */
void jg_setup_video(void);

/**
 * Set up audio parameters after a game is loaded
 */
void jg_setup_audio(void);

/**
 * Pass a pointer to an input state into the emulator core
 * @param pointer to input state
 * @param index/port number
 */
void jg_set_inputstate(jg_inputstate_t*, int);

/**
 * Tell the emulator information about the game
 * @param jg_fileinfo_t
 */
void jg_set_gameinfo(jg_fileinfo_t);

/**
 * Tell the emulator information about an auxiliary file
 * @param jg_fileinfo_t
 * @param index number
 */
void jg_set_auxinfo(jg_fileinfo_t, int);

/**
 * Tell the emulator the paths for file operations
 * @param jg_pathinfo_t
 */
void jg_set_paths(jg_pathinfo_t);

#ifdef __cplusplus
}
#endif
#endif
