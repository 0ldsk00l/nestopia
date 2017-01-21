# set project name
project( nestopia )
set( nestopia_VERSION 1.48-WIP )

# CMake requirements
# 3.1 is required for target_sources()
cmake_minimum_required( VERSION 3.1 )

# on macOS, MACOSX_RPATH is enabled by default on more recent versions
# of CMake. Disable this behaviour, and let user enable it if need be.
cmake_policy( SET CMP0042 OLD )

# Set Release type for builds where CMAKE_BUILD_TYPE is unset.
# This is usually a good default as this implictly enables
#
#   CXXFLAGS = -O3 -DNDEBUG
#
if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "Release" )
endif()

# Adhere to GNU filesystem layout conventions
include( GNUInstallDirs )

add_executable( nestopia
	# Core
	source/core/NstApu.cpp
	source/core/NstAssert.cpp
	source/core/NstCartridge.cpp
	source/core/NstCartridgeInes.cpp
	source/core/NstCartridgeRomset.cpp
	source/core/NstCartridgeUnif.cpp
	source/core/NstCheats.cpp
	source/core/NstChecksum.cpp
	source/core/NstChips.cpp
	source/core/NstCore.cpp
	source/core/NstCpu.cpp
	source/core/NstCrc32.cpp
	source/core/NstFds.cpp
	source/core/NstFile.cpp
	source/core/NstImage.cpp
	source/core/NstImageDatabase.cpp
	source/core/NstLog.cpp
	source/core/NstMachine.cpp
	source/core/NstMemory.cpp
	source/core/NstNsf.cpp
	source/core/NstPatcher.cpp
	source/core/NstPatcherIps.cpp
	source/core/NstPatcherUps.cpp
	source/core/NstPins.cpp
	source/core/NstPpu.cpp
	source/core/NstProperties.cpp
	source/core/NstRam.cpp
	source/core/NstSha1.cpp
	source/core/NstSoundPcm.cpp
	source/core/NstSoundPlayer.cpp
	source/core/NstSoundRenderer.cpp
	source/core/NstState.cpp
	source/core/NstStream.cpp
	source/core/NstTracker.cpp
	source/core/NstTrackerMovie.cpp
	source/core/NstTrackerRewinder.cpp
	source/core/NstVector.cpp
	source/core/NstVideoFilter2xSaI.cpp
	source/core/NstVideoFilterHqX.cpp
	source/core/NstVideoFilterNone.cpp
	source/core/NstVideoFilterNtsc.cpp
	source/core/NstVideoFilterNtscCfg.cpp
	source/core/NstVideoFilterScaleX.cpp
	source/core/NstVideoFilterxBR.cpp
	source/core/NstVideoRenderer.cpp
	source/core/NstVideoScreen.cpp
	source/core/NstXml.cpp
	source/core/NstZlib.cpp
	# API
	source/core/api/NstApiBarcodeReader.cpp
	source/core/api/NstApiCartridge.cpp
	source/core/api/NstApiCheats.cpp
	source/core/api/NstApiDipSwitches.cpp
	source/core/api/NstApiEmulator.cpp
	source/core/api/NstApiFds.cpp
	source/core/api/NstApiInput.cpp
	source/core/api/NstApiMachine.cpp
	source/core/api/NstApiMovie.cpp
	source/core/api/NstApiNsf.cpp
	source/core/api/NstApiRewinder.cpp
	source/core/api/NstApiSound.cpp
	source/core/api/NstApiTapeRecorder.cpp
	source/core/api/NstApiUser.cpp
	source/core/api/NstApiVideo.cpp
	# Board
	source/core/board/NstBoardAcclaimMcAcc.cpp
	source/core/board/NstBoardAe.cpp
	source/core/board/NstBoardAgci.cpp
	source/core/board/NstBoardAveD1012.cpp
	source/core/board/NstBoardAveNina.cpp
	source/core/board/NstBoardAxRom.cpp
	source/core/board/NstBoardBandai24c0x.cpp
	source/core/board/NstBoardBandaiAerobicsStudio.cpp
	source/core/board/NstBoardBandaiDatach.cpp
	source/core/board/NstBoardBandaiKaraokeStudio.cpp
	source/core/board/NstBoardBandaiLz93d50.cpp
	source/core/board/NstBoardBandaiLz93d50ex.cpp
	source/core/board/NstBoardBandaiOekaKids.cpp
	source/core/board/NstBoardBenshengBs5.cpp
	source/core/board/NstBoardBmc110in1.cpp
	source/core/board/NstBoardBmc1200in1.cpp
	source/core/board/NstBoardBmc150in1.cpp
	source/core/board/NstBoardBmc15in1.cpp
	source/core/board/NstBoardBmc20in1.cpp
	source/core/board/NstBoardBmc21in1.cpp
	source/core/board/NstBoardBmc22Games.cpp
	source/core/board/NstBoardBmc31in1.cpp
	source/core/board/NstBoardBmc35in1.cpp
	source/core/board/NstBoardBmc36in1.cpp
	source/core/board/NstBoardBmc64in1.cpp
	source/core/board/NstBoardBmc72in1.cpp
	source/core/board/NstBoardBmc76in1.cpp
	source/core/board/NstBoardBmc800in1.cpp
	source/core/board/NstBoardBmc8157.cpp
	source/core/board/NstBoardBmc9999999in1.cpp
	source/core/board/NstBoardBmcA65as.cpp
	source/core/board/NstBoardBmcBallgames11in1.cpp
	source/core/board/NstBoardBmcCh001.cpp
	source/core/board/NstBoardBmcCtc65.cpp
	source/core/board/NstBoardBmcFamily4646B.cpp
	source/core/board/NstBoardBmcFk23c.cpp
	source/core/board/NstBoardBmcGamestarA.cpp
	source/core/board/NstBoardBmcGamestarB.cpp
	source/core/board/NstBoardBmcGolden190in1.cpp
	source/core/board/NstBoardBmcGoldenCard6in1.cpp
	source/core/board/NstBoardBmcGoldenGame260in1.cpp
	source/core/board/NstBoardBmcHero.cpp
	source/core/board/NstBoardBmcMarioParty7in1.cpp
	source/core/board/NstBoardBmcNovelDiamond.cpp
	source/core/board/NstBoardBmcPowerjoy84in1.cpp
	source/core/board/NstBoardBmcResetBased4in1.cpp
	source/core/board/NstBoardBmcSuper22Games.cpp
	source/core/board/NstBoardBmcSuper24in1.cpp
	source/core/board/NstBoardBmcSuper40in1.cpp
	source/core/board/NstBoardBmcSuper700in1.cpp
	source/core/board/NstBoardBmcSuperBig7in1.cpp
	source/core/board/NstBoardBmcSuperGun20in1.cpp
	source/core/board/NstBoardBmcSuperHiK300in1.cpp
	source/core/board/NstBoardBmcSuperHiK4in1.cpp
	source/core/board/NstBoardBmcSuperVision16in1.cpp
	source/core/board/NstBoardBmcT262.cpp
	source/core/board/NstBoardBmcVrc4.cpp
	source/core/board/NstBoardBmcVt5201.cpp
	source/core/board/NstBoardBmcY2k64in1.cpp
	source/core/board/NstBoardBtl2708.cpp
	source/core/board/NstBoardBtl6035052.cpp
	source/core/board/NstBoardBtlAx5705.cpp
	source/core/board/NstBoardBtlDragonNinja.cpp
	source/core/board/NstBoardBtlGeniusMerioBros.cpp
	source/core/board/NstBoardBtlMarioBaby.cpp
	source/core/board/NstBoardBtlPikachuY2k.cpp
	source/core/board/NstBoardBtlShuiGuanPipe.cpp
	source/core/board/NstBoardBtlSmb2a.cpp
	source/core/board/NstBoardBtlSmb2b.cpp
	source/core/board/NstBoardBtlSmb2c.cpp
	source/core/board/NstBoardBtlSmb3.cpp
	source/core/board/NstBoardBtlSuperBros11.cpp
	source/core/board/NstBoardBtlT230.cpp
	source/core/board/NstBoardBtlTobidaseDaisakusen.cpp
	source/core/board/NstBoardBxRom.cpp
	source/core/board/NstBoardCaltron.cpp
	source/core/board/NstBoardCamerica.cpp
	source/core/board/NstBoardCneDecathlon.cpp
	source/core/board/NstBoardCnePsb.cpp
	source/core/board/NstBoardCneShlz.cpp
	source/core/board/NstBoardCony.cpp
	source/core/board/NstBoard.cpp
	source/core/board/NstBoardCxRom.cpp
	source/core/board/NstBoardDiscrete.cpp
	source/core/board/NstBoardDreamtech.cpp
	source/core/board/NstBoardEvent.cpp
	source/core/board/NstBoardFb.cpp
	source/core/board/NstBoardFfe.cpp
	source/core/board/NstBoardFujiya.cpp
	source/core/board/NstBoardFukutake.cpp
	source/core/board/NstBoardFutureMedia.cpp
	source/core/board/NstBoardGouder.cpp
	source/core/board/NstBoardGxRom.cpp
	source/core/board/NstBoardHenggedianzi.cpp
	source/core/board/NstBoardHes.cpp
	source/core/board/NstBoardHosenkan.cpp
	source/core/board/NstBoardIremG101.cpp
	source/core/board/NstBoardIremH3001.cpp
	source/core/board/NstBoardIremHolyDiver.cpp
	source/core/board/NstBoardIremKaiketsu.cpp
	source/core/board/NstBoardIremLrog017.cpp
	source/core/board/NstBoardJalecoJf11.cpp
	source/core/board/NstBoardJalecoJf13.cpp
	source/core/board/NstBoardJalecoJf16.cpp
	source/core/board/NstBoardJalecoJf17.cpp
	source/core/board/NstBoardJalecoJf19.cpp
	source/core/board/NstBoardJalecoSs88006.cpp
	source/core/board/NstBoardJyCompany.cpp
	source/core/board/NstBoardKaiser.cpp
	source/core/board/NstBoardKasing.cpp
	source/core/board/NstBoardKayH2288.cpp
	source/core/board/NstBoardKayPandaPrince.cpp
	source/core/board/NstBoardKonamiVrc1.cpp
	source/core/board/NstBoardKonamiVrc2.cpp
	source/core/board/NstBoardKonamiVrc3.cpp
	source/core/board/NstBoardKonamiVrc4.cpp
	source/core/board/NstBoardKonamiVrc6.cpp
	source/core/board/NstBoardKonamiVrc7.cpp
	source/core/board/NstBoardKonamiVsSystem.cpp
	source/core/board/NstBoardMagicSeries.cpp
	source/core/board/NstBoardMmc1.cpp
	source/core/board/NstBoardMmc2.cpp
	source/core/board/NstBoardMmc3.cpp
	source/core/board/NstBoardMmc4.cpp
	source/core/board/NstBoardMmc5.cpp
	source/core/board/NstBoardMmc6.cpp
	source/core/board/NstBoardNamcot163.cpp
	source/core/board/NstBoardNamcot175.cpp
	source/core/board/NstBoardNamcot34xx.cpp
	source/core/board/NstBoardNanjing.cpp
	source/core/board/NstBoardNihon.cpp
	source/core/board/NstBoardNitra.cpp
	source/core/board/NstBoardNtdec.cpp
	source/core/board/NstBoardOpenCorp.cpp
	source/core/board/NstBoardQj.cpp
	source/core/board/NstBoardRcm.cpp
	source/core/board/NstBoardRexSoftDb5z.cpp
	source/core/board/NstBoardRexSoftSl1632.cpp
	source/core/board/NstBoardRumbleStation.cpp
	source/core/board/NstBoardSachen74x374.cpp
	source/core/board/NstBoardSachenS8259.cpp
	source/core/board/NstBoardSachenSa0036.cpp
	source/core/board/NstBoardSachenSa0037.cpp
	source/core/board/NstBoardSachenSa72007.cpp
	source/core/board/NstBoardSachenSa72008.cpp
	source/core/board/NstBoardSachenStreetHeroes.cpp
	source/core/board/NstBoardSachenTca01.cpp
	source/core/board/NstBoardSachenTcu.cpp
	source/core/board/NstBoardSomeriTeamSl12.cpp
	source/core/board/NstBoardSubor.cpp
	source/core/board/NstBoardSunsoft1.cpp
	source/core/board/NstBoardSunsoft2.cpp
	source/core/board/NstBoardSunsoft3.cpp
	source/core/board/NstBoardSunsoft4.cpp
	source/core/board/NstBoardSunsoft5b.cpp
	source/core/board/NstBoardSunsoftDcs.cpp
	source/core/board/NstBoardSunsoftFme7.cpp
	source/core/board/NstBoardSuperGameBoogerman.cpp
	source/core/board/NstBoardSuperGameLionKing.cpp
	source/core/board/NstBoardSuperGamePocahontas2.cpp
	source/core/board/NstBoardTaitoTc0190fmc.cpp
	source/core/board/NstBoardTaitoTc0190fmcPal16r4.cpp
	source/core/board/NstBoardTaitoX1005.cpp
	source/core/board/NstBoardTaitoX1017.cpp
	source/core/board/NstBoardTengen.cpp
	source/core/board/NstBoardTengenRambo1.cpp
	source/core/board/NstBoardTxc.cpp
	source/core/board/NstBoardTxcMxmdhtwo.cpp
	source/core/board/NstBoardTxcPoliceman.cpp
	source/core/board/NstBoardTxcTw.cpp
	source/core/board/NstBoardTxRom.cpp
	source/core/board/NstBoardUnlA9746.cpp
	source/core/board/NstBoardUnlCc21.cpp
	source/core/board/NstBoardUnlEdu2000.cpp
	source/core/board/NstBoardUnlKingOfFighters96.cpp
	source/core/board/NstBoardUnlKingOfFighters97.cpp
	source/core/board/NstBoardUnlMortalKombat2.cpp
	source/core/board/NstBoardUnlN625092.cpp
	source/core/board/NstBoardUnlSuperFighter3.cpp
	source/core/board/NstBoardUnlTf1201.cpp
	source/core/board/NstBoardUnlWorldHero.cpp
	source/core/board/NstBoardUnlXzy.cpp
	source/core/board/NstBoardUxRom.cpp
	source/core/board/NstBoardVsSystem.cpp
	source/core/board/NstBoardWaixing.cpp
	source/core/board/NstBoardWaixingFfv.cpp
	source/core/board/NstBoardWaixingPs2.cpp
	source/core/board/NstBoardWaixingSecurity.cpp
	source/core/board/NstBoardWaixingSgz.cpp
	source/core/board/NstBoardWaixingSgzlz.cpp
	source/core/board/NstBoardWaixingSh2.cpp
	source/core/board/NstBoardWaixingZs.cpp
	source/core/board/NstBoardWhirlwind.cpp
	source/core/board/NstBoardZz.cpp
	# Input
	source/core/input/NstInpAdapter.cpp
	source/core/input/NstInpBandaiHyperShot.cpp
	source/core/input/NstInpBarcodeWorld.cpp
	source/core/input/NstInpCrazyClimber.cpp
	source/core/input/NstInpDoremikkoKeyboard.cpp
	source/core/input/NstInpExcitingBoxing.cpp
	source/core/input/NstInpFamilyKeyboard.cpp
	source/core/input/NstInpFamilyTrainer.cpp
	source/core/input/NstInpHoriTrack.cpp
	source/core/input/NstInpKonamiHyperShot.cpp
	source/core/input/NstInpMahjong.cpp
	source/core/input/NstInpMouse.cpp
	source/core/input/NstInpOekaKidsTablet.cpp
	source/core/input/NstInpPachinko.cpp
	source/core/input/NstInpPad.cpp
	source/core/input/NstInpPaddle.cpp
	source/core/input/NstInpPartyTap.cpp
	source/core/input/NstInpPokkunMoguraa.cpp
	source/core/input/NstInpPowerGlove.cpp
	source/core/input/NstInpPowerPad.cpp
	source/core/input/NstInpRob.cpp
	source/core/input/NstInpSuborKeyboard.cpp
	source/core/input/NstInpTopRider.cpp
	source/core/input/NstInpTurboFile.cpp
	source/core/input/NstInpZapper.cpp
	# VS System
	source/core/vssystem/NstVsRbiBaseball.cpp
	source/core/vssystem/NstVsSuperXevious.cpp
	source/core/vssystem/NstVsSystem.cpp
	source/core/vssystem/NstVsTkoBoxing.cpp
	# Interface
	source/unix/main.cpp
	source/unix/cli.cpp
	source/unix/audio.cpp
	source/unix/video.cpp
	source/unix/input.cpp
	source/unix/config.cpp
	source/unix/cheats.cpp
	source/unix/cursor.cpp
	source/unix/ini.cpp
	source/unix/png.cpp
)

# check whether compiler accepts -Wno-narrowing
# in order to silence C++11/14 narrowing warnings
include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( -Wno-narrowing HAS_NO_NARROWING )
if ( HAS_NO_NARROWING )
	target_compile_options( nestopia PRIVATE -Wno-narrowing )
endif()

target_compile_definitions( nestopia PRIVATE -DDATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}/nestopia\" -DDATAROOTDIR=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}\" -DNST_PRAGMA_ONCE )
target_include_directories( nestopia PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source )

# pkg-config for a number of libraries
find_package( PkgConfig REQUIRED )

# zlib
find_package( ZLIB REQUIRED )
target_include_directories( nestopia PRIVATE ${ZLIB_INCLUDE_DIRS} )
target_link_libraries( nestopia ${ZLIB_LIBRARIES} )

# libarchive
find_package( LibArchive REQUIRED )
target_include_directories( nestopia PRIVATE ${LibArchive_INCLUDE_DIRS} )
target_link_libraries( nestopia ${LibArchive_LIBRARIES} )

# libao
pkg_check_modules( LibAO REQUIRED ao )
target_include_directories( nestopia PRIVATE ${LibAO_INCLUDE_DIRS} )
target_link_libraries( nestopia ${LibAO_LIBRARIES} )

# SDL2
pkg_check_modules( SDL2 REQUIRED sdl2 )
target_include_directories( nestopia PRIVATE ${SDL2_INCLUDE_DIRS} )
target_link_libraries( nestopia ${SDL2_LIBRARIES} )

# LibEpoxy
pkg_check_modules( LibEpoxy REQUIRED epoxy )
target_include_directories( nestopia PRIVATE ${LibEpoxy_INCLUDE_DIRS} )
target_link_libraries( nestopia ${LibEpoxy_LIBRARIES} )

if( MINGW )
	# Building for MinGW
	target_compile_definitions( nestopia PRIVATE -D_MINGW )

elseif( APPLE )
	# Building on macOS
	target_compile_definitions( nestopia PRIVATE -D_APPLE )

elseif( UNIX )
	# most likely, building on Unix
	# GTK+3
	option( ENABLE_GTK "Build GTK+3 based UI" ON )
	if( ENABLE_GTK )
		# GTK+3
		# compile options
		target_compile_definitions( nestopia PRIVATE -D_GTK )

		pkg_check_modules( GTK3 REQUIRED gtk+-3.0 )
		target_include_directories( nestopia PRIVATE ${GTK3_INCLUDE_DIRS} )
		target_link_libraries( nestopia ${GTK3_LIBRARIES} )

		# check whether compiler accepts -Wno-deprecated-declarations
		# in order to silence GTK+3 deprecation warnings
		check_cxx_compiler_flag( -Wno-deprecated-declarations HAS_NO_DEPRECATED_DECLARATIONS )
		if ( HAS_NO_DEPRECATED_DECLARATIONS )
			target_compile_options( nestopia PRIVATE -Wno-deprecated-declarations )
		endif()

		# GTK+3 GUI files
		target_sources( nestopia PRIVATE
			source/unix/gtkui/gtkui.cpp
			source/unix/gtkui/gtkui_archive.cpp
			source/unix/gtkui/gtkui_callbacks.cpp
			source/unix/gtkui/gtkui_cheats.cpp
			source/unix/gtkui/gtkui_config.cpp
			source/unix/gtkui/gtkui_dialogs.cpp
		)
	endif()
endif()

################
# Installation #
################
# program-specific files
install( TARGETS nestopia                          DESTINATION ${CMAKE_INSTALL_BINDIR} )
install( FILES NstDatabase.xml                     DESTINATION ${CMAKE_INSTALL_DATADIR}/nestopia )

# freedesktop.org-specific files
install( FILES source/unix/icons/nestopia.desktop  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )
install( FILES source/unix/icons/32/nestopia.png   DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps )
install( FILES source/unix/icons/48/nestopia.png   DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps )
install( FILES source/unix/icons/64/nestopia.png   DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps )
install( FILES source/unix/icons/96/nestopia.png   DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/96x96/apps )
install( FILES source/unix/icons/128/nestopia.png  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps )
install( FILES source/unix/icons/svg/nestopia.svg  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps )
install( FILES source/unix/icons/svg/nespad.svg    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps )

# documentation
install( FILES AUTHORS ChangeLog README.md README.unix DESTINATION ${CMAKE_INSTALL_DOCDIR} )
install( FILES readme.html DESTINATION ${CMAKE_INSTALL_DOCDIR}/html )

option( ENABLE_DOC "Install full documentation suite" )
if( ENABLE_DOC )
	install( DIRECTORY doc/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/html )
endif()
