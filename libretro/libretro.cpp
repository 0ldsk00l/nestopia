#include "libretro.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <fstream>

#include "../source/core/api/NstApiMachine.hpp"
#include "../source/core/api/NstApiEmulator.hpp"
#include "../source/core/api/NstApiVideo.hpp"
#include "../source/core/api/NstApiCheats.hpp"
#include "../source/core/api/NstApiSound.hpp"
#include "../source/core/api/NstApiInput.hpp"
#include "../source/core/api/NstApiCartridge.hpp"
#include "../source/core/api/NstApiUser.hpp"
#include "../source/core/api/NstApiFds.hpp"

#include "../source/core/NstMachine.hpp"

#define NST_VERSION "1.50-WIP"

#define MIN(a,b)      ((a)<(b)?(a):(b))
#define MAX(a,b)      ((a)>(b)?(a):(b))
#define NES_NTSC_PAR ((Api::Video::Output::WIDTH - (overscan_h ? 16 : 0)) * (8.0 / 7.0)) / (Api::Video::Output::HEIGHT - (overscan_v ? 16 : 0))
#define NES_PAL_PAR ((Api::Video::Output::WIDTH - (overscan_h ? 16 : 0)) * (2950000.0 / 2128137.0)) / (Api::Video::Output::HEIGHT - (overscan_v ? 16 : 0))
#define NES_4_3_DAR (4.0 / 3.0);
#define SAMPLERATE 48000

using namespace Nes;

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

#ifdef _3DS
extern "C" void* linearMemAlign(size_t size, size_t alignment);
extern "C" void linearFree(void* mem);
#endif
static uint32_t* video_buffer = NULL;

static int16_t audio_buffer[(SAMPLERATE / 50)];
static int16_t audio_stereo_buffer[2 * (SAMPLERATE / 50)];
static Api::Emulator emulator;
static Api::Machine *machine;
static Api::Fds *fds;
static char g_basename[256];
static char g_rom_dir[256];
static char *g_save_dir;
static char samp_dir[256];
static unsigned blargg_ntsc;
static bool fds_auto_insert;
static bool overscan_v;
static bool overscan_h;
static unsigned aspect_ratio_mode;
static unsigned tpulse;

int16_t video_width = Api::Video::Output::WIDTH;
size_t pitch;

static Api::Video::Output *video;
static Api::Sound::Output *audio;
static Api::Input::Controllers *input;
static unsigned input_type[4];
static Api::Machine::FavoredSystem favsystem;

static void *sram;
static unsigned long sram_size;
static bool is_pal;
static bool dbpresent;
static byte custpal[64*3];
static char slash;

static const byte cxa2025as_palette[64][3] =
{
  {0x58,0x58,0x58}, {0x00,0x23,0x8C}, {0x00,0x13,0x9B}, {0x2D,0x05,0x85},
  {0x5D,0x00,0x52}, {0x7A,0x00,0x17}, {0x7A,0x08,0x00}, {0x5F,0x18,0x00},
  {0x35,0x2A,0x00}, {0x09,0x39,0x00}, {0x00,0x3F,0x00}, {0x00,0x3C,0x22},
  {0x00,0x32,0x5D}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xA1,0xA1,0xA1}, {0x00,0x53,0xEE}, {0x15,0x3C,0xFE}, {0x60,0x28,0xE4},
  {0xA9,0x1D,0x98}, {0xD4,0x1E,0x41}, {0xD2,0x2C,0x00}, {0xAA,0x44,0x00},
  {0x6C,0x5E,0x00}, {0x2D,0x73,0x00}, {0x00,0x7D,0x06}, {0x00,0x78,0x52},
  {0x00,0x69,0xA9}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0x1F,0xA5,0xFE}, {0x5E,0x89,0xFE}, {0xB5,0x72,0xFE},
  {0xFE,0x65,0xF6}, {0xFE,0x67,0x90}, {0xFE,0x77,0x3C}, {0xFE,0x93,0x08},
  {0xC4,0xB2,0x00}, {0x79,0xCA,0x10}, {0x3A,0xD5,0x4A}, {0x11,0xD1,0xA4},
  {0x06,0xBF,0xFE}, {0x42,0x42,0x42}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0xA0,0xD9,0xFE}, {0xBD,0xCC,0xFE}, {0xE1,0xC2,0xFE},
  {0xFE,0xBC,0xFB}, {0xFE,0xBD,0xD0}, {0xFE,0xC5,0xA9}, {0xFE,0xD1,0x8E},
  {0xE9,0xDE,0x86}, {0xC7,0xE9,0x92}, {0xA8,0xEE,0xB0}, {0x95,0xEC,0xD9},
  {0x91,0xE4,0xFE}, {0xAC,0xAC,0xAC}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

static const byte pal_palette[64][3] =
{
  {0x80,0x80,0x80}, {0x00,0x00,0xBA}, {0x37,0x00,0xBF}, {0x84,0x00,0xA6},
  {0xBB,0x00,0x6A}, {0xB7,0x00,0x1E}, {0xB3,0x00,0x00}, {0x91,0x26,0x00},
  {0x7B,0x2B,0x00}, {0x00,0x3E,0x00}, {0x00,0x48,0x0D}, {0x00,0x3C,0x22},
  {0x00,0x2F,0x66}, {0x00,0x00,0x00}, {0x05,0x05,0x05}, {0x05,0x05,0x05},
  {0xC8,0xC8,0xC8}, {0x00,0x59,0xFF}, {0x44,0x3C,0xFF}, {0xB7,0x33,0xCC},
  {0xFE,0x33,0xAA}, {0xFE,0x37,0x5E}, {0xFE,0x37,0x1A}, {0xD5,0x4B,0x00},
  {0xC4,0x62,0x00}, {0x3C,0x7B,0x00}, {0x1D,0x84,0x15}, {0x00,0x95,0x66},
  {0x00,0x84,0xC4}, {0x11,0x11,0x11}, {0x09,0x09,0x09}, {0x09,0x09,0x09},
  {0xFE,0xFE,0xFE}, {0x00,0x95,0xFF}, {0x6F,0x84,0xFF}, {0xD5,0x6F,0xFF},
  {0xFE,0x77,0xCC}, {0xFE,0x6F,0x99}, {0xFE,0x7B,0x59}, {0xFE,0x91,0x5F},
  {0xFE,0xA2,0x33}, {0xA6,0xBF,0x00}, {0x51,0xD9,0x6A}, {0x4D,0xD5,0xAE},
  {0x00,0xD9,0xFF}, {0x66,0x66,0x66}, {0x0D,0x0D,0x0D}, {0x0D,0x0D,0x0D},
  {0xFE,0xFE,0xFE}, {0x84,0xBF,0xFF}, {0xBB,0xBB,0xFF}, {0xD0,0xBB,0xFF},
  {0xFE,0xBF,0xEA}, {0xFE,0xBF,0xCC}, {0xFE,0xC4,0xB7}, {0xFE,0xCC,0xAE},
  {0xFE,0xD9,0xA2}, {0xCC,0xE1,0x99}, {0xAE,0xEE,0xB7}, {0xAA,0xF8,0xEE},
  {0xB3,0xEE,0xFF}, {0xDD,0xDD,0xDD}, {0x11,0x11,0x11}, {0x11,0x11,0x11}
};

static const byte composite_direct_fbx_palette[64][3] =
{
  {0x65,0x65,0x65}, {0x00,0x12,0x7D}, {0x18,0x00,0x8E}, {0x36,0x00,0x82},
  {0x56,0x00,0x5D}, {0x5A,0x00,0x18}, {0x4F,0x05,0x00}, {0x38,0x19,0x00},
  {0x1D,0x31,0x00}, {0x00,0x3D,0x00}, {0x00,0x41,0x00}, {0x00,0x3B,0x17},
  {0x00,0x2E,0x55}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xAF,0xAF,0xAF}, {0x19,0x4E,0xC8}, {0x47,0x2F,0xE3}, {0x6B,0x1F,0xD7},
  {0x93,0x1B,0xAE}, {0x9E,0x1A,0x5E}, {0x99,0x32,0x00}, {0x7B,0x4B,0x00},
  {0x5B,0x67,0x00}, {0x26,0x7A,0x00}, {0x00,0x82,0x00}, {0x00,0x7A,0x3E},
  {0x00,0x6E,0x8A}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0x64,0xA9,0xFF}, {0x8E,0x89,0xFF}, {0xB6,0x76,0xFF},
  {0xE0,0x6F,0xFF}, {0xEF,0x6C,0xC4}, {0xF0,0x80,0x6A}, {0xD8,0x98,0x2C},
  {0xB9,0xB4,0x0A}, {0x83,0xCB,0x0C}, {0x5B,0xD6,0x3F}, {0x4A,0xD1,0x7E},
  {0x4D,0xC7,0xCB}, {0x4C,0x4C,0x4C}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0xC7,0xE5,0xFF}, {0xD9,0xD9,0xFF}, {0xE9,0xD1,0xFF},
  {0xF9,0xCE,0xFF}, {0xFF,0xCC,0xF1}, {0xFF,0xD4,0xCB}, {0xF8,0xDF,0xB1},
  {0xED,0xEA,0xA4}, {0xD6,0xF4,0xA4}, {0xC5,0xF8,0xB8}, {0xBE,0xF6,0xD3},
  {0xBF,0xF1,0xF1}, {0xB9,0xB9,0xB9}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

static const byte pvm_style_d93_fbx_palette[64][3] =
{
  {0x69,0x6B,0x63}, {0x00,0x17,0x74}, {0x1E,0x00,0x87}, {0x34,0x00,0x73},
  {0x56,0x00,0x57}, {0x5E,0x00,0x13}, {0x53,0x1A,0x00}, {0x3B,0x24,0x00},
  {0x24,0x30,0x00}, {0x06,0x3A,0x00}, {0x00,0x3F,0x00}, {0x00,0x3B,0x1E},
  {0x00,0x33,0x4E}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xB9,0xBB,0xB3}, {0x14,0x53,0xB9}, {0x4D,0x2C,0xDA}, {0x67,0x1E,0xDE},
  {0x98,0x18,0x9C}, {0x9D,0x23,0x44}, {0xA0,0x3E,0x00}, {0x8D,0x55,0x00},
  {0x65,0x6D,0x00}, {0x2C,0x79,0x00}, {0x00,0x81,0x00}, {0x00,0x7D,0x42},
  {0x00,0x78,0x8A}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0x69,0xA8,0xFF}, {0x96,0x91,0xFF}, {0xB2,0x8A,0xFA},
  {0xEA,0x7D,0xFA}, {0xF3,0x7B,0xC7}, {0xF2,0x8E,0x59}, {0xE6,0xAD,0x27},
  {0xD7,0xC8,0x05}, {0x90,0xDF,0x07}, {0x64,0xE5,0x3C}, {0x45,0xE2,0x7D},
  {0x48,0xD5,0xD9}, {0x4E,0x50,0x48}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0xD2,0xEA,0xFF}, {0xE2,0xE2,0xFF}, {0xE9,0xD8,0xFF},
  {0xF5,0xD2,0xFF}, {0xF8,0xD9,0xEA}, {0xFA,0xDE,0xB9}, {0xF9,0xE8,0x9B},
  {0xF3,0xF2,0x8C}, {0xD3,0xFA,0x91}, {0xB8,0xFC,0xA8}, {0xAE,0xFA,0xCA},
  {0xCA,0xF3,0xF3}, {0xBE,0xC0,0xB8}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

static const byte ntsc_hardware_fbx_palette[64][3] =
{
  {0x6A,0x6D,0x6A}, {0x00,0x13,0x80}, {0x1E,0x00,0x8A}, {0x39,0x00,0x7A},
  {0x55,0x00,0x56}, {0x5A,0x00,0x18}, {0x4F,0x10,0x00}, {0x38,0x21,0x00},
  {0x21,0x33,0x00}, {0x00,0x3D,0x00}, {0x00,0x40,0x00}, {0x00,0x39,0x24},
  {0x00,0x2E,0x55}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xB9,0xBC,0xB9}, {0x18,0x50,0xC7}, {0x4B,0x30,0xE3}, {0x73,0x22,0xD6},
  {0x95,0x1F,0xA9}, {0x9D,0x28,0x5C}, {0x96,0x3C,0x00}, {0x7A,0x51,0x00},
  {0x5B,0x67,0x00}, {0x22,0x77,0x00}, {0x02,0x7E,0x02}, {0x00,0x76,0x45},
  {0x00,0x6E,0x8A}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0x68,0xA6,0xFF}, {0x92,0x99,0xFF}, {0xB0,0x85,0xFF},
  {0xD9,0x75,0xFD}, {0xE3,0x77,0xB9}, {0xE5,0x8D,0x68}, {0xCF,0xA2,0x2C},
  {0xB3,0xAF,0x0C}, {0x7B,0xC2,0x11}, {0x55,0xCA,0x47}, {0x46,0xCB,0x81},
  {0x47,0xC1,0xC5}, {0x4A,0x4D,0x4A}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFF,0xFF,0xFF}, {0xCC,0xEA,0xFF}, {0xDD,0xDE,0xFF}, {0xEC,0xDA,0xFF},
  {0xF8,0xD7,0xFE}, {0xFC,0xD6,0xF5}, {0xFD,0xDB,0xCF}, {0xF9,0xE7,0xB5},
  {0xF1,0xF0,0xAA}, {0xDA,0xFA,0xA9}, {0xC9,0xFF,0xBC}, {0xC3,0xFB,0xD7},
  {0xC4,0xF6,0xF6}, {0xBE,0xC1,0xBE}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

static const byte nes_classic_fbx_fs_palette[64][3] =
{
  {0x60,0x61,0x5F}, {0x00,0x00,0x83}, {0x1D,0x01,0x95}, {0x34,0x08,0x75},
  {0x51,0x05,0x5E}, {0x56,0x00,0x0F}, {0x4C,0x07,0x00}, {0x37,0x23,0x08},
  {0x20,0x3A,0x0B}, {0x0F,0x4B,0x0E}, {0x19,0x4C,0x16}, {0x02,0x42,0x1E},
  {0x02,0x31,0x54}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xA9,0xAA,0xA8}, {0x10,0x4B,0xBF}, {0x47,0x12,0xD8}, {0x63,0x00,0xCA},
  {0x88,0x00,0xA9}, {0x93,0x0B,0x46}, {0x8A,0x2D,0x04}, {0x6F,0x52,0x06},
  {0x5C,0x71,0x14}, {0x1B,0x8D,0x12}, {0x19,0x95,0x09}, {0x17,0x84,0x48},
  {0x20,0x6B,0x8E}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFB,0xFB,0xFB}, {0x66,0x99,0xF8}, {0x89,0x74,0xF9}, {0xAB,0x58,0xF8},
  {0xD5,0x57,0xEF}, {0xDE,0x5F,0xA9}, {0xDC,0x7F,0x59}, {0xC7,0xA2,0x24},
  {0xA7,0xBE,0x03}, {0x75,0xD7,0x03}, {0x60,0xE3,0x4F}, {0x3C,0xD6,0x8D},
  {0x56,0xC9,0xCC}, {0x41,0x42,0x40}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
  {0xFB,0xFB,0xFB}, {0xBE,0xD4,0xFA}, {0xC9,0xC7,0xF9}, {0xD7,0xBE,0xFA},
  {0xE8,0xB8,0xF9}, {0xF5,0xBA,0xE5}, {0xF3,0xCA,0xC2}, {0xDF,0xCD,0xA7},
  {0xD9,0xE0,0x9C}, {0xC9,0xEB,0x9E}, {0xC0,0xED,0xB8}, {0xB5,0xF4,0xC7},
  {0xB9,0xEA,0xE9}, {0xAB,0xAB,0xAB}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

int crossx = 0;
int crossy = 0;

#define CROSSHAIR_SIZE 3

void draw_crosshair(int x, int y)
{
   uint32_t w = 0xFFFFFFFF;
   uint32_t b = 0x00000000;
   int current_width = 256;
   
   if (blargg_ntsc){
      x *= 2.36;
      current_width = 602;
   }

   for (int i = MAX(-CROSSHAIR_SIZE, -x); i <= MIN(CROSSHAIR_SIZE, current_width - x); i++) {
     video_buffer[current_width * y + x + i] = i % 2 == 0 ? w : b;
   }

   for (int i = MAX(-CROSSHAIR_SIZE, -y); i <= MIN(CROSSHAIR_SIZE, 239 - y); i++) {
     video_buffer[current_width * (y + i) + x] = i % 2 == 0 ? w : b;
   }
}

static void load_wav(const char* sampgame, Api::User::File& file)
{
   char samp_path[292];
   int length = 0;
   int blockalign = 0;
   int numchannels = 0;
   int bitspersample = 0;
   char fmt[4] = { 0x66, 0x6d, 0x74, 0x20};
   char subchunk2id[4] = { 0x64, 0x61, 0x74, 0x61};
   char *wavfile;
   char *dataptr;

   sprintf(samp_path, "%s%c%s%c%02d.wav", samp_dir, slash, sampgame, slash, file.GetId());

   std::ifstream samp_file(samp_path, std::ifstream::in|std::ifstream::binary);

   if (samp_file) {
	   samp_file.seekg(0, samp_file.end);
	   length = samp_file.tellg();
	   samp_file.seekg(0, samp_file.beg);
	   wavfile = (char*)malloc(length * sizeof(char));
	   samp_file.read(wavfile, length);

	   // Check to see if it has a valid header
	   if (memcmp(&wavfile[0x00], "RIFF", 4) != 0) { return; }
	   if (memcmp(&wavfile[0x08], "WAVE", 4) != 0) { return; }
	   if (memcmp(&wavfile[0x0c], &fmt, 4) != 0) { return; }
	   if (memcmp(&wavfile[0x24], &subchunk2id, 4) != 0) { return; }

	   // Load the sample into the emulator
	   dataptr = &wavfile[0x2c];
	   blockalign = wavfile[0x21] << 8 | wavfile[0x20];
	   numchannels = wavfile[0x17] << 8 | wavfile[0x16];
	   bitspersample = wavfile[0x23] << 8 | wavfile[0x22];
	   file.SetSampleContent(dataptr, (length - 44) / blockalign, 0, bitspersample, 44100);
	   free(wavfile);
   }
}

static void NST_CALLBACK file_io_callback(void*, Api::User::File &file)
{
   const void *addr;
   unsigned long addr_size;

#ifdef _WIN32
   slash = '\\';
#else
   slash = '/';
#endif

   switch (file.GetAction())
   {
      case Api::User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU:
         load_wav("moepro", file); break;
      case Api::User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88:
         load_wav("moepro88", file); break;
      case Api::User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS:
         load_wav("mptennis", file); break;
      case Api::User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU:
         load_wav("terao", file); break;
      case Api::User::File::LOAD_SAMPLE_AEROBICS_STUDIO:
         load_wav("ftaerobi", file); break;

      case Api::User::File::LOAD_BATTERY:
      case Api::User::File::LOAD_EEPROM:
      case Api::User::File::LOAD_TAPE:
      case Api::User::File::LOAD_TURBOFILE:
         file.GetRawStorage(sram, sram_size);
         break;

      case Api::User::File::SAVE_BATTERY:
      case Api::User::File::SAVE_EEPROM:
      case Api::User::File::SAVE_TAPE:
      case Api::User::File::SAVE_TURBOFILE:
         file.GetContent(addr, addr_size);
         if (addr != sram || sram_size != addr_size)
            if (log_cb)
               log_cb(RETRO_LOG_INFO, "[Nestopia]: SRAM changed place in RAM!\n");
         break;
      case Api::User::File::LOAD_FDS:
         {
            char base[256];
            sprintf(base, "%s%c%s.sav", g_save_dir, slash, g_basename);
            if (log_cb)
               log_cb(RETRO_LOG_INFO, "Want to load FDS sav from: %s\n", base);
            std::ifstream in_tmp(base,std::ifstream::in|std::ifstream::binary);

            if (!in_tmp.is_open())
               return;

            file.SetPatchContent(in_tmp);
         }
         break;
      case Api::User::File::SAVE_FDS:
         {
            char base[256];
            sprintf(base, "%s%c%s.sav", g_save_dir, slash, g_basename);
            if (log_cb)
               log_cb(RETRO_LOG_INFO, "Want to save FDS sav to: %s\n", base);
            std::ofstream out_tmp(base,std::ifstream::out|std::ifstream::binary);

            if (out_tmp.is_open())
               file.GetPatchContent(Api::User::File::PATCH_UPS, out_tmp);
         }
         break;
      default:
         break;
   }
}

static void check_system_specs(void)
{
   unsigned level = 6;
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_init(void)
{
   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;


   check_system_specs();
}

void retro_deinit(void)
{
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   if (port >= 4)
      return;

   input_type[port] = device;
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "Nestopia";
#ifdef GIT_VERSION
   info->library_version  = NST_VERSION GIT_VERSION;
#else
   info->library_version  = NST_VERSION;
#endif
   info->need_fullpath    = false;
   info->valid_extensions = "nes|fds|unf|unif";
}

double get_aspect_ratio(void)
{
  double aspect_ratio = is_pal ? NES_PAL_PAR : NES_NTSC_PAR;

  if (aspect_ratio_mode == 1)
    aspect_ratio = NES_NTSC_PAR;
  else if (aspect_ratio_mode == 2)
    aspect_ratio = NES_PAL_PAR;
  else if (aspect_ratio_mode == 3)
    aspect_ratio = NES_4_3_DAR;
    
  return aspect_ratio;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   const retro_system_timing timing = { is_pal ? 50.0 : 60.0, SAMPLERATE };
   info->timing = timing;

   // It's better if the size is based on NTSC_WIDTH if the filter is on
   const retro_game_geometry geom = {
      Api::Video::Output::WIDTH - (overscan_h ? 16 : 0),
      Api::Video::Output::HEIGHT - (overscan_v ? 16 : 0),
      Api::Video::Output::NTSC_WIDTH,
      Api::Video::Output::HEIGHT,
      get_aspect_ratio(),
   };
   info->geometry = geom;
}


void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   static const struct retro_variable vars[] = {
      { "nestopia_blargg_ntsc_filter", "Blargg NTSC filter; disabled|composite|svideo|rgb|monochrome" },
      { "nestopia_palette", "Palette; cxa2025as|consumer|canonical|alternative|rgb|pal|composite-direct-fbx|pvm-style-d93-fbx|ntsc-hardware-fbx|nes-classic-fbx-fs|raw|custom" },
      { "nestopia_nospritelimit", "Remove 8-sprites-per-scanline hardware limit; disabled|enabled" },
      { "nestopia_overclock", "CPU Speed (Overclock); 1x|2x" },
	  { "nestopia_select_adapter", "4 Player Adapter; auto|ntsc|famicom" },
      { "nestopia_fds_auto_insert", "Automatically insert first FDS disk on reset; enabled|disabled" },
      { "nestopia_overscan_v", "Mask Overscan (Vertical); enabled|disabled" },
      { "nestopia_overscan_h", "Mask Overscan (Horizontal); disabled|enabled" },
      { "nestopia_aspect" ,  "Preferred aspect ratio; auto|ntsc|pal|4:3" },
      { "nestopia_genie_distortion", "Game Genie Sound Distortion; disabled|enabled" },
      { "nestopia_favored_system", "Favored System; auto|ntsc|pal|famicom|dendy" },
      { "nestopia_ram_power_state", "RAM Power-on State; 0x00|0xFF|random" },
      { "nestopia_button_shift", "Shift A/B/X/Y Clockwise; disabled|enabled" },
      { "nestopia_turbo_pulse", "Turbo Pulse Speed; 2|3|4|5|6|7|8|9" },
      { NULL, NULL },
   };

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_reset(void)
{
   machine->Reset(false);

   if (machine->Is(Nes::Api::Machine::DISK))
   {
      fds->EjectDisk();
      if (fds_auto_insert)
         fds->InsertDisk(0, 0);
   }
}

typedef struct
{
   unsigned retro;
   unsigned nes;
} keymap;

static keymap bindmap_default[] = {
   { RETRO_DEVICE_ID_JOYPAD_A, Core::Input::Controllers::Pad::A },
   { RETRO_DEVICE_ID_JOYPAD_B, Core::Input::Controllers::Pad::B },
   { RETRO_DEVICE_ID_JOYPAD_X, Core::Input::Controllers::Pad::A },
   { RETRO_DEVICE_ID_JOYPAD_Y, Core::Input::Controllers::Pad::B },
   { RETRO_DEVICE_ID_JOYPAD_SELECT, Core::Input::Controllers::Pad::SELECT },
   { RETRO_DEVICE_ID_JOYPAD_START, Core::Input::Controllers::Pad::START },
   { RETRO_DEVICE_ID_JOYPAD_UP, Core::Input::Controllers::Pad::UP },
   { RETRO_DEVICE_ID_JOYPAD_DOWN, Core::Input::Controllers::Pad::DOWN },
   { RETRO_DEVICE_ID_JOYPAD_LEFT, Core::Input::Controllers::Pad::LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT, Core::Input::Controllers::Pad::RIGHT },
};

static keymap bindmap_shifted[] = {
   { RETRO_DEVICE_ID_JOYPAD_B, Core::Input::Controllers::Pad::A },
   { RETRO_DEVICE_ID_JOYPAD_Y, Core::Input::Controllers::Pad::B },
   { RETRO_DEVICE_ID_JOYPAD_A, Core::Input::Controllers::Pad::A },
   { RETRO_DEVICE_ID_JOYPAD_X, Core::Input::Controllers::Pad::B },
   { RETRO_DEVICE_ID_JOYPAD_SELECT, Core::Input::Controllers::Pad::SELECT },
   { RETRO_DEVICE_ID_JOYPAD_START, Core::Input::Controllers::Pad::START },
   { RETRO_DEVICE_ID_JOYPAD_UP, Core::Input::Controllers::Pad::UP },
   { RETRO_DEVICE_ID_JOYPAD_DOWN, Core::Input::Controllers::Pad::DOWN },
   { RETRO_DEVICE_ID_JOYPAD_LEFT, Core::Input::Controllers::Pad::LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT, Core::Input::Controllers::Pad::RIGHT },
};

static keymap *bindmap = bindmap_default;

static void update_input()
{
   input_poll_cb();
   input->pad[0].buttons = 0;
   input->pad[1].buttons = 0;
   input->pad[2].buttons = 0;
   input->pad[3].buttons = 0;
   input->pad[1].mic = 0;
   input->zapper.fire = 0;
   input->vsSystem.insertCoin = 0;
   
   if (Api::Input(emulator).GetConnectedController(1) == 5) {
      static int zapx = overscan_h ? 8 : 0; 
      static int zapy = overscan_v ? 8 : 0;
      int min_x = overscan_h ? 8 : 0;
      int max_x = overscan_h ? 247 : 255; 
      int min_y = overscan_v ? 8 : 0;
      int max_y = overscan_v ? 231 : 239;

      if (zapx > max_x)
         zapx = max_x;
      else if (zapx < min_x)
         zapx = min_x;
      else
         zapx += input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_X);

      if (zapy > max_y)
         zapy = max_y;
      else if (zapy < min_y)
         zapy = min_y;
      else
         zapy += input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);

      if (zapx > max_x) { crossx = max_x; }
      else if (zapx < min_x) { crossx = min_x; }
      else {crossx = zapx; }

      if (zapy > max_y) { crossy = max_y; }
      else if (zapy < min_y) { crossy = min_y; }
      else {crossy = zapy; }

      if (input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER)) {
         input->zapper.x = zapx;
         input->zapper.y = zapy;
         input->zapper.fire = 1;
      }
      
      if (input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TURBO)) {
         input->zapper.x = ~1U;
         input->zapper.fire = 1;
      }
   }
   
   static unsigned tstate = 2;
   
   for (unsigned p = 0; p < 4; p++) {
      for (unsigned bind = 0; bind < sizeof(bindmap_default) / sizeof(bindmap[0]); bind++)
      {
         input->pad[p].buttons |= input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, bindmap[bind].retro) ? bindmap[bind].nes : 0;
      }
      if (input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, bindmap[2].retro))
         tstate ? input->pad[p].buttons &= ~Core::Input::Controllers::Pad::A : input->pad[p].buttons |= Core::Input::Controllers::Pad::A;
      if (input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, bindmap[3].retro))
         tstate ? input->pad[p].buttons &= ~Core::Input::Controllers::Pad::B : input->pad[p].buttons |= Core::Input::Controllers::Pad::B;
   }
      
   if (tstate) tstate--; else tstate = tpulse;
   
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3))
      input->pad[1].mic |= 0x04;
   
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2))
      input->vsSystem.insertCoin |= Core::Input::Controllers::VsSystem::COIN_1;
      
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2))
      input->vsSystem.insertCoin |= Core::Input::Controllers::VsSystem::COIN_2;
      
   if (machine->Is(Nes::Api::Machine::DISK))
   {
      bool curL = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
      static bool prevL = false;
      if (curL && !prevL)
      {
         if (!fds->IsAnyDiskInserted())
            fds->InsertDisk(0, 0);
         else if (fds->CanChangeDiskSide())
            fds->ChangeSide();
      }
      prevL = curL;
      
      bool curR = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
      static bool prevR = false;
      if (curR && !prevR && (fds->GetNumDisks() > 1))
      {
         int currdisk = fds->GetCurrentDisk();
         fds->EjectDisk();
         fds->InsertDisk(!currdisk, 0);
      }
      prevR = curR;
   }
}

static void check_variables(void)
{
   static bool last_ntsc_val_same;
   struct retro_variable var = {0};
   struct retro_system_av_info av_info;

   Api::Sound sound(emulator);
   Api::Video video(emulator);
   Api::Video::RenderState renderState;
   Api::Machine machine(emulator);
   Api::Video::RenderState::Filter filter;

   var.key = "nestopia_button_shift";
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "disabled") == 0)
         bindmap = bindmap_default;
      else if (strcmp(var.value, "enabled") == 0)
         bindmap = bindmap_shifted;
   }
   
   var.key = "nestopia_favored_system";
   is_pal = false;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "auto") == 0)
      {
         if (dbpresent)
         {
            machine.SetMode(machine.GetDesiredMode());
            if (machine.GetMode() == Api::Machine::PAL)
            {
               is_pal = true;
               favsystem = Api::Machine::FAVORED_NES_PAL;
               machine.SetMode(Api::Machine::PAL);
            }
            else
            {
               favsystem = Api::Machine::FAVORED_NES_NTSC;
               machine.SetMode(Api::Machine::NTSC);
            }
         }
      }
      else if (strcmp(var.value, "ntsc") == 0)
      {
         favsystem = Api::Machine::FAVORED_NES_NTSC;
         machine.SetMode(Api::Machine::NTSC);
      }
      else if (strcmp(var.value, "pal") == 0)
      {
         favsystem = Api::Machine::FAVORED_NES_PAL;
         machine.SetMode(Api::Machine::PAL);
         is_pal = true;
      }
      else if (strcmp(var.value, "famicom") == 0)
      {
         favsystem = Api::Machine::FAVORED_FAMICOM;
         machine.SetMode(Api::Machine::NTSC);
      }
      else if (strcmp(var.value, "dendy") == 0)
      {
         favsystem = Api::Machine::FAVORED_DENDY;
         machine.SetMode(Api::Machine::PAL);
         is_pal = true;
      }
      else
      {
         favsystem = Api::Machine::FAVORED_NES_NTSC;
         machine.SetMode(Api::Machine::NTSC);
      }
   }
   if (audio) delete audio;
   audio = new Api::Sound::Output(audio_buffer, is_pal ? SAMPLERATE / 50 : SAMPLERATE / 60);

   var.key = "nestopia_genie_distortion";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "disabled") == 0)
         sound.SetGenie(0);
      else if (strcmp(var.value, "enabled") == 0)
         sound.SetGenie(1);
   }
   
   var.key = "nestopia_ram_power_state";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "0x00") == 0)
         machine.SetRamPowerState(0);
      else if (strcmp(var.value, "0xFF") == 0)
         machine.SetRamPowerState(1);
      else if (strcmp(var.value, "random") == 0)
         machine.SetRamPowerState(2);
   }

   var.key = "nestopia_nospritelimit";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "disabled") == 0)
         video.EnableUnlimSprites(false);
      else if (strcmp(var.value, "enabled") == 0)
         video.EnableUnlimSprites(true);
   }
   
   var.key = "nestopia_overclock";
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "1x") == 0)
         video.EnableOverclocking(false);
      else if (strcmp(var.value, "2x") == 0)
         video.EnableOverclocking(true);
   }
   
   var.key = "nestopia_fds_auto_insert";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
      fds_auto_insert = (strcmp(var.value, "enabled") == 0);
   
   var.key = "nestopia_blargg_ntsc_filter";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "disabled") == 0)
         blargg_ntsc = 0;
      else if (strcmp(var.value, "composite") == 0)
         blargg_ntsc = 2;
      else if (strcmp(var.value, "svideo") == 0)
         blargg_ntsc = 3;
      else if (strcmp(var.value, "rgb") == 0)
         blargg_ntsc = 4;
      else if (strcmp(var.value, "monochrome") == 0)
         blargg_ntsc = 5;
   }

   switch(blargg_ntsc)
   {
      case 0:
         filter = Api::Video::RenderState::FILTER_NONE;
         video_width = Api::Video::Output::WIDTH;
         video.SetSaturation(Api::Video::DEFAULT_SATURATION);
         break;
      case 2:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_COMP);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_COMP);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_COMP);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_COMP);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_COMP);
         video.SetSaturation(Api::Video::DEFAULT_SATURATION_COMP);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
      case 3:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_SVIDEO);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_SVIDEO);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_SVIDEO);
         video.SetSaturation(Api::Video::DEFAULT_SATURATION_SVIDEO);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
      case 4:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_RGB);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_RGB);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_RGB);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_RGB);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_RGB);
         video.SetSaturation(Api::Video::DEFAULT_SATURATION_RGB);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
     case 5:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_MONO);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_MONO);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_MONO);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_MONO);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_MONO);
         video.SetSaturation(Api::Video::DEFAULT_SATURATION_MONO);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
   }
   
   var.key = "nestopia_palette";
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "consumer") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_YUV);
         video.SetDecoder(Api::Video::DECODER_CONSUMER);
      }
      else if (strcmp(var.value, "canonical") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_YUV);
         video.SetDecoder(Api::Video::DECODER_CANONICAL);
      }
      else if (strcmp(var.value, "alternative") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_YUV);
         video.SetDecoder(Api::Video::DECODER_ALTERNATIVE);
      }
      else if (strcmp(var.value, "rgb") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_RGB);
      }
      else if (strcmp(var.value, "cxa2025as") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(cxa2025as_palette, Api::Video::Palette::STD_PALETTE);
      }
      else if (strcmp(var.value, "pal") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(pal_palette, Api::Video::Palette::STD_PALETTE);
      }
      else if (strcmp(var.value, "composite-direct-fbx") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(composite_direct_fbx_palette, Api::Video::Palette::STD_PALETTE);
      }
      else if (strcmp(var.value, "pvm-style-d93-fbx") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(pvm_style_d93_fbx_palette, Api::Video::Palette::STD_PALETTE);
      }
      else if (strcmp(var.value, "ntsc-hardware-fbx") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(ntsc_hardware_fbx_palette, Api::Video::Palette::STD_PALETTE);
      }
      else if (strcmp(var.value, "nes-classic-fbx-fs") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(nes_classic_fbx_fs_palette, Api::Video::Palette::STD_PALETTE);
      }
      else if (strcmp(var.value, "raw") == 0) {
         /* outputs raw chroma/level/emphasis in the R/G/B channels
          * that can be decoded by the frontend (using shaders for example)
          * the following formulas can be used to extract the
          * values back from a normalized R/G/B triplet
          * chroma   = floor((R * 15.0) + 0.5)
          * level    = floor((G *  3.0) + 0.5)
          * emphasis = floor((B *  7.0) + 0.5) */
         unsigned char raw_palette[512][3];
         int i;
         for (i = 0; i < 512; i++)
         {
            raw_palette[i][0] = (((i >> 0) & 0xF) * 255) / 15;
            raw_palette[i][1] = (((i >> 4) & 0x3) * 255) / 3;
            raw_palette[i][2] = (((i >> 6) & 0x7) * 255) / 7;
         }
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom(raw_palette, Api::Video::Palette::EXT_PALETTE);
      }
      else if (strcmp(var.value, "custom") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_CUSTOM);
         video.GetPalette().SetCustom((const byte(*)[3])custpal, Api::Video::Palette::STD_PALETTE);
      }
   }
   
   var.key = "nestopia_overscan_v";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
      overscan_v = (strcmp(var.value, "enabled") == 0);
   
   var.key = "nestopia_overscan_h";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
      overscan_h = (strcmp(var.value, "enabled") == 0);

   var.key = "nestopia_aspect";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
     if (!strcmp(var.value, "ntsc"))
       aspect_ratio_mode = 1;
     else if (!strcmp(var.value, "pal"))
       aspect_ratio_mode = 2;
     else if (!strcmp(var.value, "4:3"))
       aspect_ratio_mode = 3;
     else
       aspect_ratio_mode = 0;
   }
   
   var.key = "nestopia_select_adapter";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   { 
	   if (!strcmp(var.value, "auto")) {
		   if (dbpresent)
		   {
			   Api::Input(emulator).AutoSelectController(2);
			   Api::Input(emulator).AutoSelectController(3);
			   Api::Input(emulator).AutoSelectAdapter();
		   }
	   }
	   else if (!strcmp(var.value, "ntsc")) {
		   Api::Input(emulator).ConnectController(2, Api::Input::PAD3);
	       Api::Input(emulator).ConnectController(3, Api::Input::PAD4);
		   Api::Input(emulator).ConnectAdapter(Api::Input::ADAPTER_NES);
		}
	   else if (!strcmp(var.value, "famicom")) {
		   Api::Input(emulator).ConnectController(2, Api::Input::PAD3);
		   Api::Input(emulator).ConnectController(3, Api::Input::PAD4);
		   Api::Input(emulator).ConnectAdapter(Api::Input::ADAPTER_FAMICOM);
		}
   }

   var.key = "nestopia_turbo_pulse";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
      tpulse = atoi(var.value);
   
   pitch = video_width * 4;
   
   renderState.filter = filter;
   renderState.width = video_width;
   renderState.height = Api::Video::Output::HEIGHT;
   renderState.bits.count = 32;
   renderState.bits.mask.r = 0x00ff0000;
   renderState.bits.mask.g = 0x0000ff00;
   renderState.bits.mask.b = 0x000000ff;
   if (NES_FAILED(video.SetRenderState( renderState )) && log_cb)
      log_cb(RETRO_LOG_WARN, "Nestopia core rejected render state\n");;

   retro_get_system_av_info(&av_info);
   environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);

}

void retro_run(void)
{
   update_input();
   emulator.Execute(video, audio, input);

   if (Api::Input(emulator).GetConnectedController(1) == 5)
      draw_crosshair(crossx, crossy);
   
   unsigned frames = is_pal ? SAMPLERATE / 50 : SAMPLERATE / 60;
   for (unsigned i = 0; i < frames; i++)
      audio_stereo_buffer[(i << 1) + 0] = audio_stereo_buffer[(i << 1) + 1] = audio_buffer[i];
   audio_batch_cb(audio_stereo_buffer, frames);
   
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   {
      check_variables();
      delete video;
      video = 0;
      video = new Api::Video::Output(video_buffer, video_width * sizeof(uint32_t));
   }
   
   // Absolute mess of inline if statements...
   int dif = blargg_ntsc ? 18 : 8;

   video_cb(video_buffer + (overscan_v ? ((overscan_h ? dif : 0) + (blargg_ntsc ? Api::Video::Output::NTSC_WIDTH : Api::Video::Output::WIDTH) * 8) : (overscan_h ? dif : 0) + 0),
         video_width - (overscan_h ? 2 * dif : 0),
         Api::Video::Output::HEIGHT - (overscan_v ? 16 : 0),
         pitch);
}

static void extract_basename(char *buf, const char *path, size_t size)
{
   const char *base = strrchr(path, '/');
   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   char *ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';
}

static void extract_directory(char *buf, const char *path, size_t size)
{
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   char *base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';
}


bool retro_load_game(const struct retro_game_info *info)
{
   const char *dir;
   char slash;
   char db_path[256];
   char palette_path[256];
   
#if defined(_WIN32)
   slash = '\\';
#else
   slash = '/';
#endif

   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "Turbo A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Turbo B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "(FDS) Disk Side Change" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "(FDS) Eject Disk" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,    "(VSSystem) Coin 1" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,    "(VSSystem) Coin 2" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,    "(Famicom) Microphone" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "Turbo A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Turbo B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "(FDS) Disk Side Change" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "(FDS) Eject Disk" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "Turbo A" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Turbo B" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "(FDS) Disk Side Change" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "(FDS) Eject Disk" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "Turbo A" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Turbo B" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "(FDS) Disk Side Change" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "(FDS) Eject Disk" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 0 },
   };

#ifdef _3DS
   video_buffer = (uint32_t*)linearMemAlign(Api::Video::Output::NTSC_WIDTH * Api::Video::Output::HEIGHT * sizeof(uint32_t), 0x80);
#else
   video_buffer = (uint32_t*)malloc(Api::Video::Output::NTSC_WIDTH * Api::Video::Output::HEIGHT * sizeof(uint32_t));
#endif

   machine = new Api::Machine(emulator);
   input = new Api::Input::Controllers;
   Api::User::fileIoCallback.Set(file_io_callback, 0);

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   if (!environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) || !dir)
      return false;

   sprintf(samp_dir, "%s%cnestopia%csamples", dir, slash, slash);

   sprintf(palette_path, "%s%ccustom.pal", dir, slash);

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "Custom palette path: %s\n", palette_path);
   
   std::ifstream *custompalette = new std::ifstream(palette_path, std::ifstream::in|std::ifstream::binary);
   
   if (custompalette->is_open())
   {
      custompalette->read((char*)custpal, sizeof(custpal));
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "custom.pal loaded from system directory.\n");
   }
   else
   {
      memcpy(custpal, cxa2025as_palette, sizeof(custpal));
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "custom.pal not found in system directory.\n");
   }
   delete custompalette;
   
   sprintf(db_path, "%s%cNstDatabase.xml", dir, slash);

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "NstDatabase.xml path: %s\n", db_path);
   
   Api::Cartridge::Database database(emulator);
   std::ifstream *db_file = new std::ifstream(db_path, std::ifstream::in|std::ifstream::binary);
   
   if (db_file->is_open())
   {
      database.Load(*db_file);
      database.Enable(true);
      dbpresent = true;
   }
   else
   {
      if (log_cb)
         log_cb(RETRO_LOG_WARN, "NstDatabase.xml required to detect region and some mappers.\n");
      delete db_file;
      dbpresent = false;
   }
   
   if (info->path != NULL)
   {
      extract_basename(g_basename, info->path, sizeof(g_basename));
      extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));
   }
   
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "XRGB8888 is not supported.\n");
      return false;
   }
   
   std::stringstream ss(std::string(reinterpret_cast<const char*>(info->data),
            reinterpret_cast<const char*>(info->data) + info->size));

   if (info->path && (strstr(info->path, ".fds") || strstr(info->path, ".FDS")))
   {
      fds = new Api::Fds(emulator);

      if (fds)
      {
         char fds_bios_path[256];
         /* search for BIOS in system directory */
         bool found = false;

         sprintf(fds_bios_path, "%s%cdisksys.rom", dir, slash);
         if (log_cb)
            log_cb(RETRO_LOG_INFO, "FDS BIOS path: %s\n", fds_bios_path);

         std::ifstream *fds_bios_file = new std::ifstream(fds_bios_path, std::ifstream::in|std::ifstream::binary);

         if (fds_bios_file->is_open())
            fds->SetBIOS(fds_bios_file);
         else
         {
            delete fds_bios_file;
            return false;
         }
      }
      else
         return false;
   }
   
   if (!environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &g_save_dir))
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "Could not find save directory.\n");
   }

   is_pal = false;
   check_variables();

   if (machine->Load(ss, favsystem))
      return false;

   Api::Video ivideo(emulator);
   ivideo.SetSharpness(Api::Video::DEFAULT_SHARPNESS_RGB);
   ivideo.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_RGB);
   ivideo.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_RGB);
   ivideo.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_RGB);
   ivideo.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_RGB);

   Api::Video::RenderState state;
   state.filter = Api::Video::RenderState::FILTER_NONE;
   state.width = 256;
   state.height = 240;
   state.bits.count = 32;
   state.bits.mask.r = 0x00ff0000;
   state.bits.mask.g = 0x0000ff00;
   state.bits.mask.b = 0x000000ff;
   ivideo.SetRenderState(state);

   Api::Sound isound(emulator);
   isound.SetSampleBits(16);
   isound.SetSampleRate(SAMPLERATE);
   isound.SetSpeaker(Api::Sound::SPEAKER_MONO);

   if (dbpresent)
   {
      Api::Input(emulator).AutoSelectController(0);
      Api::Input(emulator).AutoSelectController(1);
   }
   else
   {
      Api::Input(emulator).ConnectController(0, Api::Input::PAD1);
      Api::Input(emulator).ConnectController(1, Api::Input::PAD2);
      //Api::Input(emulator).ConnectController(1, Api::Input::ZAPPER);
   }

   machine->Power(true);

   check_variables();

   if (fds_auto_insert && machine->Is(Nes::Api::Machine::DISK))
      fds->InsertDisk(0, 0);
   
   video = new Api::Video::Output(video_buffer, video_width * sizeof(uint32_t));
   
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "[Nestopia]: Machine is %s.\n", is_pal ? "PAL" : "NTSC");

   return true;
}

void retro_unload_game(void)
{
   if (machine)
   {
      machine->Unload();

      if (machine->Is(Nes::Api::Machine::DISK))
      {
         if (fds)
            delete fds;
         fds = 0;
      }

      delete machine;
   }

   if (video)
      delete video;
   if (audio)
      delete audio;
   if (input)
      delete input;

   machine = 0;
   video   = 0;
   audio   = 0;
   input   = 0;

   sram = 0;
   sram_size = 0;

#ifdef _3DS
   linearFree(video_buffer);
#else
   free(video_buffer);
#endif
   video_buffer = NULL;
}

unsigned retro_get_region(void)
{
   return is_pal ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned, const struct retro_game_info *, size_t)
{
   return false;
}

size_t retro_serialize_size(void)
{
   std::stringstream ss;
   if (machine->SaveState(ss, Api::Machine::NO_COMPRESSION))
      return 0;
   return ss.str().size();
}

bool retro_serialize(void *data, size_t size)
{
   std::stringstream ss;
   if (machine->SaveState(ss, Api::Machine::NO_COMPRESSION))
      return false;

   std::string state = ss.str();
   if (state.size() > size)
      return false;

   std::copy(state.begin(), state.end(), reinterpret_cast<char*>(data));
   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   std::stringstream ss(std::string(reinterpret_cast<const char*>(data),
            reinterpret_cast<const char*>(data) + size));
   return !machine->LoadState(ss);
}

void *retro_get_memory_data(unsigned id)
{
   Core::Machine& machineGet = emulator;
   switch(id)
   {
      case RETRO_MEMORY_SAVE_RAM:
      return sram;
       
      case RETRO_MEMORY_SYSTEM_RAM:
      return (void*)&machineGet.cpu.GetRam()[0];
       
   }

   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   Core::Machine& machineGet = emulator;
   switch(id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         return sram_size;
       
      case RETRO_MEMORY_SYSTEM_RAM:
         return machineGet.cpu.RAM_SIZE;
   }

   return 0;
}

void retro_cheat_reset(void)
{
   Nes::Api::Cheats cheater(emulator);
   cheater.ClearCodes();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   Nes::Api::Cheats cheater(emulator);
   Nes::Api::Cheats::Code ggCode;
   char codeCopy[256];
   char *part;

   if (code == NULL) return;
   strcpy(codeCopy,code);
   part = strtok(codeCopy,"+,;._ ");

   while (part)
   {
      if ((strlen(part) == 7) && (part[4]==':'))
      {
         part[4]='\0';
         ggCode.address=strtoul(part,NULL,16);
         ggCode.value=strtoul(part+5,NULL,16);
         cheater.SetCode(ggCode);
      }
      else if ((strlen(part)==10) && (part[4]=='?') && (part[7]==':'))
      {
         part[4]='\0';
         part[7]='\0';
         ggCode.address=strtoul(part,NULL,16);
         ggCode.compare=strtoul(part+5,NULL,16);
         ggCode.useCompare=true;
         ggCode.value=strtoul(part+8,NULL,16);
         cheater.SetCode(ggCode);
      }
      else if (Nes::Api::Cheats::GameGenieDecode(part, ggCode) == RESULT_OK)
         cheater.SetCode(ggCode);
      else if (Nes::Api::Cheats::ProActionRockyDecode(part, ggCode) == RESULT_OK)
         cheater.SetCode(ggCode);
      part = strtok(NULL,"+,;._ ");
   }
}
