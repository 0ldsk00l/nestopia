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

#define NST_VERSION "1.46-WIP"

#ifdef _WIN32
#define snprintf _snprintf
#endif

using namespace Nes;

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static uint32_t video_buffer[Api::Video::Output::NTSC_WIDTH * Api::Video::Output::HEIGHT];
static int16_t audio_buffer[(44100 / 50)];
static int16_t audio_stereo_buffer[2 * (44100 / 50)];
static Api::Emulator emulator;
static Api::Machine *machine;
static Api::Fds *fds;
static char g_basename[256];
static char g_rom_dir[256];
static bool use_overscan;
static unsigned blargg_ntsc;

int16_t video_width = Api::Video::Output::WIDTH;
size_t pitch;

static Api::Video::Output *video;
static Api::Sound::Output *audio;
static Api::Input::Controllers *input;

static void *sram;
static unsigned long sram_size;
static bool is_pal;

int crossx = 0;
int crossy = 0;

void draw_crosshair(int x, int y)
{
   uint32_t w = 0xFFFFFFFF;
   uint32_t b = 0x00000000;
   
   for(int i = -3; i < 4; i++) {
      video_buffer[256 * y + x + i] = b;
      video_buffer[256 * (y + i) + x] = b;
   }
   
   for(int i = -2; i < 3; i += 2) {
      video_buffer[256 * y + x + i] = w;
      video_buffer[256 * (y + i) + x] = w;
   }
}

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
            if (log_cb)
               log_cb(RETRO_LOG_INFO, "[Nestopia]: SRAM changed place in RAM!\n");
         break;
      case Api::User::File::LOAD_FDS:
         {
            char base[256];
            snprintf(base, sizeof(base), "%s%c%s.sav", g_rom_dir, slash, g_basename);
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
            snprintf(base, sizeof(base), "%s%c%s.sav", g_rom_dir, slash, g_basename);
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

   machine = new Api::Machine(emulator);
   input = new Api::Input::Controllers;
   Api::User::fileIoCallback.Set(file_io_callback, 0);

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

   check_system_specs();
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
   info->valid_extensions = "nes|fds";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   const retro_system_timing timing = { is_pal ? 50.0 : 60.0, 44100.0 };
   info->timing = timing;

   // It's better if the size is based on NTSC_WIDTH if the filter is on
   const retro_game_geometry geom = {
      Api::Video::Output::WIDTH - (use_overscan ? 0 : 16),
      Api::Video::Output::HEIGHT - (use_overscan ? 0 : 16),
      Api::Video::Output::NTSC_WIDTH,
      Api::Video::Output::HEIGHT,
      4.0 / 3.0,
   };
   info->geometry = geom;
}


void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   static const struct retro_variable vars[] = {
      { "blargg_ntsc_filter", "Blargg NTSC filter; disabled|composite|svideo|rgb" },
      { "palette", "Palette; canonical|consumer|alternative|rgb" },
      { "nospritelimit", "Remove 8-sprites-per-scanline hardware limit; disabled|enabled" },
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
   input->zapper.fire = 0;
   input->vsSystem.insertCoin = 0;
   
   if (Api::Input(emulator).GetConnectedController(1) == 5) {
      static int zapx = use_overscan ? 0 : 8;
      static int zapy = use_overscan ? 0 : 8;
      zapx += input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_X);
      zapy += input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);
   
      if (zapx >= 256) { crossx = 255; }
      else if (zapx <= 0) { crossx = 0; }
      else {crossx = zapx; }
      
      if (zapy >= 240) { crossy = 239; }
      else if (zapy <= 0) { crossy = 0; }
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
   
   for (unsigned p = 0; p < 2; p++)
      for (unsigned bind = 0; bind < sizeof(bindmap) / sizeof(bindmap[0]); bind++)
         input->pad[p].buttons |= input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, bindmap[bind].retro) ? bindmap[bind].nes : 0;
         
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
      input->vsSystem.insertCoin |= Core::Input::Controllers::VsSystem::COIN_1;
      
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R))
      input->vsSystem.insertCoin |= Core::Input::Controllers::VsSystem::COIN_2;
      
   if (machine->Is(Nes::Api::Machine::DISK))
   {
      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y) &&
            fds->CanChangeDiskSide())
         fds->ChangeSide();
      
      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) && (fds->GetNumDisks() > 1))
      {
         int currdisk = fds->GetCurrentDisk();
         fds->EjectDisk();
         fds->InsertDisk(!currdisk, 0);
      }
   }
}

static void check_variables(void)
{
   static bool last_ntsc_val_same;
   struct retro_variable var = {0};

   retro_reset();
   Api::Video video(emulator);
   Api::Video::RenderState renderState;
   Api::Machine machine( emulator );
   Api::Video::RenderState::Filter filter;
   
   var.key = "nospritelimit";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "disabled") == 0)
         video.EnableUnlimSprites(false);
      else if (strcmp(var.value, "enabled") == 0)
         video.EnableUnlimSprites(true);
   }
   
   var.key = "blargg_ntsc_filter";
   
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
   }
   
   switch(blargg_ntsc)
   {
      case 0:
         filter = Api::Video::RenderState::FILTER_NONE;
         video_width = Api::Video::Output::WIDTH;
         break;
      case 1:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
      case 2:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_COMP);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_COMP);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_COMP);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_COMP);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_COMP);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
      case 3:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_SVIDEO);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_SVIDEO);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_SVIDEO);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
      case 4:
         filter = Api::Video::RenderState::FILTER_NTSC;
         video.SetSharpness(Api::Video::DEFAULT_SHARPNESS_RGB);
         video.SetColorResolution(Api::Video::DEFAULT_COLOR_RESOLUTION_RGB);
         video.SetColorBleed(Api::Video::DEFAULT_COLOR_BLEED_RGB);
         video.SetColorArtifacts(Api::Video::DEFAULT_COLOR_ARTIFACTS_RGB);
         video.SetColorFringing(Api::Video::DEFAULT_COLOR_FRINGING_RGB);
         video_width = Api::Video::Output::NTSC_WIDTH;
         break;
   }
   
   var.key = "palette";
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "canonical") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_YUV);
         video.SetDecoder(Api::Video::DECODER_CANONICAL);
      }
      else if (strcmp(var.value, "consumer") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_YUV);
         video.SetDecoder(Api::Video::DECODER_CONSUMER);
      }
      else if (strcmp(var.value, "alternative") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_YUV);
         video.SetDecoder(Api::Video::DECODER_ALTERNATIVE);
      }
      else if (strcmp(var.value, "rgb") == 0) {
         video.GetPalette().SetMode(Api::Video::Palette::MODE_RGB);
      }
   }
   
   pitch = video_width * 4;
   
   renderState.filter = filter;
   renderState.width = video_width;
   renderState.height = Api::Video::Output::HEIGHT;
   renderState.bits.count = 32;
   renderState.bits.mask.r = 0x00ff0000;
   renderState.bits.mask.g = 0x0000ff00;
   renderState.bits.mask.b = 0x000000ff;
   if (NES_FAILED(video.SetRenderState( renderState )) && log_cb)
      log_cb(RETRO_LOG_INFO, "Nestopia core rejected render state\n");;
}

void retro_run(void)
{
   update_input();
   emulator.Execute(video, audio, input);

   if (Api::Input(emulator).GetConnectedController(1) == 5)
      draw_crosshair(crossx, crossy);
   
   unsigned frames = is_pal ? 44100 / 50 : 44100 / 60;
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
   
   bool overscan = blargg_ntsc || use_overscan;
   
   video_cb(video_buffer + (overscan ? 0 : (8 + 256 * 8)),
         video_width - (overscan ? 0 : 16),
         Api::Video::Output::HEIGHT - (overscan ? 0 : 16),
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
   
#if defined(_WIN32)
   slash = '\\';
#else
   slash = '/';
#endif

   if (!environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) || !dir)
      return false;

   if (!environ_cb(RETRO_ENVIRONMENT_GET_OVERSCAN, &use_overscan))
      use_overscan = true;

   snprintf(db_path, sizeof(db_path), "%s%cNstDatabase.xml", dir, slash);

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "NstDatabase.xml path: %s\n", db_path);
   
   Api::Cartridge::Database database(emulator);
   std::ifstream *db_file = new std::ifstream(db_path, std::ifstream::in|std::ifstream::binary);
   
   if (db_file->is_open())
   {
      database.Load(*db_file);
      database.Enable(true);
   }
   else
   {
      if (log_cb)
         log_cb(RETRO_LOG_WARN, "NstDatabase.xml required to detect region and some mappers.\n");
      delete db_file;
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

   /*if (info->path && (strstr(info->path, "(E)") || strstr(info->path, "(Europe)")))
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[Nestopia]: Favoring PAL.\n");
      system = Api::Machine::FAVORED_NES_PAL;
      is_pal = true;
   }
   else if (info->path && (strstr(info->path, "(J)") || strstr(info->path, "(Japan)")))
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[Nestopia]: Favoring Famicom.\n");
      system = Api::Machine::FAVORED_FAMICOM;
   }*/

   if (info->path && (strstr(info->path, ".fds") || strstr(info->path, ".FDS")))
   {
      fds = new Api::Fds(emulator);

      if (fds)
      {
         char fds_bios_path[256];
         /* search for BIOS in system directory */
         bool found = false;

         snprintf(fds_bios_path, sizeof(fds_bios_path), "%s%cdisksys.rom", dir, slash);
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

   Api::Machine::FavoredSystem system = Api::Machine::FAVORED_NES_NTSC;
   is_pal = false;

   if (machine->Load(ss, system))
      return false;

   // If you have no Database, no region detection for you.
   if (database.IsLoaded())
   {
	   machine->SetMode(machine->GetDesiredMode());
	   if (machine->GetMode() == Api::Machine::PAL)
		  is_pal = true;
   }
   else
      machine->SetMode(Api::Machine::NTSC);

   if (machine->Is(Nes::Api::Machine::DISK))
      fds->InsertDisk(0, 0);

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "[Nestopia]: Machine is %s.\n", is_pal ? "PAL" : "NTSC");

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

   //Api::Input(emulator).ConnectController(0, Api::Input::PAD1);
   //Api::Input(emulator).ConnectController(1, Api::Input::PAD2);
   Api::Input(emulator).AutoSelectController(0);
   Api::Input(emulator).AutoSelectController(1);
   //Api::Input(emulator).ConnectController(1, Api::Input::ZAPPER);

   machine->Power(true);

   check_variables();
   
   video = new Api::Video::Output(video_buffer, video_width * sizeof(uint32_t));

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
