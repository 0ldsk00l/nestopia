DEBUG = 0

NST_DIR = ..
HAVE_GCC_WARNINGS := 1

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
EXE_EXT = .exe
   system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   system_platform = osx
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   system_platform = win
endif

LIBRETRO_DIR := libretro

ifeq ($(platform), unix)
   TARGET := libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,-version-script=link.T -Wl,-no-undefined
else ifeq ($(platform), osx)
   TARGET := libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifeq ($(platform), ps3)
	HAVE_GCC_WARNINGS := 0
   TARGET := libretro_ps3.a
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   PLATFORM_DEFINES := -D__CELLOS_LV2__
else ifeq ($(platform), sncps3)
	HAVE_GCC_WARNINGS := 0
   TARGET := libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   PLATFORM_DEFINES := -D__CELLOS_LV2__
else ifeq ($(platform), psl1ght)
   TARGET := libretro_psl1ght.a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   PLATFORM_DEFINES := -D__CELLOS_LV2__
else ifeq ($(platform), xenon)
   TARGET := libretro_xenon360.a
   CC = xenon-gcc$(EXE_EXT)
   CXX = xenon-g++$(EXE_EXT)
   AR = xenon-ar$(EXE_EXT)
   PLATFORM_DEFINES := -D__LIBXENON__
else ifeq ($(platform), ngc)
   TARGET := libretro_ngc.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   PLATFORM_DEFINES += -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float
else ifeq ($(platform), wii)
   TARGET := libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   PLATFORM_DEFINES += -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float
else
   TARGET := retro.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -static-libgcc -static-libstdc++ -Wl,-no-undefined -Wl,-version-script=link.T
endif

all: $(TARGET)

ifeq ($(DEBUG), 1)
   CFLAGS += -O0 -g
   CXXFLAGS += -O0 -g
else
   CFLAGS += -O3
   CXXFLAGS += -O3
endif

OBJS = $(NST_DIR)/source/core/NstZlib.o

## Yes, I didn't type all this out ...
# core objs
OBJS += $(NST_DIR)/source/core/NstApu.o              $(NST_DIR)/source/core/NstFds.o            $(NST_DIR)/source/core/NstPpu.o              $(NST_DIR)/source/core/NstVector.o
OBJS += $(NST_DIR)/source/core/NstAssert.o           $(NST_DIR)/source/core/NstFile.o           $(NST_DIR)/source/core/NstProperties.o       $(NST_DIR)/source/core/NstVideoFilter2xSaI.o
OBJS += $(NST_DIR)/source/core/NstCartridge.o        $(NST_DIR)/source/core/NstImage.o          $(NST_DIR)/source/core/NstRam.o              $(NST_DIR)/source/core/NstVideoFilterHqX.o
OBJS += $(NST_DIR)/source/core/NstCartridgeInes.o    $(NST_DIR)/source/core/NstImageDatabase.o  $(NST_DIR)/source/core/NstSha1.o             $(NST_DIR)/source/core/NstVideoFilterNone.o
OBJS += $(NST_DIR)/source/core/NstCartridgeRomset.o  $(NST_DIR)/source/core/NstLog.o            $(NST_DIR)/source/core/NstSoundPcm.o         $(NST_DIR)/source/core/NstVideoFilterNtsc.o
OBJS += $(NST_DIR)/source/core/NstCartridgeUnif.o    $(NST_DIR)/source/core/NstMachine.o        $(NST_DIR)/source/core/NstSoundPlayer.o      $(NST_DIR)/source/core/NstVideoFilterScaleX.o
OBJS += $(NST_DIR)/source/core/NstCheats.o           $(NST_DIR)/source/core/NstMemory.o         $(NST_DIR)/source/core/NstSoundRenderer.o    $(NST_DIR)/source/core/NstVideoRenderer.o
OBJS += $(NST_DIR)/source/core/NstChecksum.o         $(NST_DIR)/source/core/NstNsf.o            $(NST_DIR)/source/core/NstState.o            $(NST_DIR)/source/core/NstVideoScreen.o
OBJS += $(NST_DIR)/source/core/NstChips.o            $(NST_DIR)/source/core/NstPatcher.o        $(NST_DIR)/source/core/NstStream.o           $(NST_DIR)/source/core/NstXml.o
OBJS += $(NST_DIR)/source/core/NstCore.o             $(NST_DIR)/source/core/NstPatcherIps.o     $(NST_DIR)/source/core/NstTracker.o          
OBJS += $(NST_DIR)/source/core/NstCpu.o              $(NST_DIR)/source/core/NstPatcherUps.o     $(NST_DIR)/source/core/NstTrackerMovie.o
OBJS += $(NST_DIR)/source/core/NstCrc32.o            $(NST_DIR)/source/core/NstPins.o           $(NST_DIR)/source/core/NstTrackerRewinder.o
OBJS += $(NST_DIR)/source/core/NstVideoFilterNtscCfg.o

# core/api
OBJS += $(NST_DIR)/source/core/api/NstApiBarcodeReader.o  $(NST_DIR)/source/core/api/NstApiEmulator.o  $(NST_DIR)/source/core/api/NstApiMovie.o     $(NST_DIR)/source/core/api/NstApiTapeRecorder.o
OBJS += $(NST_DIR)/source/core/api/NstApiCartridge.o      $(NST_DIR)/source/core/api/NstApiFds.o       $(NST_DIR)/source/core/api/NstApiNsf.o       $(NST_DIR)/source/core/api/NstApiUser.o
OBJS += $(NST_DIR)/source/core/api/NstApiCheats.o         $(NST_DIR)/source/core/api/NstApiInput.o     $(NST_DIR)/source/core/api/NstApiRewinder.o  $(NST_DIR)/source/core/api/NstApiVideo.o
OBJS += $(NST_DIR)/source/core/api/NstApiDipSwitches.o    $(NST_DIR)/source/core/api/NstApiMachine.o   $(NST_DIR)/source/core/api/NstApiSound.o

# core/board
OBJS += $(NST_DIR)/source/core/board/NstBoardAe.o                    $(NST_DIR)/source/core/board/NstBoardBtlPikachuY2k.o          $(NST_DIR)/source/core/board/NstBoardNihon.o
OBJS += $(NST_DIR)/source/core/board/NstBoardAgci.o                  $(NST_DIR)/source/core/board/NstBoardBtlShuiGuanPipe.o        $(NST_DIR)/source/core/board/NstBoardNitra.o
OBJS += $(NST_DIR)/source/core/board/NstBoardAveD1012.o              $(NST_DIR)/source/core/board/NstBoardBtlSmb2a.o               $(NST_DIR)/source/core/board/NstBoardNtdec.o
OBJS += $(NST_DIR)/source/core/board/NstBoardAveNina.o               $(NST_DIR)/source/core/board/NstBoardBtlSmb2b.o               $(NST_DIR)/source/core/board/NstBoardOpenCorp.o
OBJS += $(NST_DIR)/source/core/board/NstBoardAxRom.o                 $(NST_DIR)/source/core/board/NstBoardBtlSmb2c.o               $(NST_DIR)/source/core/board/NstBoardQj.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandai24c0x.o           $(NST_DIR)/source/core/board/NstBoardBtlSmb3.o                $(NST_DIR)/source/core/board/NstBoardRcm.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiAerobicsStudio.o  $(NST_DIR)/source/core/board/NstBoardBtlSuperBros11.o         $(NST_DIR)/source/core/board/NstBoardRexSoftDb5z.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiDatach.o          $(NST_DIR)/source/core/board/NstBoardBtlT230.o                $(NST_DIR)/source/core/board/NstBoardRexSoftSl1632.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiKaraokeStudio.o   $(NST_DIR)/source/core/board/NstBoardBtlTobidaseDaisakusen.o  $(NST_DIR)/source/core/board/NstBoardRumbleStation.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiLz93d50.o         $(NST_DIR)/source/core/board/NstBoardBxRom.o                  $(NST_DIR)/source/core/board/NstBoardSachen74x374.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiLz93d50ex.o       $(NST_DIR)/source/core/board/NstBoardCaltron.o                $(NST_DIR)/source/core/board/NstBoardSachenS8259.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiOekaKids.o        $(NST_DIR)/source/core/board/NstBoardCamerica.o               $(NST_DIR)/source/core/board/NstBoardSachenSa0036.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBenshengBs5.o           $(NST_DIR)/source/core/board/NstBoardCneDecathlon.o           $(NST_DIR)/source/core/board/NstBoardSachenSa0037.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc110in1.o             $(NST_DIR)/source/core/board/NstBoardCnePsb.o                 $(NST_DIR)/source/core/board/NstBoardSachenSa72007.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc1200in1.o            $(NST_DIR)/source/core/board/NstBoardCneShlz.o                $(NST_DIR)/source/core/board/NstBoardSachenSa72008.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc150in1.o             $(NST_DIR)/source/core/board/NstBoardCony.o                   $(NST_DIR)/source/core/board/NstBoardSachenStreetHeroes.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc15in1.o              $(NST_DIR)/source/core/board/NstBoard.o                       $(NST_DIR)/source/core/board/NstBoardSachenTca01.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc20in1.o              $(NST_DIR)/source/core/board/NstBoardCxRom.o                  $(NST_DIR)/source/core/board/NstBoardSachenTcu.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc21in1.o              $(NST_DIR)/source/core/board/NstBoardDiscrete.o               $(NST_DIR)/source/core/board/NstBoardSomeriTeamSl12.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc22Games.o            $(NST_DIR)/source/core/board/NstBoardDreamtech.o              $(NST_DIR)/source/core/board/NstBoardSubor.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc31in1.o              $(NST_DIR)/source/core/board/NstBoardEvent.o                  $(NST_DIR)/source/core/board/NstBoardSunsoft1.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc35in1.o              $(NST_DIR)/source/core/board/NstBoardFb.o                     $(NST_DIR)/source/core/board/NstBoardSunsoft2.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc36in1.o              $(NST_DIR)/source/core/board/NstBoardFfe.o                    $(NST_DIR)/source/core/board/NstBoardSunsoft3.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc64in1.o              $(NST_DIR)/source/core/board/NstBoardFujiya.o                 $(NST_DIR)/source/core/board/NstBoardSunsoft4.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc72in1.o              $(NST_DIR)/source/core/board/NstBoardFukutake.o               $(NST_DIR)/source/core/board/NstBoardSunsoft5b.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc76in1.o              $(NST_DIR)/source/core/board/NstBoardFutureMedia.o            $(NST_DIR)/source/core/board/NstBoardSunsoftDcs.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc800in1.o             $(NST_DIR)/source/core/board/NstBoardGouder.o                 $(NST_DIR)/source/core/board/NstBoardSunsoftFme7.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc8157.o               $(NST_DIR)/source/core/board/NstBoardGxRom.o                  $(NST_DIR)/source/core/board/NstBoardSuperGameBoogerman.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc9999999in1.o         $(NST_DIR)/source/core/board/NstBoardHenggedianzi.o           $(NST_DIR)/source/core/board/NstBoardSuperGameLionKing.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcA65as.o              $(NST_DIR)/source/core/board/NstBoardHes.o                    $(NST_DIR)/source/core/board/NstBoardSuperGamePocahontas2.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcBallgames11in1.o     $(NST_DIR)/source/core/board/NstBoardHosenkan.o               $(NST_DIR)/source/core/board/NstBoardTaitoTc0190fmc.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcCh001.o              $(NST_DIR)/source/core/board/NstBoardIremG101.o               $(NST_DIR)/source/core/board/NstBoardTaitoTc0190fmcPal16r4.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcCtc65.o              $(NST_DIR)/source/core/board/NstBoardIremH3001.o              $(NST_DIR)/source/core/board/NstBoardTaitoX1005.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcFamily4646B.o        $(NST_DIR)/source/core/board/NstBoardIremHolyDiver.o          $(NST_DIR)/source/core/board/NstBoardTaitoX1017.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcFk23c.o              $(NST_DIR)/source/core/board/NstBoardIremKaiketsu.o           $(NST_DIR)/source/core/board/NstBoardTengen.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGamestarA.o          $(NST_DIR)/source/core/board/NstBoardIremLrog017.o            $(NST_DIR)/source/core/board/NstBoardTengenRambo1.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGamestarB.o          $(NST_DIR)/source/core/board/NstBoardJalecoJf11.o             $(NST_DIR)/source/core/board/NstBoardTxc.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGolden190in1.o       $(NST_DIR)/source/core/board/NstBoardJalecoJf13.o             $(NST_DIR)/source/core/board/NstBoardTxcMxmdhtwo.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGoldenCard6in1.o     $(NST_DIR)/source/core/board/NstBoardJalecoJf16.o             $(NST_DIR)/source/core/board/NstBoardTxcPoliceman.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGoldenGame260in1.o   $(NST_DIR)/source/core/board/NstBoardJalecoJf17.o             $(NST_DIR)/source/core/board/NstBoardTxcTw.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcHero.o               $(NST_DIR)/source/core/board/NstBoardJalecoJf19.o             $(NST_DIR)/source/core/board/NstBoardTxRom.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcMarioParty7in1.o     $(NST_DIR)/source/core/board/NstBoardJalecoSs88006.o          $(NST_DIR)/source/core/board/NstBoardUnlA9746.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcNovelDiamond.o       $(NST_DIR)/source/core/board/NstBoardJyCompany.o              $(NST_DIR)/source/core/board/NstBoardUnlCc21.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcPowerjoy84in1.o      $(NST_DIR)/source/core/board/NstBoardKaiser.o                 $(NST_DIR)/source/core/board/NstBoardUnlEdu2000.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcResetBased4in1.o     $(NST_DIR)/source/core/board/NstBoardKasing.o                 $(NST_DIR)/source/core/board/NstBoardUnlKingOfFighters96.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper22Games.o       $(NST_DIR)/source/core/board/NstBoardKayH2288.o               $(NST_DIR)/source/core/board/NstBoardUnlKingOfFighters97.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper24in1.o         $(NST_DIR)/source/core/board/NstBoardKayPandaPrince.o         $(NST_DIR)/source/core/board/NstBoardUnlMortalKombat2.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper40in1.o         $(NST_DIR)/source/core/board/NstBoardKonamiVrc1.o             $(NST_DIR)/source/core/board/NstBoardUnlN625092.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper700in1.o        $(NST_DIR)/source/core/board/NstBoardKonamiVrc2.o             $(NST_DIR)/source/core/board/NstBoardUnlSuperFighter3.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperBig7in1.o       $(NST_DIR)/source/core/board/NstBoardKonamiVrc3.o             $(NST_DIR)/source/core/board/NstBoardUnlTf1201.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperGun20in1.o      $(NST_DIR)/source/core/board/NstBoardKonamiVrc4.o             $(NST_DIR)/source/core/board/NstBoardUnlWorldHero.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperHiK300in1.o     $(NST_DIR)/source/core/board/NstBoardKonamiVrc6.o             $(NST_DIR)/source/core/board/NstBoardUnlXzy.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperHiK4in1.o       $(NST_DIR)/source/core/board/NstBoardKonamiVrc7.o             $(NST_DIR)/source/core/board/NstBoardUxRom.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperVision16in1.o   $(NST_DIR)/source/core/board/NstBoardKonamiVsSystem.o         $(NST_DIR)/source/core/board/NstBoardVsSystem.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcT262.o               $(NST_DIR)/source/core/board/NstBoardMagicSeries.o            $(NST_DIR)/source/core/board/NstBoardWaixing.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcVrc4.o               $(NST_DIR)/source/core/board/NstBoardMmc1.o                   $(NST_DIR)/source/core/board/NstBoardWaixingFfv.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcVt5201.o             $(NST_DIR)/source/core/board/NstBoardMmc2.o                   $(NST_DIR)/source/core/board/NstBoardWaixingPs2.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcY2k64in1.o           $(NST_DIR)/source/core/board/NstBoardMmc3.o                   $(NST_DIR)/source/core/board/NstBoardWaixingSecurity.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBtl2708.o               $(NST_DIR)/source/core/board/NstBoardMmc4.o                   $(NST_DIR)/source/core/board/NstBoardWaixingSgz.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBtl6035052.o            $(NST_DIR)/source/core/board/NstBoardMmc5.o                   $(NST_DIR)/source/core/board/NstBoardWaixingSgzlz.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlAx5705.o             $(NST_DIR)/source/core/board/NstBoardMmc6.o                   $(NST_DIR)/source/core/board/NstBoardWaixingSh2.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlDragonNinja.o        $(NST_DIR)/source/core/board/NstBoardNamcot163.o              $(NST_DIR)/source/core/board/NstBoardWaixingZs.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlGeniusMerioBros.o    $(NST_DIR)/source/core/board/NstBoardNamcot34xx.o             $(NST_DIR)/source/core/board/NstBoardWhirlwind.o
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlMarioBaby.o          $(NST_DIR)/source/core/board/NstBoardNanjing.o                $(NST_DIR)/source/core/board/NstBoardZz.o

# core/input
OBJS += $(NST_DIR)/source/core/input/NstInpAdapter.o            $(NST_DIR)/source/core/input/NstInpKonamiHyperShot.o  $(NST_DIR)/source/core/input/NstInpPowerGlove.o
OBJS += $(NST_DIR)/source/core/input/NstInpBandaiHyperShot.o    $(NST_DIR)/source/core/input/NstInpMahjong.o          $(NST_DIR)/source/core/input/NstInpPowerPad.o
OBJS += $(NST_DIR)/source/core/input/NstInpBarcodeWorld.o       $(NST_DIR)/source/core/input/NstInpMouse.o            $(NST_DIR)/source/core/input/NstInpRob.o
OBJS += $(NST_DIR)/source/core/input/NstInpCrazyClimber.o       $(NST_DIR)/source/core/input/NstInpOekaKidsTablet.o   $(NST_DIR)/source/core/input/NstInpSuborKeyboard.o
OBJS += $(NST_DIR)/source/core/input/NstInpDoremikkoKeyboard.o  $(NST_DIR)/source/core/input/NstInpPachinko.o         $(NST_DIR)/source/core/input/NstInpTopRider.o
OBJS += $(NST_DIR)/source/core/input/NstInpExcitingBoxing.o     $(NST_DIR)/source/core/input/NstInpPad.o              $(NST_DIR)/source/core/input/NstInpTurboFile.o
OBJS += $(NST_DIR)/source/core/input/NstInpFamilyKeyboard.o     $(NST_DIR)/source/core/input/NstInpPaddle.o           $(NST_DIR)/source/core/input/NstInpZapper.o
OBJS += $(NST_DIR)/source/core/input/NstInpFamilyTrainer.o      $(NST_DIR)/source/core/input/NstInpPartyTap.o
OBJS += $(NST_DIR)/source/core/input/NstInpHoriTrack.o          $(NST_DIR)/source/core/input/NstInpPokkunMoguraa.o

# core/vssystem
OBJS += $(NST_DIR)/source/core/vssystem/NstVsRbiBaseball.o  $(NST_DIR)/source/core/vssystem/NstVsSuperXevious.o  $(NST_DIR)/source/core/vssystem/NstVsSystem.o  $(NST_DIR)/source/core/vssystem/NstVsTkoBoxing.o

OBJS += libretro.o

OBJS := $(strip $(OBJS))
LIBS := -lz

ifeq ($(HAVE_GCC_WARNINGS),1)
GCC_WARNINGS := -Wno-unused-result
endif

DEFINES := -D__LIBRETRO__ $(PLATFORM_DEFINES) -Wno-deprecated $(GCC_WARNINGS) -Wno-write-strings

CFLAGS += $(fpic) $(DEFINES) -std=gnu99
CXXFLAGS += $(fpic) $(DEFINES) -fno-rtti

INCDIRS := -I.. -I$(NST_DIR)/source

$(TARGET): $(OBJS)
ifeq ($(platform), ps3)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), sncps3)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), psl1ght)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), xenon)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), ngc)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), wii)
	$(AR) rcs $@ $(OBJS)
else
	$(CXX) -o $@ $(SHARED) $(OBJS) $(LDFLAGS) $(LIBS)
endif

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCDIRS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCDIRS)

clean-objs:
	rm -f $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

.PHONY: clean clean-objs

