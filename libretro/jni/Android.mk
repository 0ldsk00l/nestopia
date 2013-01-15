LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

NST_DIR = ../..

LOCAL_MODULE    := libretro

ifeq ($(TARGET_ARCH),arm)
LOCAL_CXXFLAGS += -DANDROID_ARM
LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CXXFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CXXFLAGS += -DANDROID_MIPS
endif

OBJS := $(NST_DIR)/source/core/NstZlib.cpp

## Yes, I didn't type all this out ...
# core objs
OBJS += $(NST_DIR)/source/core/NstApu.cpp              $(NST_DIR)/source/core/NstFds.cpp            $(NST_DIR)/source/core/NstPpu.cpp              $(NST_DIR)/source/core/NstVector.cpp
OBJS += $(NST_DIR)/source/core/NstAssert.cpp           $(NST_DIR)/source/core/NstFile.cpp           $(NST_DIR)/source/core/NstProperties.cpp       $(NST_DIR)/source/core/NstVideoFilter2xSaI.cpp
OBJS += $(NST_DIR)/source/core/NstCartridge.cpp        $(NST_DIR)/source/core/NstImage.cpp          $(NST_DIR)/source/core/NstRam.cpp              $(NST_DIR)/source/core/NstVideoFilterHqX.cpp
OBJS += $(NST_DIR)/source/core/NstCartridgeInes.cpp    $(NST_DIR)/source/core/NstImageDatabase.cpp  $(NST_DIR)/source/core/NstSha1.cpp             $(NST_DIR)/source/core/NstVideoFilterNone.cpp
OBJS += $(NST_DIR)/source/core/NstCartridgeRomset.cpp  $(NST_DIR)/source/core/NstLog.cpp            $(NST_DIR)/source/core/NstSoundPcm.cpp
OBJS += $(NST_DIR)/source/core/NstCartridgeUnif.cpp    $(NST_DIR)/source/core/NstMachine.cpp        $(NST_DIR)/source/core/NstSoundPlayer.cpp      $(NST_DIR)/source/core/NstVideoFilterScaleX.cpp
OBJS += $(NST_DIR)/source/core/NstCheats.cpp           $(NST_DIR)/source/core/NstMemory.cpp         $(NST_DIR)/source/core/NstSoundRenderer.cpp    $(NST_DIR)/source/core/NstVideoRenderer.cpp
OBJS += $(NST_DIR)/source/core/NstChecksum.cpp         $(NST_DIR)/source/core/NstNsf.cpp            $(NST_DIR)/source/core/NstState.cpp            $(NST_DIR)/source/core/NstVideoScreen.cpp
OBJS += $(NST_DIR)/source/core/NstChips.cpp            $(NST_DIR)/source/core/NstPatcher.cpp        $(NST_DIR)/source/core/NstStream.cpp           $(NST_DIR)/source/core/NstXml.cpp
OBJS += $(NST_DIR)/source/core/NstCore.cpp             $(NST_DIR)/source/core/NstPatcherIps.cpp     $(NST_DIR)/source/core/NstTracker.cpp          
OBJS += $(NST_DIR)/source/core/NstCpu.cpp              $(NST_DIR)/source/core/NstPatcherUps.cpp     $(NST_DIR)/source/core/NstTrackerMovie.cpp
OBJS += $(NST_DIR)/source/core/NstCrc32.cpp            $(NST_DIR)/source/core/NstPins.cpp           $(NST_DIR)/source/core/NstTrackerRewinder.cpp

# core/api
OBJS += $(NST_DIR)/source/core/api/NstApiBarcodeReader.cpp  $(NST_DIR)/source/core/api/NstApiEmulator.cpp  $(NST_DIR)/source/core/api/NstApiMovie.cpp     $(NST_DIR)/source/core/api/NstApiTapeRecorder.cpp
OBJS += $(NST_DIR)/source/core/api/NstApiCartridge.cpp      $(NST_DIR)/source/core/api/NstApiFds.cpp       $(NST_DIR)/source/core/api/NstApiNsf.cpp       $(NST_DIR)/source/core/api/NstApiUser.cpp
OBJS += $(NST_DIR)/source/core/api/NstApiCheats.cpp         $(NST_DIR)/source/core/api/NstApiInput.cpp     $(NST_DIR)/source/core/api/NstApiRewinder.cpp  $(NST_DIR)/source/core/api/NstApiVideo.cpp
OBJS += $(NST_DIR)/source/core/api/NstApiDipSwitches.cpp    $(NST_DIR)/source/core/api/NstApiMachine.cpp   $(NST_DIR)/source/core/api/NstApiSound.cpp

# core/board
OBJS += $(NST_DIR)/source/core/board/NstBoardAe.cpp                    $(NST_DIR)/source/core/board/NstBoardBtlPikachuY2k.cpp          $(NST_DIR)/source/core/board/NstBoardNihon.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardAgci.cpp                  $(NST_DIR)/source/core/board/NstBoardBtlShuiGuanPipe.cpp        $(NST_DIR)/source/core/board/NstBoardNitra.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardAveD1012.cpp              $(NST_DIR)/source/core/board/NstBoardBtlSmb2a.cpp               $(NST_DIR)/source/core/board/NstBoardNtdec.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardAveNina.cpp               $(NST_DIR)/source/core/board/NstBoardBtlSmb2b.cpp               $(NST_DIR)/source/core/board/NstBoardOpenCorp.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardAxRom.cpp                 $(NST_DIR)/source/core/board/NstBoardBtlSmb2c.cpp               $(NST_DIR)/source/core/board/NstBoardQj.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandai24c0x.cpp           $(NST_DIR)/source/core/board/NstBoardBtlSmb3.cpp                $(NST_DIR)/source/core/board/NstBoardRcm.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiAerobicsStudio.cpp  $(NST_DIR)/source/core/board/NstBoardBtlSuperBros11.cpp         $(NST_DIR)/source/core/board/NstBoardRexSoftDb5z.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiDatach.cpp          $(NST_DIR)/source/core/board/NstBoardBtlT230.cpp                $(NST_DIR)/source/core/board/NstBoardRexSoftSl1632.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiKaraokeStudio.cpp   $(NST_DIR)/source/core/board/NstBoardBtlTobidaseDaisakusen.cpp  $(NST_DIR)/source/core/board/NstBoardRumbleStation.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiLz93d50.cpp         $(NST_DIR)/source/core/board/NstBoardBxRom.cpp                  $(NST_DIR)/source/core/board/NstBoardSachen74x374.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiLz93d50ex.cpp       $(NST_DIR)/source/core/board/NstBoardCaltron.cpp                $(NST_DIR)/source/core/board/NstBoardSachenS8259.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBandaiOekaKids.cpp        $(NST_DIR)/source/core/board/NstBoardCamerica.cpp               $(NST_DIR)/source/core/board/NstBoardSachenSa0036.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBenshengBs5.cpp           $(NST_DIR)/source/core/board/NstBoardCneDecathlon.cpp           $(NST_DIR)/source/core/board/NstBoardSachenSa0037.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc110in1.cpp             $(NST_DIR)/source/core/board/NstBoardCnePsb.cpp                 $(NST_DIR)/source/core/board/NstBoardSachenSa72007.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc1200in1.cpp            $(NST_DIR)/source/core/board/NstBoardCneShlz.cpp                $(NST_DIR)/source/core/board/NstBoardSachenSa72008.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc150in1.cpp             $(NST_DIR)/source/core/board/NstBoardCony.cpp                   $(NST_DIR)/source/core/board/NstBoardSachenStreetHeroes.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc15in1.cpp              $(NST_DIR)/source/core/board/NstBoard.cpp                       $(NST_DIR)/source/core/board/NstBoardSachenTca01.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc20in1.cpp              $(NST_DIR)/source/core/board/NstBoardCxRom.cpp                  $(NST_DIR)/source/core/board/NstBoardSachenTcu.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc21in1.cpp              $(NST_DIR)/source/core/board/NstBoardDiscrete.cpp               $(NST_DIR)/source/core/board/NstBoardSomeriTeamSl12.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc22Games.cpp            $(NST_DIR)/source/core/board/NstBoardDreamtech.cpp              $(NST_DIR)/source/core/board/NstBoardSubor.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc31in1.cpp              $(NST_DIR)/source/core/board/NstBoardEvent.cpp                  $(NST_DIR)/source/core/board/NstBoardSunsoft1.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc35in1.cpp              $(NST_DIR)/source/core/board/NstBoardFb.cpp                     $(NST_DIR)/source/core/board/NstBoardSunsoft2.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc36in1.cpp              $(NST_DIR)/source/core/board/NstBoardFfe.cpp                    $(NST_DIR)/source/core/board/NstBoardSunsoft3.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc64in1.cpp              $(NST_DIR)/source/core/board/NstBoardFujiya.cpp                 $(NST_DIR)/source/core/board/NstBoardSunsoft4.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc72in1.cpp              $(NST_DIR)/source/core/board/NstBoardFukutake.cpp               $(NST_DIR)/source/core/board/NstBoardSunsoft5b.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc76in1.cpp              $(NST_DIR)/source/core/board/NstBoardFutureMedia.cpp            $(NST_DIR)/source/core/board/NstBoardSunsoftDcs.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc800in1.cpp             $(NST_DIR)/source/core/board/NstBoardGouder.cpp                 $(NST_DIR)/source/core/board/NstBoardSunsoftFme7.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc8157.cpp               $(NST_DIR)/source/core/board/NstBoardGxRom.cpp                  $(NST_DIR)/source/core/board/NstBoardSuperGameBoogerman.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmc9999999in1.cpp         $(NST_DIR)/source/core/board/NstBoardHenggedianzi.cpp           $(NST_DIR)/source/core/board/NstBoardSuperGameLionKing.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcA65as.cpp              $(NST_DIR)/source/core/board/NstBoardHes.cpp                    $(NST_DIR)/source/core/board/NstBoardSuperGamePocahontas2.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcBallgames11in1.cpp     $(NST_DIR)/source/core/board/NstBoardHosenkan.cpp               $(NST_DIR)/source/core/board/NstBoardTaitoTc0190fmc.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcCh001.cpp              $(NST_DIR)/source/core/board/NstBoardIremG101.cpp               $(NST_DIR)/source/core/board/NstBoardTaitoTc0190fmcPal16r4.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcCtc65.cpp              $(NST_DIR)/source/core/board/NstBoardIremH3001.cpp              $(NST_DIR)/source/core/board/NstBoardTaitoX1005.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcFamily4646B.cpp        $(NST_DIR)/source/core/board/NstBoardIremHolyDiver.cpp          $(NST_DIR)/source/core/board/NstBoardTaitoX1017.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcFk23c.cpp              $(NST_DIR)/source/core/board/NstBoardIremKaiketsu.cpp           $(NST_DIR)/source/core/board/NstBoardTengen.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGamestarA.cpp          $(NST_DIR)/source/core/board/NstBoardIremLrog017.cpp            $(NST_DIR)/source/core/board/NstBoardTengenRambo1.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGamestarB.cpp          $(NST_DIR)/source/core/board/NstBoardJalecoJf11.cpp             $(NST_DIR)/source/core/board/NstBoardTxc.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGolden190in1.cpp       $(NST_DIR)/source/core/board/NstBoardJalecoJf13.cpp             $(NST_DIR)/source/core/board/NstBoardTxcMxmdhtwo.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGoldenCard6in1.cpp     $(NST_DIR)/source/core/board/NstBoardJalecoJf16.cpp             $(NST_DIR)/source/core/board/NstBoardTxcPoliceman.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcGoldenGame260in1.cpp   $(NST_DIR)/source/core/board/NstBoardJalecoJf17.cpp             $(NST_DIR)/source/core/board/NstBoardTxcTw.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcHero.cpp               $(NST_DIR)/source/core/board/NstBoardJalecoJf19.cpp             $(NST_DIR)/source/core/board/NstBoardTxRom.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcMarioParty7in1.cpp     $(NST_DIR)/source/core/board/NstBoardJalecoSs88006.cpp          $(NST_DIR)/source/core/board/NstBoardUnlA9746.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcNovelDiamond.cpp       $(NST_DIR)/source/core/board/NstBoardJyCompany.cpp              $(NST_DIR)/source/core/board/NstBoardUnlCc21.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcPowerjoy84in1.cpp      $(NST_DIR)/source/core/board/NstBoardKaiser.cpp                 $(NST_DIR)/source/core/board/NstBoardUnlEdu2000.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcResetBased4in1.cpp     $(NST_DIR)/source/core/board/NstBoardKasing.cpp                 $(NST_DIR)/source/core/board/NstBoardUnlKingOfFighters96.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper22Games.cpp       $(NST_DIR)/source/core/board/NstBoardKayH2288.cpp               $(NST_DIR)/source/core/board/NstBoardUnlKingOfFighters97.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper24in1.cpp         $(NST_DIR)/source/core/board/NstBoardKayPandaPrince.cpp         $(NST_DIR)/source/core/board/NstBoardUnlMortalKombat2.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper40in1.cpp         $(NST_DIR)/source/core/board/NstBoardKonamiVrc1.cpp             $(NST_DIR)/source/core/board/NstBoardUnlN625092.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuper700in1.cpp        $(NST_DIR)/source/core/board/NstBoardKonamiVrc2.cpp             $(NST_DIR)/source/core/board/NstBoardUnlSuperFighter3.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperBig7in1.cpp       $(NST_DIR)/source/core/board/NstBoardKonamiVrc3.cpp             $(NST_DIR)/source/core/board/NstBoardUnlTf1201.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperGun20in1.cpp      $(NST_DIR)/source/core/board/NstBoardKonamiVrc4.cpp             $(NST_DIR)/source/core/board/NstBoardUnlWorldHero.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperHiK300in1.cpp     $(NST_DIR)/source/core/board/NstBoardKonamiVrc6.cpp             $(NST_DIR)/source/core/board/NstBoardUnlXzy.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperHiK4in1.cpp       $(NST_DIR)/source/core/board/NstBoardKonamiVrc7.cpp             $(NST_DIR)/source/core/board/NstBoardUxRom.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcSuperVision16in1.cpp   $(NST_DIR)/source/core/board/NstBoardKonamiVsSystem.cpp         $(NST_DIR)/source/core/board/NstBoardVsSystem.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcT262.cpp               $(NST_DIR)/source/core/board/NstBoardMagicSeries.cpp            $(NST_DIR)/source/core/board/NstBoardWaixing.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcVrc4.cpp               $(NST_DIR)/source/core/board/NstBoardMmc1.cpp                   $(NST_DIR)/source/core/board/NstBoardWaixingFfv.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcVt5201.cpp             $(NST_DIR)/source/core/board/NstBoardMmc2.cpp                   $(NST_DIR)/source/core/board/NstBoardWaixingPs2.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBmcY2k64in1.cpp           $(NST_DIR)/source/core/board/NstBoardMmc3.cpp                   $(NST_DIR)/source/core/board/NstBoardWaixingSecurity.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBtl2708.cpp               $(NST_DIR)/source/core/board/NstBoardMmc4.cpp                   $(NST_DIR)/source/core/board/NstBoardWaixingSgz.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBtl6035052.cpp            $(NST_DIR)/source/core/board/NstBoardMmc5.cpp                   $(NST_DIR)/source/core/board/NstBoardWaixingSgzlz.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlAx5705.cpp             $(NST_DIR)/source/core/board/NstBoardMmc6.cpp                   $(NST_DIR)/source/core/board/NstBoardWaixingSh2.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlDragonNinja.cpp        $(NST_DIR)/source/core/board/NstBoardNamcot163.cpp              $(NST_DIR)/source/core/board/NstBoardWaixingZs.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlGeniusMerioBros.cpp    $(NST_DIR)/source/core/board/NstBoardNamcot34xx.cpp             $(NST_DIR)/source/core/board/NstBoardWhirlwind.cpp
OBJS += $(NST_DIR)/source/core/board/NstBoardBtlMarioBaby.cpp          $(NST_DIR)/source/core/board/NstBoardNanjing.cpp                $(NST_DIR)/source/core/board/NstBoardZz.cpp

# core/input
OBJS += $(NST_DIR)/source/core/input/NstInpAdapter.cpp            $(NST_DIR)/source/core/input/NstInpKonamiHyperShot.cpp  $(NST_DIR)/source/core/input/NstInpPowerGlove.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpBandaiHyperShot.cpp    $(NST_DIR)/source/core/input/NstInpMahjong.cpp          $(NST_DIR)/source/core/input/NstInpPowerPad.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpBarcodeWorld.cpp       $(NST_DIR)/source/core/input/NstInpMouse.cpp            $(NST_DIR)/source/core/input/NstInpRob.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpCrazyClimber.cpp       $(NST_DIR)/source/core/input/NstInpOekaKidsTablet.cpp   $(NST_DIR)/source/core/input/NstInpSuborKeyboard.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpDoremikkoKeyboard.cpp  $(NST_DIR)/source/core/input/NstInpPachinko.cpp         $(NST_DIR)/source/core/input/NstInpTopRider.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpExcitingBoxing.cpp     $(NST_DIR)/source/core/input/NstInpPad.cpp              $(NST_DIR)/source/core/input/NstInpTurboFile.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpFamilyKeyboard.cpp     $(NST_DIR)/source/core/input/NstInpPaddle.cpp           $(NST_DIR)/source/core/input/NstInpZapper.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpFamilyTrainer.cpp      $(NST_DIR)/source/core/input/NstInpPartyTap.cpp
OBJS += $(NST_DIR)/source/core/input/NstInpHoriTrack.cpp          $(NST_DIR)/source/core/input/NstInpPokkunMoguraa.cpp

# core/vssystem
OBJS += $(NST_DIR)/source/core/vssystem/NstVsRbiBaseball.cpp  $(NST_DIR)/source/core/vssystem/NstVsSuperXevious.cpp  $(NST_DIR)/source/core/vssystem/NstVsSystem.cpp  $(NST_DIR)/source/core/vssystem/NstVsTkoBoxing.cpp

OBJS += ../libretro.cpp

LOCAL_SRC_FILES := $(OBJS)

LOCAL_CXXFLAGS += -DINLINE=inline -DHAVE_STDINT_H -DHAVE_INTTYPES_H -DLSB_FIRST -D__LIBRETRO__ -DNST_NO_ZLIB -fexceptions
LOCAL_C_INCLUDES  = $(NST_DIR) $(NST_DIR)/source

include $(BUILD_SHARED_LIBRARY)
