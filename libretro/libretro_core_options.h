#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */

struct retro_core_option_definition option_defs_us[] = {
   {
      "nestopia_blargg_ntsc_filter",
      "Blargg NTSC filter",
      "Enable Blargg NTSC filters.",
      {
         { "disabled", "Disabled" },
         { "composite", "Composite Video"  },
         { "svideo", "S-Video" },
         { "rgb", "RGB SCART" },
         { "monochrome", "Monochrome" },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "nestopia_palette",
      "Palette",
      "Choose which color palette is going to be used.",
      {
         { "cxa2025as",   "CXA2025AS"   },
         { "consumer",    "Consumer"    },
         { "canonical",   "Canonical"   },
         { "alternative", "Alternative" },
         { "rgb",         "RGB"         },
         { "pal",         "PAL"         },
         { "composite-direct-fbx", "Composite Direct FBx"},
         { "pvm-style-d93-fbx",    "PVM-style D93 FBx"},
         { "ntsc-hardware-fbx",    "NTSC hardware FBx"},
         { "nes-classic-fbx-fs",   "NES Classic FBx FS"},
         { "raw",                  "Raw"},
         { "custom",               "Custom"},
         { NULL, NULL },
      },
      "cxa2025as" /* TODO/FIXME - is this correct ? */
   },
   {
      "nestopia_nospritelimit",
      "Remove 8-sprites-per-scanline hardware limit",
      "Self-explanatory.",
      {
         { "disabled",  "Disabled" },
         { "enabled",   "Enabled" },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "nestopia_overclock",
      "CPU Speed (Overclock)",
      "Overclock the emulated CPU.",
      {
         { "1x",  NULL },
         { "2x", NULL },
      },
      "1x"
   },
   {
      "nestopia_select_adapter",
      "4 Player Adapter",
      "Manually select a 4 Player Adapter if needed. Some games will not recognize the adapter correctly through the NstDatabase.xml database, this option should help fix that.",
      {
         { "auto", NULL },
         { "ntsc", NULL },
         { "famicom", NULL },
         { NULL, NULL },
      },
      "auto"
   },
   {
      "nestopia_fds_auto_insert",
      "Automatically insert first FDS disk on reset; enabled|disabled",
      "Self-explanatory",
      {
         { "disabled",           "Disabled" },
         { "enabled",            "Enabled" },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      "nestopia_overscan_v",
      "Mask Overscan (Vertical)",
      "Mask out (vertically) the potentially random glitchy video output that would have been hidden by the bezel around the edge of a standard-definition television screen.",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      "nestopia_overscan_h",
      "Mask Overscan (Horizontal)",
      "Mask out (horizontally) the potentially random glitchy video output that would have been hidden by the bezel around the edge of a standard-definition television screen.",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "nestopia_aspect",
      "Preferred aspect ratio",
      "Choose the preferred aspect ratio. RetroArch's aspect ratio must be set to Core provided in the Video seetings. 'auto' will use the NstDatabase.xml database file for aspect ratio autodetection. If there is no database present it will default to NTSC for 'auto'.",
      {
         { "auto", "Auto" },
         { "ntsc", "NTSC" },
         { "pal",  "PAL" },
         { "4:3",  "4:3" },
         { NULL, NULL },
      },
      "auto"
   },
   {
      "nestopia_genie_distortion",
      "Game Genie Sound Distortion",
      "The Game Genie cheat device could inadvertently introduce sound distortion in games. By enabling this, you can simulate the distortion it would add to a game's sound.",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "nestopia_favored_system",
      "Favored System",
      "Choose which region the system is from. 'auto' will use the NstDatabase.xml database file for region autodetection. If there is no database present it will default to NTSC for 'auto'.",
      {
         { "auto",  "Auto" },
         { "ntsc", "NTSC" },
         { "pal",  "PAL" },
         { "famicom",  "Famicom" },
         { "dendy",  "Dendy" },
         { NULL, NULL },
      },
      "auto"
   },
   {
      "nestopia_ram_power_state",
      "RAM Power-on State",
      "Awaiting description.",
      {
         { "0x00", NULL },
         { "0xFF", NULL },
         { "random",  "Random" },
         { NULL, NULL },
      },
      "0x00"
   },
   {
      "nestopia_button_shift",
      "Shift A/B/X/Y Clockwise",
      "Awaiting description.",
      {
         { "disabled", NULL },
         { "enabled", NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "nestopia_turbo_pulse",
      "Turbo Pulse Speed",
      "Set the turbo pulse speed for the Turbo B and Turbo A buttons.",
      {
         { "2", NULL },
         { "3", NULL },
         { "4", NULL },
         { "5", NULL },
         { "6", NULL },
         { "7", NULL },
         { "8", NULL },
         { "9", NULL },
         { NULL, NULL },
      },
      "2"
   },
   { NULL, NULL, NULL, {{0}}, NULL },
};

/* RETRO_LANGUAGE_JAPANESE */

/* RETRO_LANGUAGE_FRENCH */

/* RETRO_LANGUAGE_SPANISH */

/* RETRO_LANGUAGE_GERMAN */

/* RETRO_LANGUAGE_ITALIAN */

/* RETRO_LANGUAGE_DUTCH */

/* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */

/* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */

/* RETRO_LANGUAGE_RUSSIAN */

/* RETRO_LANGUAGE_KOREAN */

/* RETRO_LANGUAGE_CHINESE_TRADITIONAL */

/* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */

/* RETRO_LANGUAGE_ESPERANTO */

/* RETRO_LANGUAGE_POLISH */

/* RETRO_LANGUAGE_VIETNAMESE */

/* RETRO_LANGUAGE_ARABIC */

/* RETRO_LANGUAGE_GREEK */

/* RETRO_LANGUAGE_TURKISH */

/*
 ********************************
 * Language Mapping
 ********************************
*/

struct retro_core_option_definition *option_defs_intl[RETRO_LANGUAGE_LAST] = {
   option_defs_us, /* RETRO_LANGUAGE_ENGLISH */
   NULL,           /* RETRO_LANGUAGE_JAPANESE */
   NULL,           /* RETRO_LANGUAGE_FRENCH */
   NULL,           /* RETRO_LANGUAGE_SPANISH */
   NULL,           /* RETRO_LANGUAGE_GERMAN */
   NULL,           /* RETRO_LANGUAGE_ITALIAN */
   NULL,           /* RETRO_LANGUAGE_DUTCH */
   NULL,           /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,           /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,           /* RETRO_LANGUAGE_RUSSIAN */
   NULL,           /* RETRO_LANGUAGE_KOREAN */
   NULL,           /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,           /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,           /* RETRO_LANGUAGE_ESPERANTO */
   NULL,           /* RETRO_LANGUAGE_POLISH */
   NULL,           /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,           /* RETRO_LANGUAGE_ARABIC */
   NULL,           /* RETRO_LANGUAGE_GREEK */
   NULL,           /* RETRO_LANGUAGE_TURKISH */
};

/*
 ********************************
 * Functions
 ********************************
*/

/* Handles configuration/setting of core options.
 * Should only be called inside retro_set_environment().
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

static INLINE void libretro_set_core_options(retro_environment_t environ_cb)
{
   unsigned version = 0;

   if (!environ_cb)
      return;

   if (environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version) && (version == 1))
   {
      struct retro_core_options_intl core_options_intl;
      unsigned language = 0;

      core_options_intl.us    = option_defs_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = option_defs_intl[language];

      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_intl);
   }
   else
   {
      size_t i;
      size_t num_options               = 0;
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine number of options */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      /* Allocate arrays */
      variables  = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
      values_buf = (char **)calloc(num_options, sizeof(char *));

      if (!variables || !values_buf)
         goto error;

      /* Copy parameters from option_defs_us array */
      for (i = 0; i < num_options; i++)
      {
         const char *key                        = option_defs_us[i].key;
         const char *desc                       = option_defs_us[i].desc;
         const char *default_value              = option_defs_us[i].default_value;
         struct retro_core_option_value *values = option_defs_us[i].values;
         size_t buf_len                         = 3;
         size_t default_index                   = 0;

         values_buf[i] = NULL;

         if (desc)
         {
            size_t num_values = 0;

            /* Determine number of values */
            while (true)
            {
               if (values[num_values].value)
               {
                  /* Check if this is the default value */
                  if (default_value)
                     if (strcmp(values[num_values].value, default_value) == 0)
                        default_index = num_values;

                  buf_len += strlen(values[num_values].value);
                  num_values++;
               }
               else
                  break;
            }

            /* Build values string */
            if (num_values > 1)
            {
               size_t j;

               buf_len += num_values - 1;
               buf_len += strlen(desc);

               values_buf[i] = (char *)calloc(buf_len, sizeof(char));
               if (!values_buf[i])
                  goto error;

               strcpy(values_buf[i], desc);
               strcat(values_buf[i], "; ");

               /* Default value goes first */
               strcat(values_buf[i], values[default_index].value);

               /* Add remaining values */
               for (j = 0; j < num_values; j++)
               {
                  if (j != default_index)
                  {
                     strcat(values_buf[i], "|");
                     strcat(values_buf[i], values[j].value);
                  }
               }
            }
         }

         variables[i].key   = key;
         variables[i].value = values_buf[i];
      }
      
      /* Set variables */
      environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

error:

      /* Clean up */
      if (values_buf)
      {
         for (i = 0; i < num_options; i++)
         {
            if (values_buf[i])
            {
               free(values_buf[i]);
               values_buf[i] = NULL;
            }
         }

         free(values_buf);
         values_buf = NULL;
      }

      if (variables)
      {
         free(variables);
         variables = NULL;
      }
   }
}

#ifdef __cplusplus
}
#endif

#endif
