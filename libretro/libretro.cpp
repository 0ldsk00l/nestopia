#include "libretro.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

#include <core/api/NstApiMachine.hpp>
#include <core/api/NstApiEmulator.hpp>
#include <core/api/NstApiVideo.hpp>
#include <core/api/NstApiSound.hpp>
#include <core/api/NstApiInput.hpp>

using namespace Nes;

static uint32_t video_buffer[Api::Video::Output::WIDTH * Api::Video::Output::HEIGHT];
static int16_t audio_buffer[2 * (44100 / 60)];
static Api::Emulator emulator;
static Api::Machine *machine;

static Api::Video::Output *video;
static Api::Sound::Output *audio;
static Api::Input::Controllers *input;

//static bool NST_CALLBACK lock_video(void*, Video::Output&) { return true; }
//static void NST_CALLBACK unlock_video(void*, Video::Output&) {}

void retro_init(void)
{
   machine = new Api::Machine(emulator);
   video = new Api::Video::Output(video_buffer, Api::Video::Output::WIDTH * sizeof(uint32_t));
   audio = new Api::Sound::Output(audio_buffer, 44100 / 60);
   input = new Api::Input::Controllers;

   //Video::Output::lockCallback.Set(lock_video, 0);
   //Video::Output::unlockCallback.Set(unlock_video, 0);
}

void retro_deinit(void)
{
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
   info->library_version  = "v1";
   info->need_fullpath    = false;
   info->valid_extensions = "nes|NES"; // Anything is fine, we don't care.
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   const retro_system_timing timing = { 60.0, 44100.0 };
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
}

void retro_run(void)
{
   update_input();
   emulator.Execute(video, audio, input);

   video_cb(video_buffer, 256, 240, 1024);
   audio_batch_cb(audio_buffer, 44100 / 60);
}

bool retro_load_game(const struct retro_game_info *info)
{
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "XRGB8888 is not supported.\n");
      return false;
   }
   
   std::stringstream ss(std::string(reinterpret_cast<const char*>(info->data),
            reinterpret_cast<const char*>(info->data) + info->size));

   if (machine->LoadCartridge(ss, Api::Machine::FAVORED_NES_NTSC))
      return false;

   machine->SetMode(machine->GetDesiredMode());
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
   isound.SetSpeaker(Api::Sound::SPEAKER_STEREO);

   Api::Input(emulator).ConnectController(0, Api::Input::PAD1);
   Api::Input(emulator).ConnectController(1, Api::Input::PAD2);

   machine->Power(true);

   return true;
}

void retro_unload_game(void)
{
   machine->Unload();
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned, const struct retro_game_info *, size_t)
{
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *, size_t)
{
   return false;
}

bool retro_unserialize(const void *, size_t)
{
   return false;
}

void *retro_get_memory_data(unsigned)
{
   return 0;
}

size_t retro_get_memory_size(unsigned)
{
   return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned, bool, const char *)
{}

