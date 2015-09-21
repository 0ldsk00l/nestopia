CC = cc
CXX = c++
CXXFLAGS ?= -O3 -g3
CPPFLAGS += -DNST_PRAGMA_ONCE
CFLAGS = $(shell sdl2-config --cflags)

INCLUDES = -Isource
WARNINGS = -Wno-write-strings

LDFLAGS = -Wl,--as-needed
LIBS = -lstdc++ -lm -lz
LIBS += $(shell sdl2-config --libs)

UNAME := $(shell uname)

BIN = nestopia

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/nestopia

ifneq ($(findstring MINGW,$(UNAME)),)
	DEFINES = -D_MINGW
	LDFLAGS += -mconsole
	LIBS += -lepoxy -lopengl32
else ifneq ($(findstring Darwin,$(UNAME)),)
	DEFINES = -D_APPLE
	DEFINES += -DDATADIR=\"$(DATADIR)\"
	INCLUDES += -I/usr/local/include -I/usr/local/opt/libarchive/include
	LDFLAGS = -Wl -L/usr/local/opt/libarchive/lib
	LIBS += -larchive -lepoxy -lao
	# GTK Stuff - Comment this section to disable GTK+
	CFLAGS += $(shell pkg-config --cflags gtk+-3.0)
	LIBS += $(shell pkg-config --libs gtk+-3.0)
	DEFINES += -D_GTK
	IOBJS += objs/unix/gtkui/gtkui.o
	IOBJS += objs/unix/gtkui/gtkui_archive.o
	IOBJS += objs/unix/gtkui/gtkui_callbacks.o
	IOBJS += objs/unix/gtkui/gtkui_cheats.o
	IOBJS += objs/unix/gtkui/gtkui_config.o
	IOBJS += objs/unix/gtkui/gtkui_dialogs.o
	OBJDIRS += objs/unix/gtkui
	WARNINGS += -Wno-deprecated-declarations
	# end GTK
else
	DEFINES = -DDATADIR=\"$(DATADIR)\"
	LIBS += -larchive -lepoxy -lGL -lGLU -lao
	# GTK Stuff - Comment this section to disable GTK+
	CFLAGS += $(shell pkg-config --cflags gtk+-3.0)
	LIBS += $(shell pkg-config --libs gtk+-3.0)
	DEFINES += -D_GTK
	IOBJS += objs/unix/gtkui/gtkui.o
	IOBJS += objs/unix/gtkui/gtkui_archive.o
	IOBJS += objs/unix/gtkui/gtkui_callbacks.o
	IOBJS += objs/unix/gtkui/gtkui_cheats.o
	IOBJS += objs/unix/gtkui/gtkui_config.o
	IOBJS += objs/unix/gtkui/gtkui_dialogs.o
	OBJDIRS += objs/unix/gtkui
	WARNINGS += -Wno-deprecated-declarations
	# end GTK
endif

# Core
OBJS += objs/core/NstApu.o
OBJS += objs/core/NstAssert.o
OBJS += objs/core/NstCartridge.o
OBJS += objs/core/NstCartridgeInes.o
OBJS += objs/core/NstCartridgeRomset.o
OBJS += objs/core/NstCartridgeUnif.o
OBJS += objs/core/NstCheats.o
OBJS += objs/core/NstChecksum.o
OBJS += objs/core/NstChips.o
OBJS += objs/core/NstCore.o
OBJS += objs/core/NstCpu.o
OBJS += objs/core/NstCrc32.o
OBJS += objs/core/NstFds.o
OBJS += objs/core/NstFile.o
OBJS += objs/core/NstImage.o
OBJS += objs/core/NstImageDatabase.o
OBJS += objs/core/NstLog.o
OBJS += objs/core/NstMachine.o
OBJS += objs/core/NstMemory.o
OBJS += objs/core/NstNsf.o
OBJS += objs/core/NstPatcher.o
OBJS += objs/core/NstPatcherIps.o
OBJS += objs/core/NstPatcherUps.o
OBJS += objs/core/NstPins.o
OBJS += objs/core/NstPpu.o
OBJS += objs/core/NstProperties.o
OBJS += objs/core/NstRam.o
OBJS += objs/core/NstSha1.o
OBJS += objs/core/NstSoundPcm.o
OBJS += objs/core/NstSoundPlayer.o
OBJS += objs/core/NstSoundRenderer.o
OBJS += objs/core/NstState.o
OBJS += objs/core/NstStream.o
OBJS += objs/core/NstTracker.o
OBJS += objs/core/NstTrackerMovie.o
OBJS += objs/core/NstTrackerRewinder.o
OBJS += objs/core/NstVector.o
OBJS += objs/core/NstVideoFilter2xSaI.o
OBJS += objs/core/NstVideoFilterHqX.o
OBJS += objs/core/NstVideoFilterNone.o
OBJS += objs/core/NstVideoFilterNtsc.o
OBJS += objs/core/NstVideoFilterNtscCfg.o
OBJS += objs/core/NstVideoFilterScaleX.o
OBJS += objs/core/NstVideoFilterxBR.o
OBJS += objs/core/NstVideoRenderer.o
OBJS += objs/core/NstVideoScreen.o
OBJS += objs/core/NstXml.o
OBJS += objs/core/NstZlib.o

# API
OBJS += objs/core/api/NstApiBarcodeReader.o
OBJS += objs/core/api/NstApiCartridge.o
OBJS += objs/core/api/NstApiCheats.o
OBJS += objs/core/api/NstApiDipSwitches.o
OBJS += objs/core/api/NstApiEmulator.o
OBJS += objs/core/api/NstApiFds.o
OBJS += objs/core/api/NstApiInput.o
OBJS += objs/core/api/NstApiMachine.o
OBJS += objs/core/api/NstApiMovie.o
OBJS += objs/core/api/NstApiNsf.o
OBJS += objs/core/api/NstApiRewinder.o
OBJS += objs/core/api/NstApiSound.o
OBJS += objs/core/api/NstApiTapeRecorder.o
OBJS += objs/core/api/NstApiUser.o
OBJS += objs/core/api/NstApiVideo.o

# Board
OBJS += objs/core/board/NstBoardAcclaimMcAcc.o
OBJS += objs/core/board/NstBoardAe.o
OBJS += objs/core/board/NstBoardAgci.o
OBJS += objs/core/board/NstBoardAveD1012.o
OBJS += objs/core/board/NstBoardAveNina.o
OBJS += objs/core/board/NstBoardAxRom.o
OBJS += objs/core/board/NstBoardBandai24c0x.o
OBJS += objs/core/board/NstBoardBandaiAerobicsStudio.o
OBJS += objs/core/board/NstBoardBandaiDatach.o
OBJS += objs/core/board/NstBoardBandaiKaraokeStudio.o
OBJS += objs/core/board/NstBoardBandaiLz93d50.o
OBJS += objs/core/board/NstBoardBandaiLz93d50ex.o
OBJS += objs/core/board/NstBoardBandaiOekaKids.o
OBJS += objs/core/board/NstBoardBenshengBs5.o
OBJS += objs/core/board/NstBoardBmc110in1.o
OBJS += objs/core/board/NstBoardBmc1200in1.o
OBJS += objs/core/board/NstBoardBmc150in1.o
OBJS += objs/core/board/NstBoardBmc15in1.o
OBJS += objs/core/board/NstBoardBmc20in1.o
OBJS += objs/core/board/NstBoardBmc21in1.o
OBJS += objs/core/board/NstBoardBmc22Games.o
OBJS += objs/core/board/NstBoardBmc31in1.o
OBJS += objs/core/board/NstBoardBmc35in1.o
OBJS += objs/core/board/NstBoardBmc36in1.o
OBJS += objs/core/board/NstBoardBmc64in1.o
OBJS += objs/core/board/NstBoardBmc72in1.o
OBJS += objs/core/board/NstBoardBmc76in1.o
OBJS += objs/core/board/NstBoardBmc800in1.o
OBJS += objs/core/board/NstBoardBmc8157.o
OBJS += objs/core/board/NstBoardBmc9999999in1.o
OBJS += objs/core/board/NstBoardBmcA65as.o
OBJS += objs/core/board/NstBoardBmcBallgames11in1.o
OBJS += objs/core/board/NstBoardBmcCh001.o
OBJS += objs/core/board/NstBoardBmcCtc65.o
OBJS += objs/core/board/NstBoardBmcFamily4646B.o
OBJS += objs/core/board/NstBoardBmcFk23c.o
OBJS += objs/core/board/NstBoardBmcGamestarA.o
OBJS += objs/core/board/NstBoardBmcGamestarB.o
OBJS += objs/core/board/NstBoardBmcGolden190in1.o
OBJS += objs/core/board/NstBoardBmcGoldenCard6in1.o
OBJS += objs/core/board/NstBoardBmcGoldenGame260in1.o
OBJS += objs/core/board/NstBoardBmcHero.o
OBJS += objs/core/board/NstBoardBmcMarioParty7in1.o
OBJS += objs/core/board/NstBoardBmcNovelDiamond.o
OBJS += objs/core/board/NstBoardBmcPowerjoy84in1.o
OBJS += objs/core/board/NstBoardBmcResetBased4in1.o
OBJS += objs/core/board/NstBoardBmcSuper22Games.o
OBJS += objs/core/board/NstBoardBmcSuper24in1.o
OBJS += objs/core/board/NstBoardBmcSuper40in1.o
OBJS += objs/core/board/NstBoardBmcSuper700in1.o
OBJS += objs/core/board/NstBoardBmcSuperBig7in1.o
OBJS += objs/core/board/NstBoardBmcSuperGun20in1.o
OBJS += objs/core/board/NstBoardBmcSuperHiK300in1.o
OBJS += objs/core/board/NstBoardBmcSuperHiK4in1.o
OBJS += objs/core/board/NstBoardBmcSuperVision16in1.o
OBJS += objs/core/board/NstBoardBmcT262.o
OBJS += objs/core/board/NstBoardBmcVrc4.o
OBJS += objs/core/board/NstBoardBmcVt5201.o
OBJS += objs/core/board/NstBoardBmcY2k64in1.o
OBJS += objs/core/board/NstBoardBtl2708.o
OBJS += objs/core/board/NstBoardBtl6035052.o
OBJS += objs/core/board/NstBoardBtlAx5705.o
OBJS += objs/core/board/NstBoardBtlDragonNinja.o
OBJS += objs/core/board/NstBoardBtlGeniusMerioBros.o
OBJS += objs/core/board/NstBoardBtlMarioBaby.o
OBJS += objs/core/board/NstBoardBtlPikachuY2k.o
OBJS += objs/core/board/NstBoardBtlShuiGuanPipe.o
OBJS += objs/core/board/NstBoardBtlSmb2a.o
OBJS += objs/core/board/NstBoardBtlSmb2b.o
OBJS += objs/core/board/NstBoardBtlSmb2c.o
OBJS += objs/core/board/NstBoardBtlSmb3.o
OBJS += objs/core/board/NstBoardBtlSuperBros11.o
OBJS += objs/core/board/NstBoardBtlT230.o
OBJS += objs/core/board/NstBoardBtlTobidaseDaisakusen.o
OBJS += objs/core/board/NstBoardBxRom.o
OBJS += objs/core/board/NstBoardCaltron.o
OBJS += objs/core/board/NstBoardCamerica.o
OBJS += objs/core/board/NstBoardCneDecathlon.o
OBJS += objs/core/board/NstBoardCnePsb.o
OBJS += objs/core/board/NstBoardCneShlz.o
OBJS += objs/core/board/NstBoardCony.o
OBJS += objs/core/board/NstBoard.o
OBJS += objs/core/board/NstBoardCxRom.o
OBJS += objs/core/board/NstBoardDiscrete.o
OBJS += objs/core/board/NstBoardDreamtech.o
OBJS += objs/core/board/NstBoardEvent.o
OBJS += objs/core/board/NstBoardFb.o
OBJS += objs/core/board/NstBoardFfe.o
OBJS += objs/core/board/NstBoardFujiya.o
OBJS += objs/core/board/NstBoardFukutake.o
OBJS += objs/core/board/NstBoardFutureMedia.o
OBJS += objs/core/board/NstBoardGouder.o
OBJS += objs/core/board/NstBoardGxRom.o
OBJS += objs/core/board/NstBoardHenggedianzi.o
OBJS += objs/core/board/NstBoardHes.o
OBJS += objs/core/board/NstBoardHosenkan.o
OBJS += objs/core/board/NstBoardIremG101.o
OBJS += objs/core/board/NstBoardIremH3001.o
OBJS += objs/core/board/NstBoardIremHolyDiver.o
OBJS += objs/core/board/NstBoardIremKaiketsu.o
OBJS += objs/core/board/NstBoardIremLrog017.o
OBJS += objs/core/board/NstBoardJalecoJf11.o
OBJS += objs/core/board/NstBoardJalecoJf13.o
OBJS += objs/core/board/NstBoardJalecoJf16.o
OBJS += objs/core/board/NstBoardJalecoJf17.o
OBJS += objs/core/board/NstBoardJalecoJf19.o
OBJS += objs/core/board/NstBoardJalecoSs88006.o
OBJS += objs/core/board/NstBoardJyCompany.o
OBJS += objs/core/board/NstBoardKaiser.o
OBJS += objs/core/board/NstBoardKasing.o
OBJS += objs/core/board/NstBoardKayH2288.o
OBJS += objs/core/board/NstBoardKayPandaPrince.o
OBJS += objs/core/board/NstBoardKonamiVrc1.o
OBJS += objs/core/board/NstBoardKonamiVrc2.o
OBJS += objs/core/board/NstBoardKonamiVrc3.o
OBJS += objs/core/board/NstBoardKonamiVrc4.o
OBJS += objs/core/board/NstBoardKonamiVrc6.o
OBJS += objs/core/board/NstBoardKonamiVrc7.o
OBJS += objs/core/board/NstBoardKonamiVsSystem.o
OBJS += objs/core/board/NstBoardMagicSeries.o
OBJS += objs/core/board/NstBoardMmc1.o
OBJS += objs/core/board/NstBoardMmc2.o
OBJS += objs/core/board/NstBoardMmc3.o
OBJS += objs/core/board/NstBoardMmc4.o
OBJS += objs/core/board/NstBoardMmc5.o
OBJS += objs/core/board/NstBoardMmc6.o
OBJS += objs/core/board/NstBoardNamcot163.o
OBJS += objs/core/board/NstBoardNamcot175.o
OBJS += objs/core/board/NstBoardNamcot34xx.o
OBJS += objs/core/board/NstBoardNanjing.o
OBJS += objs/core/board/NstBoardNihon.o
OBJS += objs/core/board/NstBoardNitra.o
OBJS += objs/core/board/NstBoardNtdec.o
OBJS += objs/core/board/NstBoardOpenCorp.o
OBJS += objs/core/board/NstBoardQj.o
OBJS += objs/core/board/NstBoardRcm.o
OBJS += objs/core/board/NstBoardRexSoftDb5z.o
OBJS += objs/core/board/NstBoardRexSoftSl1632.o
OBJS += objs/core/board/NstBoardRumbleStation.o
OBJS += objs/core/board/NstBoardSachen74x374.o
OBJS += objs/core/board/NstBoardSachenS8259.o
OBJS += objs/core/board/NstBoardSachenSa0036.o
OBJS += objs/core/board/NstBoardSachenSa0037.o
OBJS += objs/core/board/NstBoardSachenSa72007.o
OBJS += objs/core/board/NstBoardSachenSa72008.o
OBJS += objs/core/board/NstBoardSachenStreetHeroes.o
OBJS += objs/core/board/NstBoardSachenTca01.o
OBJS += objs/core/board/NstBoardSachenTcu.o
OBJS += objs/core/board/NstBoardSomeriTeamSl12.o
OBJS += objs/core/board/NstBoardSubor.o
OBJS += objs/core/board/NstBoardSunsoft1.o
OBJS += objs/core/board/NstBoardSunsoft2.o
OBJS += objs/core/board/NstBoardSunsoft3.o
OBJS += objs/core/board/NstBoardSunsoft4.o
OBJS += objs/core/board/NstBoardSunsoft5b.o
OBJS += objs/core/board/NstBoardSunsoftDcs.o
OBJS += objs/core/board/NstBoardSunsoftFme7.o
OBJS += objs/core/board/NstBoardSuperGameBoogerman.o
OBJS += objs/core/board/NstBoardSuperGameLionKing.o
OBJS += objs/core/board/NstBoardSuperGamePocahontas2.o
OBJS += objs/core/board/NstBoardTaitoTc0190fmc.o
OBJS += objs/core/board/NstBoardTaitoTc0190fmcPal16r4.o
OBJS += objs/core/board/NstBoardTaitoX1005.o
OBJS += objs/core/board/NstBoardTaitoX1017.o
OBJS += objs/core/board/NstBoardTengen.o
OBJS += objs/core/board/NstBoardTengenRambo1.o
OBJS += objs/core/board/NstBoardTxc.o
OBJS += objs/core/board/NstBoardTxcMxmdhtwo.o
OBJS += objs/core/board/NstBoardTxcPoliceman.o
OBJS += objs/core/board/NstBoardTxcTw.o
OBJS += objs/core/board/NstBoardTxRom.o
OBJS += objs/core/board/NstBoardUnlA9746.o
OBJS += objs/core/board/NstBoardUnlCc21.o
OBJS += objs/core/board/NstBoardUnlEdu2000.o
OBJS += objs/core/board/NstBoardUnlKingOfFighters96.o
OBJS += objs/core/board/NstBoardUnlKingOfFighters97.o
OBJS += objs/core/board/NstBoardUnlMortalKombat2.o
OBJS += objs/core/board/NstBoardUnlN625092.o
OBJS += objs/core/board/NstBoardUnlSuperFighter3.o
OBJS += objs/core/board/NstBoardUnlTf1201.o
OBJS += objs/core/board/NstBoardUnlWorldHero.o
OBJS += objs/core/board/NstBoardUnlXzy.o
OBJS += objs/core/board/NstBoardUxRom.o
OBJS += objs/core/board/NstBoardVsSystem.o
OBJS += objs/core/board/NstBoardWaixing.o
OBJS += objs/core/board/NstBoardWaixingFfv.o
OBJS += objs/core/board/NstBoardWaixingPs2.o
OBJS += objs/core/board/NstBoardWaixingSecurity.o
OBJS += objs/core/board/NstBoardWaixingSgz.o
OBJS += objs/core/board/NstBoardWaixingSgzlz.o
OBJS += objs/core/board/NstBoardWaixingSh2.o
OBJS += objs/core/board/NstBoardWaixingZs.o
OBJS += objs/core/board/NstBoardWhirlwind.o
OBJS += objs/core/board/NstBoardZz.o

# Input
OBJS += objs/core/input/NstInpAdapter.o
OBJS += objs/core/input/NstInpBandaiHyperShot.o
OBJS += objs/core/input/NstInpBarcodeWorld.o
OBJS += objs/core/input/NstInpCrazyClimber.o
OBJS += objs/core/input/NstInpDoremikkoKeyboard.o
OBJS += objs/core/input/NstInpExcitingBoxing.o
OBJS += objs/core/input/NstInpFamilyKeyboard.o
OBJS += objs/core/input/NstInpFamilyTrainer.o
OBJS += objs/core/input/NstInpHoriTrack.o
OBJS += objs/core/input/NstInpKonamiHyperShot.o
OBJS += objs/core/input/NstInpMahjong.o
OBJS += objs/core/input/NstInpMouse.o
OBJS += objs/core/input/NstInpOekaKidsTablet.o
OBJS += objs/core/input/NstInpPachinko.o
OBJS += objs/core/input/NstInpPad.o
OBJS += objs/core/input/NstInpPaddle.o
OBJS += objs/core/input/NstInpPartyTap.o
OBJS += objs/core/input/NstInpPokkunMoguraa.o
OBJS += objs/core/input/NstInpPowerGlove.o
OBJS += objs/core/input/NstInpPowerPad.o
OBJS += objs/core/input/NstInpRob.o
OBJS += objs/core/input/NstInpSuborKeyboard.o
OBJS += objs/core/input/NstInpTopRider.o
OBJS += objs/core/input/NstInpTurboFile.o
OBJS += objs/core/input/NstInpZapper.o

# VS System
OBJS += objs/core/vssystem/NstVsRbiBaseball.o
OBJS += objs/core/vssystem/NstVsSuperXevious.o
OBJS += objs/core/vssystem/NstVsSystem.o
OBJS += objs/core/vssystem/NstVsTkoBoxing.o

# Interface
IOBJS += objs/unix/main.o
IOBJS += objs/unix/cli.o
IOBJS += objs/unix/audio.o
IOBJS += objs/unix/video.o
IOBJS += objs/unix/input.o
IOBJS += objs/unix/config.o
IOBJS += objs/unix/cheats.o
IOBJS += objs/unix/cursor.o
IOBJS += objs/unix/ini.o
IOBJS += objs/unix/png.o

# object dirs
OBJDIRS += objs objs/core objs/core/api objs/core/board objs/core/input
OBJDIRS += objs/core/vssystem objs/nes_ntsc objs/unix

# Core rules
objs/core/%.o: source/core/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Interface rules
objs/unix/%.o: source/unix/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(WARNINGS) $(DEFINES) $(CFLAGS) -c $< -o $@

all: maketree $(BIN)

core: maketree $(OBJS)

interface: maketree $(IOBJS)

maketree: $(sort $(OBJDIRS))

$(sort $(OBJDIRS)):
	@mkdir $@

$(BIN): $(OBJS) $(IOBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $(BIN)

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
	rm -f $(OBJS) $(IOBJS) $(BIN)
