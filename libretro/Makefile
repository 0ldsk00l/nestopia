DEBUG = 0

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
   TARGET := libretro.a
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   PLATFORM_DEFINES := -D__CELLOS_LV2__
else ifeq ($(platform), sncps3)
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

OBJS = ../source/core/NstZlib.o

## Yes, I didn't type all this out ...
# core objs
OBJS += ../source/core/NstApu.o              ../source/core/NstFds.o            ../source/core/NstPpu.o              ../source/core/NstVector.o
OBJS += ../source/core/NstAssert.o           ../source/core/NstFile.o           ../source/core/NstProperties.o       ../source/core/NstVideoFilter2xSaI.o
OBJS += ../source/core/NstCartridge.o        ../source/core/NstImage.o          ../source/core/NstRam.o              ../source/core/NstVideoFilterHqX.o
OBJS += ../source/core/NstCartridgeInes.o    ../source/core/NstImageDatabase.o  ../source/core/NstSha1.o             ../source/core/NstVideoFilterNone.o
OBJS += ../source/core/NstCartridgeRomset.o  ../source/core/NstLog.o            ../source/core/NstSoundPcm.o         ../source/core/NstVideoFilterNtsc.o
OBJS += ../source/core/NstCartridgeUnif.o    ../source/core/NstMachine.o        ../source/core/NstSoundPlayer.o      ../source/core/NstVideoFilterScaleX.o
OBJS += ../source/core/NstCheats.o           ../source/core/NstMemory.o         ../source/core/NstSoundRenderer.o    ../source/core/NstVideoRenderer.o
OBJS += ../source/core/NstChecksum.o         ../source/core/NstNsf.o            ../source/core/NstState.o            ../source/core/NstVideoScreen.o
OBJS += ../source/core/NstChips.o            ../source/core/NstPatcher.o        ../source/core/NstStream.o           ../source/core/NstXml.o
OBJS += ../source/core/NstCore.o             ../source/core/NstPatcherIps.o     ../source/core/NstTracker.o          
OBJS += ../source/core/NstCpu.o              ../source/core/NstPatcherUps.o     ../source/core/NstTrackerMovie.o
OBJS += ../source/core/NstCrc32.o            ../source/core/NstPins.o           ../source/core/NstTrackerRewinder.o
OBJS += ../source/core/NstVideoFilterNtscCfg.o

# core/api
OBJS += ../source/core/api/NstApiBarcodeReader.o  ../source/core/api/NstApiEmulator.o  ../source/core/api/NstApiMovie.o     ../source/core/api/NstApiTapeRecorder.o
OBJS += ../source/core/api/NstApiCartridge.o      ../source/core/api/NstApiFds.o       ../source/core/api/NstApiNsf.o       ../source/core/api/NstApiUser.o
OBJS += ../source/core/api/NstApiCheats.o         ../source/core/api/NstApiInput.o     ../source/core/api/NstApiRewinder.o  ../source/core/api/NstApiVideo.o
OBJS += ../source/core/api/NstApiDipSwitches.o    ../source/core/api/NstApiMachine.o   ../source/core/api/NstApiSound.o

# core/board
OBJS += ../source/core/board/NstBoardAe.o                    ../source/core/board/NstBoardBtlPikachuY2k.o          ../source/core/board/NstBoardNihon.o
OBJS += ../source/core/board/NstBoardAgci.o                  ../source/core/board/NstBoardBtlShuiGuanPipe.o        ../source/core/board/NstBoardNitra.o
OBJS += ../source/core/board/NstBoardAveD1012.o              ../source/core/board/NstBoardBtlSmb2a.o               ../source/core/board/NstBoardNtdec.o
OBJS += ../source/core/board/NstBoardAveNina.o               ../source/core/board/NstBoardBtlSmb2b.o               ../source/core/board/NstBoardOpenCorp.o
OBJS += ../source/core/board/NstBoardAxRom.o                 ../source/core/board/NstBoardBtlSmb2c.o               ../source/core/board/NstBoardQj.o
OBJS += ../source/core/board/NstBoardBandai24c0x.o           ../source/core/board/NstBoardBtlSmb3.o                ../source/core/board/NstBoardRcm.o
OBJS += ../source/core/board/NstBoardBandaiAerobicsStudio.o  ../source/core/board/NstBoardBtlSuperBros11.o         ../source/core/board/NstBoardRexSoftDb5z.o
OBJS += ../source/core/board/NstBoardBandaiDatach.o          ../source/core/board/NstBoardBtlT230.o                ../source/core/board/NstBoardRexSoftSl1632.o
OBJS += ../source/core/board/NstBoardBandaiKaraokeStudio.o   ../source/core/board/NstBoardBtlTobidaseDaisakusen.o  ../source/core/board/NstBoardRumbleStation.o
OBJS += ../source/core/board/NstBoardBandaiLz93d50.o         ../source/core/board/NstBoardBxRom.o                  ../source/core/board/NstBoardSachen74x374.o
OBJS += ../source/core/board/NstBoardBandaiLz93d50ex.o       ../source/core/board/NstBoardCaltron.o                ../source/core/board/NstBoardSachenS8259.o
OBJS += ../source/core/board/NstBoardBandaiOekaKids.o        ../source/core/board/NstBoardCamerica.o               ../source/core/board/NstBoardSachenSa0036.o
OBJS += ../source/core/board/NstBoardBenshengBs5.o           ../source/core/board/NstBoardCneDecathlon.o           ../source/core/board/NstBoardSachenSa0037.o
OBJS += ../source/core/board/NstBoardBmc110in1.o             ../source/core/board/NstBoardCnePsb.o                 ../source/core/board/NstBoardSachenSa72007.o
OBJS += ../source/core/board/NstBoardBmc1200in1.o            ../source/core/board/NstBoardCneShlz.o                ../source/core/board/NstBoardSachenSa72008.o
OBJS += ../source/core/board/NstBoardBmc150in1.o             ../source/core/board/NstBoardCony.o                   ../source/core/board/NstBoardSachenStreetHeroes.o
OBJS += ../source/core/board/NstBoardBmc15in1.o              ../source/core/board/NstBoard.o                       ../source/core/board/NstBoardSachenTca01.o
OBJS += ../source/core/board/NstBoardBmc20in1.o              ../source/core/board/NstBoardCxRom.o                  ../source/core/board/NstBoardSachenTcu.o
OBJS += ../source/core/board/NstBoardBmc21in1.o              ../source/core/board/NstBoardDiscrete.o               ../source/core/board/NstBoardSomeriTeamSl12.o
OBJS += ../source/core/board/NstBoardBmc22Games.o            ../source/core/board/NstBoardDreamtech.o              ../source/core/board/NstBoardSubor.o
OBJS += ../source/core/board/NstBoardBmc31in1.o              ../source/core/board/NstBoardEvent.o                  ../source/core/board/NstBoardSunsoft1.o
OBJS += ../source/core/board/NstBoardBmc35in1.o              ../source/core/board/NstBoardFb.o                     ../source/core/board/NstBoardSunsoft2.o
OBJS += ../source/core/board/NstBoardBmc36in1.o              ../source/core/board/NstBoardFfe.o                    ../source/core/board/NstBoardSunsoft3.o
OBJS += ../source/core/board/NstBoardBmc64in1.o              ../source/core/board/NstBoardFujiya.o                 ../source/core/board/NstBoardSunsoft4.o
OBJS += ../source/core/board/NstBoardBmc72in1.o              ../source/core/board/NstBoardFukutake.o               ../source/core/board/NstBoardSunsoft5b.o
OBJS += ../source/core/board/NstBoardBmc76in1.o              ../source/core/board/NstBoardFutureMedia.o            ../source/core/board/NstBoardSunsoftDcs.o
OBJS += ../source/core/board/NstBoardBmc800in1.o             ../source/core/board/NstBoardGouder.o                 ../source/core/board/NstBoardSunsoftFme7.o
OBJS += ../source/core/board/NstBoardBmc8157.o               ../source/core/board/NstBoardGxRom.o                  ../source/core/board/NstBoardSuperGameBoogerman.o
OBJS += ../source/core/board/NstBoardBmc9999999in1.o         ../source/core/board/NstBoardHenggedianzi.o           ../source/core/board/NstBoardSuperGameLionKing.o
OBJS += ../source/core/board/NstBoardBmcA65as.o              ../source/core/board/NstBoardHes.o                    ../source/core/board/NstBoardSuperGamePocahontas2.o
OBJS += ../source/core/board/NstBoardBmcBallgames11in1.o     ../source/core/board/NstBoardHosenkan.o               ../source/core/board/NstBoardTaitoTc0190fmc.o
OBJS += ../source/core/board/NstBoardBmcCh001.o              ../source/core/board/NstBoardIremG101.o               ../source/core/board/NstBoardTaitoTc0190fmcPal16r4.o
OBJS += ../source/core/board/NstBoardBmcCtc65.o              ../source/core/board/NstBoardIremH3001.o              ../source/core/board/NstBoardTaitoX1005.o
OBJS += ../source/core/board/NstBoardBmcFamily4646B.o        ../source/core/board/NstBoardIremHolyDiver.o          ../source/core/board/NstBoardTaitoX1017.o
OBJS += ../source/core/board/NstBoardBmcFk23c.o              ../source/core/board/NstBoardIremKaiketsu.o           ../source/core/board/NstBoardTengen.o
OBJS += ../source/core/board/NstBoardBmcGamestarA.o          ../source/core/board/NstBoardIremLrog017.o            ../source/core/board/NstBoardTengenRambo1.o
OBJS += ../source/core/board/NstBoardBmcGamestarB.o          ../source/core/board/NstBoardJalecoJf11.o             ../source/core/board/NstBoardTxc.o
OBJS += ../source/core/board/NstBoardBmcGolden190in1.o       ../source/core/board/NstBoardJalecoJf13.o             ../source/core/board/NstBoardTxcMxmdhtwo.o
OBJS += ../source/core/board/NstBoardBmcGoldenCard6in1.o     ../source/core/board/NstBoardJalecoJf16.o             ../source/core/board/NstBoardTxcPoliceman.o
OBJS += ../source/core/board/NstBoardBmcGoldenGame260in1.o   ../source/core/board/NstBoardJalecoJf17.o             ../source/core/board/NstBoardTxcTw.o
OBJS += ../source/core/board/NstBoardBmcHero.o               ../source/core/board/NstBoardJalecoJf19.o             ../source/core/board/NstBoardTxRom.o
OBJS += ../source/core/board/NstBoardBmcMarioParty7in1.o     ../source/core/board/NstBoardJalecoSs88006.o          ../source/core/board/NstBoardUnlA9746.o
OBJS += ../source/core/board/NstBoardBmcNovelDiamond.o       ../source/core/board/NstBoardJyCompany.o              ../source/core/board/NstBoardUnlCc21.o
OBJS += ../source/core/board/NstBoardBmcPowerjoy84in1.o      ../source/core/board/NstBoardKaiser.o                 ../source/core/board/NstBoardUnlEdu2000.o
OBJS += ../source/core/board/NstBoardBmcResetBased4in1.o     ../source/core/board/NstBoardKasing.o                 ../source/core/board/NstBoardUnlKingOfFighters96.o
OBJS += ../source/core/board/NstBoardBmcSuper22Games.o       ../source/core/board/NstBoardKayH2288.o               ../source/core/board/NstBoardUnlKingOfFighters97.o
OBJS += ../source/core/board/NstBoardBmcSuper24in1.o         ../source/core/board/NstBoardKayPandaPrince.o         ../source/core/board/NstBoardUnlMortalKombat2.o
OBJS += ../source/core/board/NstBoardBmcSuper40in1.o         ../source/core/board/NstBoardKonamiVrc1.o             ../source/core/board/NstBoardUnlN625092.o
OBJS += ../source/core/board/NstBoardBmcSuper700in1.o        ../source/core/board/NstBoardKonamiVrc2.o             ../source/core/board/NstBoardUnlSuperFighter3.o
OBJS += ../source/core/board/NstBoardBmcSuperBig7in1.o       ../source/core/board/NstBoardKonamiVrc3.o             ../source/core/board/NstBoardUnlTf1201.o
OBJS += ../source/core/board/NstBoardBmcSuperGun20in1.o      ../source/core/board/NstBoardKonamiVrc4.o             ../source/core/board/NstBoardUnlWorldHero.o
OBJS += ../source/core/board/NstBoardBmcSuperHiK300in1.o     ../source/core/board/NstBoardKonamiVrc6.o             ../source/core/board/NstBoardUnlXzy.o
OBJS += ../source/core/board/NstBoardBmcSuperHiK4in1.o       ../source/core/board/NstBoardKonamiVrc7.o             ../source/core/board/NstBoardUxRom.o
OBJS += ../source/core/board/NstBoardBmcSuperVision16in1.o   ../source/core/board/NstBoardKonamiVsSystem.o         ../source/core/board/NstBoardVsSystem.o
OBJS += ../source/core/board/NstBoardBmcT262.o               ../source/core/board/NstBoardMagicSeries.o            ../source/core/board/NstBoardWaixing.o
OBJS += ../source/core/board/NstBoardBmcVrc4.o               ../source/core/board/NstBoardMmc1.o                   ../source/core/board/NstBoardWaixingFfv.o
OBJS += ../source/core/board/NstBoardBmcVt5201.o             ../source/core/board/NstBoardMmc2.o                   ../source/core/board/NstBoardWaixingPs2.o
OBJS += ../source/core/board/NstBoardBmcY2k64in1.o           ../source/core/board/NstBoardMmc3.o                   ../source/core/board/NstBoardWaixingSecurity.o
OBJS += ../source/core/board/NstBoardBtl2708.o               ../source/core/board/NstBoardMmc4.o                   ../source/core/board/NstBoardWaixingSgz.o
OBJS += ../source/core/board/NstBoardBtl6035052.o            ../source/core/board/NstBoardMmc5.o                   ../source/core/board/NstBoardWaixingSgzlz.o
OBJS += ../source/core/board/NstBoardBtlAx5705.o             ../source/core/board/NstBoardMmc6.o                   ../source/core/board/NstBoardWaixingSh2.o
OBJS += ../source/core/board/NstBoardBtlDragonNinja.o        ../source/core/board/NstBoardNamcot163.o              ../source/core/board/NstBoardWaixingZs.o
OBJS += ../source/core/board/NstBoardBtlGeniusMerioBros.o    ../source/core/board/NstBoardNamcot34xx.o             ../source/core/board/NstBoardWhirlwind.o
OBJS += ../source/core/board/NstBoardBtlMarioBaby.o          ../source/core/board/NstBoardNanjing.o                ../source/core/board/NstBoardZz.o

# core/input
OBJS += ../source/core/input/NstInpAdapter.o            ../source/core/input/NstInpKonamiHyperShot.o  ../source/core/input/NstInpPowerGlove.o
OBJS += ../source/core/input/NstInpBandaiHyperShot.o    ../source/core/input/NstInpMahjong.o          ../source/core/input/NstInpPowerPad.o
OBJS += ../source/core/input/NstInpBarcodeWorld.o       ../source/core/input/NstInpMouse.o            ../source/core/input/NstInpRob.o
OBJS += ../source/core/input/NstInpCrazyClimber.o       ../source/core/input/NstInpOekaKidsTablet.o   ../source/core/input/NstInpSuborKeyboard.o
OBJS += ../source/core/input/NstInpDoremikkoKeyboard.o  ../source/core/input/NstInpPachinko.o         ../source/core/input/NstInpTopRider.o
OBJS += ../source/core/input/NstInpExcitingBoxing.o     ../source/core/input/NstInpPad.o              ../source/core/input/NstInpTurboFile.o
OBJS += ../source/core/input/NstInpFamilyKeyboard.o     ../source/core/input/NstInpPaddle.o           ../source/core/input/NstInpZapper.o
OBJS += ../source/core/input/NstInpFamilyTrainer.o      ../source/core/input/NstInpPartyTap.o
OBJS += ../source/core/input/NstInpHoriTrack.o          ../source/core/input/NstInpPokkunMoguraa.o

# core/vssystem
OBJS += ../source/core/vssystem/NstVsRbiBaseball.o  ../source/core/vssystem/NstVsSuperXevious.o  ../source/core/vssystem/NstVsSystem.o  ../source/core/vssystem/NstVsTkoBoxing.o

OBJS += libretro.o

OBJS := $(strip $(OBJS))
LIBS := -lz

DEFINES := -D__LIBRETRO__ $(PLATFORM_DEFINES) -Wno-deprecated -Wno-unused-result -Wno-write-strings -fno-rtti

CFLAGS += $(fpic) $(DEFINES) -std=gnu99
CXXFLAGS += $(fpic) $(DEFINES)

INCDIRS := -I.. -I../source

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

