#ifndef LIBRETRO_CORE_OPTIONS_INTL_H__
#define LIBRETRO_CORE_OPTIONS_INTL_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1500 && _MSC_VER < 1900)
/* https://support.microsoft.com/en-us/kb/980263 */
#pragma execution_character_set("utf-8")
#pragma warning(disable:4566)
#endif

#include <libretro.h>

/*
 ********************************
 * VERSION: 1.3
 ********************************
 *
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

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

struct retro_core_option_definition option_defs_tr[] = {
   {
      "nestopia_blargg_ntsc_filter",
      "Blargg NTSC Filtresi",
      "Blargg NTSC filtrelerini etkinleştirin.",
      {
         { "disabled",   "Devre Dışı" },
         { "composite",  "Kompozit Video"  },
         { "svideo",     "S-Video" },
         { "rgb",        "RGB SCART" },
         { "monochrome", "Tek renkli" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_palette",
      "Palet",
      "Hangi renk paletinin kullanılacağını seçin.",
      {
         { "cxa2025as",            "CXA2025AS" },
         { "consumer",             "Consumer" },
         { "canonical",            "Standart" },
         { "alternative",          "Alternatif" },
         { "rgb",                  "RGB" },
         { "pal",                  "PAL" },
         { "composite-direct-fbx", "Doğrudan Kompozit FBx" },
         { "pvm-style-d93-fbx",    "PVM-style D93 FBx" },
         { "ntsc-hardware-fbx",    "NTSC donanım FBx" },
         { "nes-classic-fbx-fs",   "NES Klasik FBx FS" },
         { "raw",                  "Raw" },
         { "custom",               "Özel" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_nospritelimit",
      "Sprite Limitini Kaldır",
      "Tarama başına 8 sprite donanım sınırını kaldır.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_overclock",
      "CPU Hızı (Hız aşırtma)",
      "Öykünülmüş CPU'ya hız aşırtma uygula.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_select_adapter",
      "4 Oyuncu Adaptörü",
      "Gerekirse manuel olarak bir 4 Oyuncu Adaptörü seçin. Bazı oyunlar bağdaştırıcıyı NstDatabase.xml veritabanıyla doğru bir şekilde tanıyamayabilir, bu seçenek bunu düzeltmeye yardımcı olur.",
      {
         { "auto",    "Otomatik" },
         { "ntsc",    "NTSC" },
         { "famicom", "Famicom" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_fds_auto_insert",
      "FDS Otomatik Ekleme",
      "Yeniden başlatmada ilk FDS diskini otomatik olarak yerleştirir.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_overscan_v",
      "Aşırı Taşmayı Maskele (Dikey)",
      "Standart tanımlı bir televizyon ekranının kenarı çevresinde çerçeve tarafından gizlenmiş olabilecek potansiyel olarak rastgele aksaklık video çıkışını maskeleyin (dikey olarak).",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_overscan_h",
      "Aşırı Taşmayı Maskele (Yatay)",
      "Standart tanımlı bir televizyon ekranının kenarı çevresinde çerçeve tarafından gizlenmiş olabilecek rastgele aksaklıklı video çıkışını maskeleyin (yatay olarak).",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_aspect",
      "Tercih Edilen En Boy Oranı",
      "Tercih edilen en boy oranını seçin. RetroArch'ın en boy oranı, Video ayarlarında 'Çekirdek Tarafından Sağlanan'  olarak ayarlanmalıdır. 'Otomatik', en-boy oranı otomatik belirlemesi için NstDatabase.xml veritabanını kullanır. Eğer mevcut bir veritabanı yoksa, 'Otomatik' için NTSC'ye varsayılan olacaktır.",
      {
         { "auto", "Otomatik" },
         { "ntsc", "NTSC" },
         { "pal",  "PAL" },
         { "4:3",  "4:3" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_genie_distortion",
      "Game Genie Ses Bozulma",
      "Game Genie hile cihazı yanlışlıkla oyunlarda ses bozulmalarına neden olabilir. Bunu etkinleştirerek, oyun sesine ekleyeceği bozulmayı taklit edebilirsiniz.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_favored_system",
      "Sistem Bölgesi",
      "Sistemin hangi bölgeden olacağını seçin. 'Otomatik' bölgenin otomatik belirlenmesi için NstDatabase.xml veritabanı dosyasını kullanır. Eğer mevcut bir veritabanı yoksa, 'Otomatik' için NTSC'ye varsayılan olacaktır.",
      {
         { "auto",    "Otomatik" },
         { "ntsc",    "NTSC" },
         { "pal",     "PAL" },
         { "famicom", "Famicom" },
         { "dendy",   "Dendy" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_ram_power_state",
      "Açılıştaki RAM Durumu",
      "",
      {
         { "0x00",   NULL },
         { "0xFF",   NULL },
         { "random", "Rastgele" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_button_shift",
      "A/B/X/Y Saat Yönünde Kaydırma",
      "A/B/X/Y tuşlarını saat yönünde çevirir.", /* Açıklama daha sonra eklenecek */
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "nestopia_turbo_pulse",
      "Turbo Pulse Speed",
      "Turbo B ve Turbo A düğmeleri için turbo hızını ayarlayın.",
      {
         { NULL, NULL },
      },
      NULL
   },
   { NULL, NULL, NULL, {{0}}, NULL },
};

#ifdef __cplusplus
}
#endif

#endif
