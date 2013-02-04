#
# Makefile for Linux Nestopia
# By R. Belmont 2008
# By R. Danbrook 2012
#

CC   = @gcc
CXX  = @g++
CFLAGS ?= -O3 -g3
CXXFLAGS ?= -O3 -g3
CPPFLAGS += -DNST_PRAGMA_ONCE_SUPPORT -D_SZ_ONE_DIRECTORY
CPPFLAGS += -Isource -Isource/core -Isource/zlib -Isource/core/api -Isource/core/board -Isource/core/input -Isource/unix/unzip
CPPFLAGS += -Isource/core/vssystem -Isource/unix -Isource/nes_ntsc -I.. -I../nes_ntsc -Isource/unix/7zip
SDL_CFLAGS = $(shell sdl-config --cflags)
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
CFLAGS += $(SDL_CFLAGS) $(GTK_CFLAGS)
CFLAGS += -finline-limit=2000 --param inline-unit-growth=1000 --param large-function-growth=1000 -finline-functions-called-once
CXXFLAGS += $(SDL_CFLAGS) $(GTK_CFLAGS)
CXXFLAGS += -finline-limit=2000 --param inline-unit-growth=1000 --param large-function-growth=1000 -finline-functions-called-once
UNAME := $(shell uname)

# enable this for input debugging
#CFLAGS += -DDEBUG_INPUT

LDFLAGS += -Wl,--as-needed

BIN  = nestopia

ifeq ($(UNAME), Linux)
	CXXFLAGS += -Wno-deprecated -Wno-unused-result -Wno-write-strings -fno-rtti
	LIBS = -lstdc++ -lm -lz -lasound $(shell sdl-config --libs) $(shell pkg-config --libs gtk+-3.0)
endif
ifneq ($(UNAME), Linux)
	CXXFLAGS += -Wno-deprecated -Wno-write-strings -fno-rtti
	LIBS = -lstdc++ -lm -lz $(shell sdl-config --libs) $(shell pkg-config --libs gtk+-3.0)
	CPPFLAGS += -DBSD
endif

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/nestopia

# OpenGL Support
CPPFLAGS += -DINCLUDE_OPENGL
LIBS   += -lGL -lGLU -lX11

# Allow files to go into a data directory
CPPFLAGS += -DDATADIR=\"$(DATADIR)\"

# Linux objs
OBJS = objs/unix/main.o objs/unix/oss.o objs/unix/interface.o objs/unix/settings.o 
OBJS += objs/unix/auxio.o objs/unix/input.o objs/unix/kentry.o objs/unix/controlconfig.o objs/unix/cheats.o
OBJS += objs/unix/seffect.o objs/unix/uihelp.o

# 7-zip decoder (from LZMA SDK 4.58 beta)
OBJS += objs/unix/7zip/7zAlloc.o objs/unix/7zip/7zBuf.o objs/unix/7zip/7zCrc.o objs/unix/7zip/7zDecode.o objs/unix/7zip/7zExtract.o 
OBJS += objs/unix/7zip/7zHeader.o objs/unix/7zip/7zIn.o objs/unix/7zip/7zItem.o objs/unix/7zip/LzmaDec.o
OBJS += objs/unix/7zip/Alloc.o objs/unix/7zip/Bcj2.o objs/unix/7zip/Bra.o objs/unix/7zip/Bra86.o objs/unix/7zip/BraIA64.o

# zip decoder
OBJS += objs/unix/unzip/unzip.o

# core objs
OBJS += objs/core/NstApu.o              objs/core/NstFds.o            objs/core/NstPpu.o              objs/core/NstVector.o
OBJS += objs/core/NstAssert.o           objs/core/NstFile.o           objs/core/NstProperties.o       objs/core/NstVideoFilter2xSaI.o
OBJS += objs/core/NstCartridge.o        objs/core/NstImage.o          objs/core/NstRam.o              objs/core/NstVideoFilterHqX.o
OBJS += objs/core/NstCartridgeInes.o    objs/core/NstImageDatabase.o  objs/core/NstSha1.o             objs/core/NstVideoFilterNone.o
OBJS += objs/core/NstCartridgeRomset.o  objs/core/NstLog.o            objs/core/NstSoundPcm.o         objs/core/NstVideoFilterNtsc.o
OBJS += objs/core/NstCartridgeUnif.o    objs/core/NstMachine.o        objs/core/NstSoundPlayer.o      objs/core/NstVideoFilterScaleX.o	objs/core/NstVideoFilterxBR.o
OBJS += objs/core/NstCheats.o           objs/core/NstMemory.o         objs/core/NstSoundRenderer.o    objs/core/NstVideoRenderer.o
OBJS += objs/core/NstChecksum.o         objs/core/NstNsf.o            objs/core/NstState.o            objs/core/NstVideoScreen.o
OBJS += objs/core/NstChips.o            objs/core/NstPatcher.o        objs/core/NstStream.o           objs/core/NstXml.o
OBJS += objs/core/NstCore.o             objs/core/NstPatcherIps.o     objs/core/NstTracker.o          objs/core/NstZlib.o
OBJS += objs/core/NstCpu.o              objs/core/NstPatcherUps.o     objs/core/NstTrackerMovie.o
OBJS += objs/core/NstCrc32.o            objs/core/NstPins.o           objs/core/NstTrackerRewinder.o
OBJS += objs/core/NstVideoFilterNtscCfg.o

# core/api
OBJS += objs/core/api/NstApiBarcodeReader.o  objs/core/api/NstApiEmulator.o  objs/core/api/NstApiMovie.o     objs/core/api/NstApiTapeRecorder.o
OBJS += objs/core/api/NstApiCartridge.o      objs/core/api/NstApiFds.o       objs/core/api/NstApiNsf.o       objs/core/api/NstApiUser.o
OBJS += objs/core/api/NstApiCheats.o         objs/core/api/NstApiInput.o     objs/core/api/NstApiRewinder.o  objs/core/api/NstApiVideo.o
OBJS += objs/core/api/NstApiDipSwitches.o    objs/core/api/NstApiMachine.o   objs/core/api/NstApiSound.o


# core/board
OBJS += objs/core/board/NstBoardAe.o                    objs/core/board/NstBoardBtlPikachuY2k.o          objs/core/board/NstBoardNihon.o
OBJS += objs/core/board/NstBoardAgci.o                  objs/core/board/NstBoardBtlShuiGuanPipe.o        objs/core/board/NstBoardNitra.o
OBJS += objs/core/board/NstBoardAveD1012.o              objs/core/board/NstBoardBtlSmb2a.o               objs/core/board/NstBoardNtdec.o
OBJS += objs/core/board/NstBoardAveNina.o               objs/core/board/NstBoardBtlSmb2b.o               objs/core/board/NstBoardOpenCorp.o
OBJS += objs/core/board/NstBoardAxRom.o                 objs/core/board/NstBoardBtlSmb2c.o               objs/core/board/NstBoardQj.o
OBJS += objs/core/board/NstBoardBandai24c0x.o           objs/core/board/NstBoardBtlSmb3.o                objs/core/board/NstBoardRcm.o
OBJS += objs/core/board/NstBoardBandaiAerobicsStudio.o  objs/core/board/NstBoardBtlSuperBros11.o         objs/core/board/NstBoardRexSoftDb5z.o
OBJS += objs/core/board/NstBoardBandaiDatach.o          objs/core/board/NstBoardBtlT230.o                objs/core/board/NstBoardRexSoftSl1632.o
OBJS += objs/core/board/NstBoardBandaiKaraokeStudio.o   objs/core/board/NstBoardBtlTobidaseDaisakusen.o  objs/core/board/NstBoardRumbleStation.o
OBJS += objs/core/board/NstBoardBandaiLz93d50.o         objs/core/board/NstBoardBxRom.o                  objs/core/board/NstBoardSachen74x374.o
OBJS += objs/core/board/NstBoardBandaiLz93d50ex.o       objs/core/board/NstBoardCaltron.o                objs/core/board/NstBoardSachenS8259.o
OBJS += objs/core/board/NstBoardBandaiOekaKids.o        objs/core/board/NstBoardCamerica.o               objs/core/board/NstBoardSachenSa0036.o
OBJS += objs/core/board/NstBoardBenshengBs5.o           objs/core/board/NstBoardCneDecathlon.o           objs/core/board/NstBoardSachenSa0037.o
OBJS += objs/core/board/NstBoardBmc110in1.o             objs/core/board/NstBoardCnePsb.o                 objs/core/board/NstBoardSachenSa72007.o
OBJS += objs/core/board/NstBoardBmc1200in1.o            objs/core/board/NstBoardCneShlz.o                objs/core/board/NstBoardSachenSa72008.o
OBJS += objs/core/board/NstBoardBmc150in1.o             objs/core/board/NstBoardCony.o                   objs/core/board/NstBoardSachenStreetHeroes.o
OBJS += objs/core/board/NstBoardBmc15in1.o              objs/core/board/NstBoard.o                       objs/core/board/NstBoardSachenTca01.o
OBJS += objs/core/board/NstBoardBmc20in1.o              objs/core/board/NstBoardCxRom.o                  objs/core/board/NstBoardSachenTcu.o
OBJS += objs/core/board/NstBoardBmc21in1.o              objs/core/board/NstBoardDiscrete.o               objs/core/board/NstBoardSomeriTeamSl12.o
OBJS += objs/core/board/NstBoardBmc22Games.o            objs/core/board/NstBoardDreamtech.o              objs/core/board/NstBoardSubor.o
OBJS += objs/core/board/NstBoardBmc31in1.o              objs/core/board/NstBoardEvent.o                  objs/core/board/NstBoardSunsoft1.o
OBJS += objs/core/board/NstBoardBmc35in1.o              objs/core/board/NstBoardFb.o                     objs/core/board/NstBoardSunsoft2.o
OBJS += objs/core/board/NstBoardBmc36in1.o              objs/core/board/NstBoardFfe.o                    objs/core/board/NstBoardSunsoft3.o
OBJS += objs/core/board/NstBoardBmc64in1.o              objs/core/board/NstBoardFujiya.o                 objs/core/board/NstBoardSunsoft4.o
OBJS += objs/core/board/NstBoardBmc72in1.o              objs/core/board/NstBoardFukutake.o               objs/core/board/NstBoardSunsoft5b.o
OBJS += objs/core/board/NstBoardBmc76in1.o              objs/core/board/NstBoardFutureMedia.o            objs/core/board/NstBoardSunsoftDcs.o
OBJS += objs/core/board/NstBoardBmc800in1.o             objs/core/board/NstBoardGouder.o                 objs/core/board/NstBoardSunsoftFme7.o
OBJS += objs/core/board/NstBoardBmc8157.o               objs/core/board/NstBoardGxRom.o                  objs/core/board/NstBoardSuperGameBoogerman.o
OBJS += objs/core/board/NstBoardBmc9999999in1.o         objs/core/board/NstBoardHenggedianzi.o           objs/core/board/NstBoardSuperGameLionKing.o
OBJS += objs/core/board/NstBoardBmcA65as.o              objs/core/board/NstBoardHes.o                    objs/core/board/NstBoardSuperGamePocahontas2.o
OBJS += objs/core/board/NstBoardBmcBallgames11in1.o     objs/core/board/NstBoardHosenkan.o               objs/core/board/NstBoardTaitoTc0190fmc.o
OBJS += objs/core/board/NstBoardBmcCh001.o              objs/core/board/NstBoardIremG101.o               objs/core/board/NstBoardTaitoTc0190fmcPal16r4.o
OBJS += objs/core/board/NstBoardBmcCtc65.o              objs/core/board/NstBoardIremH3001.o              objs/core/board/NstBoardTaitoX1005.o
OBJS += objs/core/board/NstBoardBmcFamily4646B.o        objs/core/board/NstBoardIremHolyDiver.o          objs/core/board/NstBoardTaitoX1017.o
OBJS += objs/core/board/NstBoardBmcFk23c.o              objs/core/board/NstBoardIremKaiketsu.o           objs/core/board/NstBoardTengen.o
OBJS += objs/core/board/NstBoardBmcGamestarA.o          objs/core/board/NstBoardIremLrog017.o            objs/core/board/NstBoardTengenRambo1.o
OBJS += objs/core/board/NstBoardBmcGamestarB.o          objs/core/board/NstBoardJalecoJf11.o             objs/core/board/NstBoardTxc.o
OBJS += objs/core/board/NstBoardBmcGolden190in1.o       objs/core/board/NstBoardJalecoJf13.o             objs/core/board/NstBoardTxcMxmdhtwo.o
OBJS += objs/core/board/NstBoardBmcGoldenCard6in1.o     objs/core/board/NstBoardJalecoJf16.o             objs/core/board/NstBoardTxcPoliceman.o
OBJS += objs/core/board/NstBoardBmcGoldenGame260in1.o   objs/core/board/NstBoardJalecoJf17.o             objs/core/board/NstBoardTxcTw.o
OBJS += objs/core/board/NstBoardBmcHero.o               objs/core/board/NstBoardJalecoJf19.o             objs/core/board/NstBoardTxRom.o
OBJS += objs/core/board/NstBoardBmcMarioParty7in1.o     objs/core/board/NstBoardJalecoSs88006.o          objs/core/board/NstBoardUnlA9746.o
OBJS += objs/core/board/NstBoardBmcNovelDiamond.o       objs/core/board/NstBoardJyCompany.o              objs/core/board/NstBoardUnlCc21.o
OBJS += objs/core/board/NstBoardBmcPowerjoy84in1.o      objs/core/board/NstBoardKaiser.o                 objs/core/board/NstBoardUnlEdu2000.o
OBJS += objs/core/board/NstBoardBmcResetBased4in1.o     objs/core/board/NstBoardKasing.o                 objs/core/board/NstBoardUnlKingOfFighters96.o
OBJS += objs/core/board/NstBoardBmcSuper22Games.o       objs/core/board/NstBoardKayH2288.o               objs/core/board/NstBoardUnlKingOfFighters97.o
OBJS += objs/core/board/NstBoardBmcSuper24in1.o         objs/core/board/NstBoardKayPandaPrince.o         objs/core/board/NstBoardUnlMortalKombat2.o
OBJS += objs/core/board/NstBoardBmcSuper40in1.o         objs/core/board/NstBoardKonamiVrc1.o             objs/core/board/NstBoardUnlN625092.o
OBJS += objs/core/board/NstBoardBmcSuper700in1.o        objs/core/board/NstBoardKonamiVrc2.o             objs/core/board/NstBoardUnlSuperFighter3.o
OBJS += objs/core/board/NstBoardBmcSuperBig7in1.o       objs/core/board/NstBoardKonamiVrc3.o             objs/core/board/NstBoardUnlTf1201.o
OBJS += objs/core/board/NstBoardBmcSuperGun20in1.o      objs/core/board/NstBoardKonamiVrc4.o             objs/core/board/NstBoardUnlWorldHero.o
OBJS += objs/core/board/NstBoardBmcSuperHiK300in1.o     objs/core/board/NstBoardKonamiVrc6.o             objs/core/board/NstBoardUnlXzy.o
OBJS += objs/core/board/NstBoardBmcSuperHiK4in1.o       objs/core/board/NstBoardKonamiVrc7.o             objs/core/board/NstBoardUxRom.o
OBJS += objs/core/board/NstBoardBmcSuperVision16in1.o   objs/core/board/NstBoardKonamiVsSystem.o         objs/core/board/NstBoardVsSystem.o
OBJS += objs/core/board/NstBoardBmcT262.o               objs/core/board/NstBoardMagicSeries.o            objs/core/board/NstBoardWaixing.o
OBJS += objs/core/board/NstBoardBmcVrc4.o               objs/core/board/NstBoardMmc1.o                   objs/core/board/NstBoardWaixingFfv.o
OBJS += objs/core/board/NstBoardBmcVt5201.o             objs/core/board/NstBoardMmc2.o                   objs/core/board/NstBoardWaixingPs2.o
OBJS += objs/core/board/NstBoardBmcY2k64in1.o           objs/core/board/NstBoardMmc3.o                   objs/core/board/NstBoardWaixingSecurity.o
OBJS += objs/core/board/NstBoardBtl2708.o               objs/core/board/NstBoardMmc4.o                   objs/core/board/NstBoardWaixingSgz.o
OBJS += objs/core/board/NstBoardBtl6035052.o            objs/core/board/NstBoardMmc5.o                   objs/core/board/NstBoardWaixingSgzlz.o
OBJS += objs/core/board/NstBoardBtlAx5705.o             objs/core/board/NstBoardMmc6.o                   objs/core/board/NstBoardWaixingSh2.o
OBJS += objs/core/board/NstBoardBtlDragonNinja.o        objs/core/board/NstBoardNamcot163.o              objs/core/board/NstBoardWaixingZs.o
OBJS += objs/core/board/NstBoardBtlGeniusMerioBros.o    objs/core/board/NstBoardNamcot34xx.o             objs/core/board/NstBoardWhirlwind.o
OBJS += objs/core/board/NstBoardBtlMarioBaby.o          objs/core/board/NstBoardNanjing.o                objs/core/board/NstBoardZz.o

# core/input
OBJS += objs/core/input/NstInpAdapter.o            objs/core/input/NstInpKonamiHyperShot.o  objs/core/input/NstInpPowerGlove.o
OBJS += objs/core/input/NstInpBandaiHyperShot.o    objs/core/input/NstInpMahjong.o          objs/core/input/NstInpPowerPad.o
OBJS += objs/core/input/NstInpBarcodeWorld.o       objs/core/input/NstInpMouse.o            objs/core/input/NstInpRob.o
OBJS += objs/core/input/NstInpCrazyClimber.o       objs/core/input/NstInpOekaKidsTablet.o   objs/core/input/NstInpSuborKeyboard.o
OBJS += objs/core/input/NstInpDoremikkoKeyboard.o  objs/core/input/NstInpPachinko.o         objs/core/input/NstInpTopRider.o
OBJS += objs/core/input/NstInpExcitingBoxing.o     objs/core/input/NstInpPad.o              objs/core/input/NstInpTurboFile.o
OBJS += objs/core/input/NstInpFamilyKeyboard.o     objs/core/input/NstInpPaddle.o           objs/core/input/NstInpZapper.o
OBJS += objs/core/input/NstInpFamilyTrainer.o      objs/core/input/NstInpPartyTap.o
OBJS += objs/core/input/NstInpHoriTrack.o          objs/core/input/NstInpPokkunMoguraa.o


# core/vssystem
OBJS += objs/core/vssystem/NstVsRbiBaseball.o  objs/core/vssystem/NstVsSuperXevious.o  objs/core/vssystem/NstVsSystem.o  objs/core/vssystem/NstVsTkoBoxing.o

# object dirs
OBJDIRS = objs objs/core objs/core/api objs/core/board objs/core/input objs/core/vssystem objs/nes_ntsc 
OBJDIRS += objs/unix objs/unix/7zip objs/unix/unzip

# build rules
objs/%.o: source/%.c
	@echo Compiling $<...
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

objs/%.o: source/%.cpp
	@echo Compiling $<...
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

all: maketree $(BIN)

maketree: $(sort $(OBJDIRS))

$(sort $(OBJDIRS)):
	@echo Creating output directory $@
	@mkdir $@

# link the commandline binary
$(BIN): $(OBJS)
	@echo Linking $@...
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN) $^ $(LIBS)

install:
	mkdir -p $(DATADIR)/icons
	mkdir -p $(PREFIX)/share/pixmaps
	install -m 0755 $(BIN) $(BINDIR)
	install -m 0644 source/unix/icons/nestopia.desktop $(DATADIR)
	install -m 0644 NstDatabase.xml $(DATADIR)
	install -m 0644 source/unix/icons/*.png $(DATADIR)/icons
	install -m 0644 source/unix/icons/*.svg $(DATADIR)/icons
	install -m 0644 source/unix/icons/nestopia.svg $(PREFIX)/share/pixmaps
	xdg-desktop-menu install --novendor $(DATADIR)/nestopia.desktop

uninstall:
	xdg-desktop-menu uninstall $(DATADIR)/nestopia.desktop
	rm $(PREFIX)/share/pixmaps/nestopia.svg
	rm $(BINDIR)/$(BIN)
	rm -rf $(DATADIR)

clean:
	-@rm -f $(OBJS) $(BIN)
