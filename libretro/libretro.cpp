#include "libretro.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <fstream>

#include <core/api/NstApiMachine.hpp>
#include <core/api/NstApiEmulator.hpp>
#include <core/api/NstApiVideo.hpp>
#include <core/api/NstApiSound.hpp>
#include <core/api/NstApiInput.hpp>
#include <core/api/NstApiCartridge.hpp>
#include <core/api/NstApiUser.hpp>
#include <core/api/NstApiFds.hpp>

#define NST_VERSION "1.44"

#ifdef _WIN32
#define snprintf _snprintf
#endif

using namespace Nes;

static uint32_t video_buffer[Api::Video::Output::WIDTH * Api::Video::Output::HEIGHT];
static int16_t audio_buffer[(44100 / 50)];
static int16_t audio_stereo_buffer[2 * (44100 / 50)];
static Api::Emulator emulator;
static Api::Machine *machine;
static Api::Fds *fds;
static char g_basename[256];
static char g_rom_dir[256];

static Api::Video::Output *video;
static Api::Sound::Output *audio;
static Api::Input::Controllers *input;

static void *sram;
static unsigned long sram_size;
static bool is_pal;

static void NST_CALLBACK file_io_callback(void*, Api::User::File &file)
{
   const void *addr;
   unsigned long addr_size;
   char slash;

#ifdef _WIN32
   slash = '\\';
#else
   slash = '/';
#endif

   switch (file.GetAction())
   {
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
            fprintf(stderr, "[Nestopia]: SRAM changed place in RAM!\n");
         break;
      case Api::User::File::LOAD_FDS:
         {
            char base[256];
            snprintf(base, sizeof(base), "%s%c%s.sav", g_rom_dir, slash, g_basename);
            fprintf(stderr, "Want to load FDS sav from: %s\n", base);
            std::ifstream in_tmp(base,std::ifstream::in|std::ifstream::binary);

            if (!in_tmp.is_open())
               return;

            file.SetPatchContent(in_tmp);
         }
         break;
      case Api::User::File::SAVE_FDS:
         {
            char base[256];
            snprintf(base, sizeof(base), "%s%c%s.sav", g_rom_dir, slash, g_basename);
            fprintf(stderr, "Want to save FDS sav to: %s\n", base);
            std::ofstream out_tmp(base,std::ifstream::out|std::ifstream::binary);

            if (out_tmp.is_open())
               file.GetPatchContent(Api::User::File::PATCH_UPS, out_tmp);
         }
         break;
      default:
         break;
   }
}

void retro_init(void)
{
   machine = new Api::Machine(emulator);
   video = new Api::Video::Output(video_buffer, Api::Video::Output::WIDTH * sizeof(uint32_t));
   input = new Api::Input::Controllers;

   Api::User::fileIoCallback.Set(file_io_callback, 0);
}

void retro_deinit(void)
{
   if (machine->Is(Nes::Api::Machine::DISK))
   {
      if (fds)
         delete fds;
      fds = 0;
   }
   
   delete machine;
   delete video;
   delete audio;
   delete input;

   machine = 0;
   video   = 0;
   audio   = 0;
   input   = 0;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned, unsigned)
{
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "Nestopia";
   info->library_version  = "v" NST_VERSION;
   info->need_fullpath    = false;
   info->valid_extensions = "nes|NES|zip|ZIP|fds|FDS"; // Anything is fine, we don't care.
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   const retro_system_timing timing = { is_pal ? 50.0 : 60.0, 44100.0 };
   info->timing = timing;

   const retro_game_geometry geom = {
      256,
      240,
      256,
      240,
      4.0 / 3.0,
   };
   info->geometry = geom;
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
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
      fds->InsertDisk(0, 0);
   }
}

typedef struct
{
   unsigned retro;
   unsigned nes;
} keymap;

static const keymap bindmap[] = {
   { RETRO_DEVICE_ID_JOYPAD_A, Core::Input::Controllers::Pad::A },
   { RETRO_DEVICE_ID_JOYPAD_B, Core::Input::Controllers::Pad::B },
   { RETRO_DEVICE_ID_JOYPAD_SELECT, Core::Input::Controllers::Pad::SELECT },
   { RETRO_DEVICE_ID_JOYPAD_START, Core::Input::Controllers::Pad::START },
   { RETRO_DEVICE_ID_JOYPAD_UP, Core::Input::Controllers::Pad::UP },
   { RETRO_DEVICE_ID_JOYPAD_DOWN, Core::Input::Controllers::Pad::DOWN },
   { RETRO_DEVICE_ID_JOYPAD_LEFT, Core::Input::Controllers::Pad::LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT, Core::Input::Controllers::Pad::RIGHT },
};

static void update_input()
{
   input_poll_cb();
   input->pad[0].buttons = 0;
   input->pad[1].buttons = 0;

   for (unsigned p = 0; p < 2; p++)
      for (unsigned bind = 0; bind < sizeof(bindmap) / sizeof(bindmap[0]); bind++)
         input->pad[p].buttons |= input_state_cb(p,
               RETRO_DEVICE_JOYPAD, 0, bindmap[bind].retro) ? bindmap[bind].nes : 0;

   if (machine->Is(Nes::Api::Machine::DISK))
   {
      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y) &&
            fds->CanChangeDiskSide())
         fds->ChangeSide();
   }
}

void retro_run(void)
{
   update_input();
   emulator.Execute(video, audio, input);

   video_cb(video_buffer, 256, 240, 1024);

   unsigned frames = is_pal ? 44100 / 50 : 44100 / 60;
   for (unsigned i = 0; i < frames; i++)
      audio_stereo_buffer[(i << 1) + 0] = audio_stereo_buffer[(i << 1) + 1] = audio_buffer[i];
   audio_batch_cb(audio_stereo_buffer, frames);
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
   extract_basename(g_basename, info->path, sizeof(g_basename));
   extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "XRGB8888 is not supported.\n");
      return false;
   }
   
   std::stringstream ss(std::string(reinterpret_cast<const char*>(info->data),
            reinterpret_cast<const char*>(info->data) + info->size));

   // Hack. Nestopia API forces us to favor either.
   Api::Machine::FavoredSystem system = Api::Machine::FAVORED_NES_NTSC;
   is_pal = false;
   if (info->path && (strstr(info->path, "(E)") || strstr(info->path, "(Europe)")))
   {
      fprintf(stderr, "[Nestopia]: Favoring PAL.\n");
      system = Api::Machine::FAVORED_NES_PAL;
      is_pal = true;
   }
   else if (info->path && (strstr(info->path, "(J)") || strstr(info->path, "(Japan)")))
   {
      fprintf(stderr, "[Nestopia]: Favoring Famicom.\n");
      system = Api::Machine::FAVORED_FAMICOM;
   }

   if (info->path && (strstr(info->path, ".fds") || strstr(info->path, ".FDS")))
   {
      fds = new Api::Fds(emulator);

      if (fds)
      {
         const char *dir;
         char fds_bios_path[256];
         char slash;
         /* search for BIOS in system directory */
         bool found = false;

         if (!environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) || !dir)
            return false;

#if defined(_WIN32)
         slash = '\\';
#else
         slash = '/';
#endif

         snprintf(fds_bios_path, sizeof(fds_bios_path), "%s%cdisksys.rom", dir, slash);
         fprintf(stderr, "FDS BIOS path: %s\n", fds_bios_path);

         std::ifstream *fds_bios_file = new std::ifstream(fds_bios_path, std::ifstream::in|std::ifstream::binary);

         if (fds_bios_file->is_open())
         {
            fds->SetBIOS(fds_bios_file);
         }
         else
         {
            delete fds_bios_file;
            return false;
         }

         system = Api::Machine::FAVORED_FAMICOM;
      }
      else
         return false;
   }

   if (machine->Load(ss, system))
      return false;

   if (machine->Is(Nes::Api::Machine::DISK))
      fds->InsertDisk(0, 0);

   machine->SetMode(is_pal ? Api::Machine::PAL : Api::Machine::NTSC);
   fprintf(stderr, "[Nestopia]: Machine is %s.\n", is_pal ? "PAL" : "NTSC");

   audio = new Api::Sound::Output(audio_buffer, is_pal ? 44100 / 50 : 44100 / 60);

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
   isound.SetSampleRate(44100);
   isound.SetSpeaker(Api::Sound::SPEAKER_MONO);

   Api::Input(emulator).ConnectController(0, Api::Input::PAD1);
   Api::Input(emulator).ConnectController(1, Api::Input::PAD2);

   machine->Power(true);

   return true;
}

void retro_unload_game(void)
{
   machine->Unload();
   sram = 0;
   sram_size = 0;
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
   if (id != RETRO_MEMORY_SAVE_RAM)
      return 0;

   return sram;
}

size_t retro_get_memory_size(unsigned id)
{
   if (id != RETRO_MEMORY_SAVE_RAM)
      return 0;

   return sram_size;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned, bool, const char *)
{}

