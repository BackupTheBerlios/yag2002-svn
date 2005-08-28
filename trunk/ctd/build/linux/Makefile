#################################################################
## This Makefile Exported by MinGW Developer Studio
## Copyright (c) 2005 by Parinya Thipchart
#################################################################

ifneq (,$(findstring Release, $(CFG)))
  override CFG = Release
else
  override CFG = Debug
endif

PROJECT = vrc
CC = "/usr/bin/g++"

ifeq ($(CFG),Debug)
  OBJ_DIR = ./obj/vrc/debug
  OUTPUT_DIR = Debug
  TARGET = vrc
  C_INCLUDE_DIRS = -I"../../src/framework" -I"../../src/gamecode" -I"../../../../../../../../usr/local/include" -I"../../../../../../../../usr/local/include/CEGUI" -I"../../../../../../../../usr/local/include/tinyxml" -I"../../../../../../../../usr/include/SDL" 
  C_PREPROC = -DLINUX 
  CFLAGS = -pipe  -Wall -g2 -O0 -frtti -fexceptions 
  LIB_DIRS = -L"../../../../../../../../usr/local/lib" -L"../../../../../../../../usr/lib/" 
  LIBS = -lframework-debug -lCEGUIBase -lCEGUITaharezLook -lcal3d -lNewton -lTinyXML -lvorbisfile -lvorbis -lopenalpp -logg -lopenal -lOpenThreads -losg -losgSDL -losgGA -losgText -losgCal -losgal -losgDB -losgUtil 
  LDFLAGS = -pipe 
endif

ifeq ($(CFG),Release)
  OBJ_DIR = ./obj/vrc/release
  OUTPUT_DIR = Release
  TARGET = vrc
  C_INCLUDE_DIRS = -I"../../src/framework" -I"../../src/gamecode" -I"../../../../../../../../usr/local/include" -I"../../../../../../../../usr/local/include/CEGUI" -I"../../../../../../../../usr/local/include/tinyxml" -I"../../../../../../../../usr/include/SDL" 
  C_PREPROC = -DLINUX 
  CFLAGS = -pipe  -Wall -g0 -O2 -frtti -fexceptions 
  LIB_DIRS = -L"../../../../../../../../usr/local/lib" -L"../../../../../../../../usr/lib/" -L"lib" 
  LIBS = -lframework -lCEGUIBase -lCEGUITaharezLook -lcal3d -lNewton -lTinyXML -lvorbisfile -lvorbis -lopenalpp -logg -lopenal -lOpenThreads -lSDL -lSDLmain -losg -losgSDL -losgGA -losgText -losgCal -losgal -losgDB -losgUtil 
  LDFLAGS = -pipe -s 
endif

SRC_OBJS = \
  $(OBJ_DIR)/Noise.o	\
  $(OBJ_DIR)/ctd_dialoglevelselect.o	\
  $(OBJ_DIR)/ctd_dialogplayercfg.o	\
  $(OBJ_DIR)/ctd_dialogsettings.o	\
  $(OBJ_DIR)/ctd_intro.o	\
  $(OBJ_DIR)/ctd_menu.o	\
  $(OBJ_DIR)/ctd_geomemitter.o	\
  $(OBJ_DIR)/ctd_physicsbase.o	\
  $(OBJ_DIR)/ctd_physicsbox.o	\
  $(OBJ_DIR)/ctd_physicscylinder.o	\
  $(OBJ_DIR)/ctd_physicssphere.o	\
  $(OBJ_DIR)/ctd_platform.o	\
  $(OBJ_DIR)/ctd_chatgui.o	\
  $(OBJ_DIR)/ctd_inputhandler.o	\
  $(OBJ_DIR)/ctd_player.o	\
  $(OBJ_DIR)/ctd_playeranim.o	\
  $(OBJ_DIR)/ctd_playerimpl.o	\
  $(OBJ_DIR)/ctd_playerimplclient.o	\
  $(OBJ_DIR)/ctd_playerimplserver.o	\
  $(OBJ_DIR)/ctd_playerimplstandalone.o	\
  $(OBJ_DIR)/ctd_playerphysics.o	\
  $(OBJ_DIR)/ctd_playersound.o	\
  $(OBJ_DIR)/ctd_spawnpoint.o	\
  $(OBJ_DIR)/ctd_3dsound.o	\
  $(OBJ_DIR)/ctd_ambientsound.o	\
  $(OBJ_DIR)/ctd_fpsdisplay.o	\
  $(OBJ_DIR)/ctd_inspector.o	\
  $(OBJ_DIR)/ctd_playerinfodisplay.o	\
  $(OBJ_DIR)/ctd_basecmd.o	\
  $(OBJ_DIR)/ctd_cmdcd.o	\
  $(OBJ_DIR)/ctd_cmdconfiglist.o	\
  $(OBJ_DIR)/ctd_cmdconfigset.o	\
  $(OBJ_DIR)/ctd_cmdconnect.o	\
  $(OBJ_DIR)/ctd_cmdentityattrlist.o	\
  $(OBJ_DIR)/ctd_cmdentityattrset.o	\
  $(OBJ_DIR)/ctd_cmdentitydump.o	\
  $(OBJ_DIR)/ctd_cmdentitylist.o	\
  $(OBJ_DIR)/ctd_cmdexec.o	\
  $(OBJ_DIR)/ctd_cmdgetconfig.o	\
  $(OBJ_DIR)/ctd_cmdhelp.o	\
  $(OBJ_DIR)/ctd_cmdidle.o	\
  $(OBJ_DIR)/ctd_cmdlevellist.o	\
  $(OBJ_DIR)/ctd_cmdlistentity.o	\
  $(OBJ_DIR)/ctd_cmdloadentities.o	\
  $(OBJ_DIR)/ctd_cmdloadfinalize.o	\
  $(OBJ_DIR)/ctd_cmdloadlevel.o	\
  $(OBJ_DIR)/ctd_cmdlogclose.o	\
  $(OBJ_DIR)/ctd_cmdlogcreate.o	\
  $(OBJ_DIR)/ctd_cmdls.o	\
  $(OBJ_DIR)/ctd_cmdphysics.o	\
  $(OBJ_DIR)/ctd_cmdpwd.o	\
  $(OBJ_DIR)/ctd_cmdregistry.o	\
  $(OBJ_DIR)/ctd_cmdserverstart.o	\
  $(OBJ_DIR)/ctd_cmdsetconfig.o	\
  $(OBJ_DIR)/ctd_cmdshowfps.o	\
  $(OBJ_DIR)/ctd_cmdshowplayerinfo.o	\
  $(OBJ_DIR)/ctd_cmdshutdown.o	\
  $(OBJ_DIR)/ctd_cmdtime.o	\
  $(OBJ_DIR)/ctd_cmdunloadlevel.o	\
  $(OBJ_DIR)/ctd_cmdversion.o	\
  $(OBJ_DIR)/ctd_cmdwriteosgfile.o	\
  $(OBJ_DIR)/ctd_console.o	\
  $(OBJ_DIR)/ctd_camera.o	\
  $(OBJ_DIR)/ctd_candle.o	\
  $(OBJ_DIR)/ctd_fog.o	\
  $(OBJ_DIR)/ctd_lightmanager.o	\
  $(OBJ_DIR)/ctd_mesh.o	\
  $(OBJ_DIR)/ctd_observer.o	\
  $(OBJ_DIR)/ctd_pointlight.o	\
  $(OBJ_DIR)/ctd_skybox.o	\
  $(OBJ_DIR)/ctd_spotlight.o	\
  $(OBJ_DIR)/ctd_water.o	\
  $(OBJ_DIR)/ctd_exceptionlogger.o	\
  $(OBJ_DIR)/ctd_gameutils.o	\
  $(OBJ_DIR)/ctd_main.o


define build_target
@echo Linking...
@$(CC) -o "$(OUTPUT_DIR)/$(TARGET)" $(SRC_OBJS) $(LIB_DIRS) $(LIBS) $(LDFLAGS)
endef

define compile_source
@echo Compiling $<
@$(CC) $(CFLAGS) $(C_PREPROC) $(C_INCLUDE_DIRS) -c "$<" -o "$@"
endef

.PHONY: print_header directories

$(TARGET): print_header directories $(SRC_OBJS)
	$(build_target)

.PHONY: clean cleanall

cleanall:
	@echo Deleting intermediate files for 'vrc - $(CFG)'
	-@rm -rf "$(OBJ_DIR)"
	-@rm -rf "$(OUTPUT_DIR)/$(TARGET)"
	-@rmdir "$(OUTPUT_DIR)"

clean:
	@echo Deleting intermediate files for 'vrc - $(CFG)'
	-@rm -rf "$(OBJ_DIR)"

print_header:
	@echo ----------Configuration: vrc - Linux $(CFG)----------

directories:
	-@if [ ! -d "$(OUTPUT_DIR)" ]; then mkdir "$(OUTPUT_DIR)"; fi
	-@if [ ! -d "$(OBJ_DIR)" ]; then mkdir "$(OBJ_DIR)"; fi

$(OBJ_DIR)/Noise.o: ../../src/gamecode/extern/Noise.cpp

	$(compile_source)

$(OBJ_DIR)/ctd_dialoglevelselect.o: ../../src/gamecode/gui/ctd_dialoglevelselect.cpp	\
../../src/gamecode/gui/ctd_dialoglevelselect.h	\
../../src/gamecode/gui/ctd_menu.h	\
../../src/gamecode/sound/ctd_ambientsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_dialogplayercfg.o: ../../src/gamecode/gui/ctd_dialogplayercfg.cpp	\
../../src/gamecode/gui/ctd_dialogplayercfg.h	\
../../src/gamecode/gui/ctd_dialogsettings.h	\
../../src/gamecode/sound/ctd_ambientsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_dialogsettings.o: ../../src/gamecode/gui/ctd_dialogsettings.cpp	\
../../src/gamecode/gui/ctd_menu.h	\
../../src/gamecode/gui/ctd_dialogsettings.h	\
../../src/gamecode/gui/ctd_dialogplayercfg.h	\
../../src/gamecode/sound/ctd_ambientsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_intro.o: ../../src/gamecode/gui/ctd_intro.cpp	\
../../src/gamecode/gui/ctd_intro.h	\
../../src/gamecode/sound/ctd_ambientsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_menu.o: ../../src/gamecode/gui/ctd_menu.cpp	\
../../src/gamecode/gui/ctd_menu.h	\
../../src/gamecode/gui/ctd_dialogsettings.h	\
../../src/gamecode/gui/ctd_dialoglevelselect.h	\
../../src/gamecode/gui/ctd_intro.h	\
../../src/gamecode/sound/ctd_ambientsound.h	\
../../src/gamecode/visuals/ctd_camera.h	\
../../src/gamecode/visuals/ctd_skybox.h	\
../../src/gamecode/visuals/ctd_fog.h
	$(compile_source)

$(OBJ_DIR)/ctd_geomemitter.o: ../../src/gamecode/physics/ctd_geomemitter.cpp	\
../../src/gamecode/physics/ctd_geomemitter.h
	$(compile_source)

$(OBJ_DIR)/ctd_physicsbase.o: ../../src/gamecode/physics/ctd_physicsbase.cpp	\
../../src/gamecode/physics/ctd_physicsbase.h	\
../../src/gamecode/physics/ctd_physicsbase.inl	\
../../src/gamecode/sound/ctd_3dsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_physicsbox.o: ../../src/gamecode/physics/ctd_physicsbox.cpp	\
../../src/gamecode/physics/ctd_physicsbox.h	\
../../src/gamecode/physics/ctd_physicsbase.h	\
../../src/gamecode/physics/ctd_physicsbase.inl
	$(compile_source)

$(OBJ_DIR)/ctd_physicscylinder.o: ../../src/gamecode/physics/ctd_physicscylinder.cpp	\
../../src/gamecode/physics/ctd_physicscylinder.h	\
../../src/gamecode/physics/ctd_physicsbase.h	\
../../src/gamecode/physics/ctd_physicsbase.inl	\
../../src/gamecode/sound/ctd_3dsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_physicssphere.o: ../../src/gamecode/physics/ctd_physicssphere.cpp	\
../../src/gamecode/physics/ctd_physicssphere.h	\
../../src/gamecode/physics/ctd_physicsbase.h	\
../../src/gamecode/physics/ctd_physicsbase.inl
	$(compile_source)

$(OBJ_DIR)/ctd_platform.o: ../../src/gamecode/platform/ctd_platform.cpp	\
../../src/gamecode/platform/ctd_platform.h
	$(compile_source)

$(OBJ_DIR)/ctd_chatgui.o: ../../src/gamecode/player/ctd_chatgui.cpp	\
../../src/gamecode/player/ctd_chatgui.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h
	$(compile_source)

$(OBJ_DIR)/ctd_inputhandler.o: ../../src/gamecode/player/ctd_inputhandler.cpp	\
../../src/gamecode/player/ctd_inputhandler.h	\
../../src/gamecode/player/ctd_player.h	\
../../src/gamecode/player/ctd_playeranim.h	\
../../src/gamecode/player/ctd_playersound.h	\
../../src/gamecode/player/ctd_playerphysics.h	\
../../src/gamecode/player/ctd_chatgui.h	\
../../src/gamecode/visuals/ctd_camera.h	\
../../src/gamecode/player/ctd_inputhandler.inl
	$(compile_source)

$(OBJ_DIR)/ctd_player.o: ../../src/gamecode/player/ctd_player.cpp	\
../../src/gamecode/player/ctd_player.h	\
../../src/gamecode/player/ctd_playerimplstandalone.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_playerimplserver.h	\
../../src/gamecode/player/ctd_playerimplclient.h	\
../../src/gamecode/player/ctd_spawnpoint.h	\
../../src/gamecode/player/ctd_chatgui.h
	$(compile_source)

$(OBJ_DIR)/ctd_playeranim.o: ../../src/gamecode/player/ctd_playeranim.cpp	\
../../src/gamecode/player/ctd_playeranim.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h
	$(compile_source)

$(OBJ_DIR)/ctd_playerimpl.o: ../../src/gamecode/player/ctd_playerimpl.cpp	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h	\
../../src/gamecode/player/ctd_playerphysics.h	\
../../src/gamecode/player/ctd_playeranim.h	\
../../src/gamecode/player/ctd_playersound.h	\
../../src/gamecode/player/ctd_inputhandler.h	\
../../src/gamecode/player/ctd_chatgui.h	\
../../src/gamecode/visuals/ctd_camera.h	\
../../src/gamecode/player/ctd_inputhandler.inl	\
../../src/gamecode/player/ctd_playernetworking.h	\
../../src/gamecode/player/networkingRoles/_RO_PlayerNetworking.h
	$(compile_source)

$(OBJ_DIR)/ctd_playerimplclient.o: ../../src/gamecode/player/ctd_playerimplclient.cpp	\
../../src/gamecode/player/ctd_player.h	\
../../src/gamecode/player/ctd_chatgui.h	\
../../src/gamecode/player/ctd_playersound.h	\
../../src/gamecode/player/ctd_inputhandler.h	\
../../src/gamecode/player/ctd_playeranim.h	\
../../src/gamecode/player/ctd_playerphysics.h	\
../../src/gamecode/visuals/ctd_camera.h	\
../../src/gamecode/player/ctd_inputhandler.inl	\
../../src/gamecode/player/ctd_playerimplclient.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_playernetworking.h	\
../../src/gamecode/player/networkingRoles/_RO_PlayerNetworking.h
	$(compile_source)

$(OBJ_DIR)/ctd_playerimplserver.o: ../../src/gamecode/player/ctd_playerimplserver.cpp	\
../../src/gamecode/player/ctd_playerimplserver.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h	\
../../src/gamecode/player/ctd_playerphysics.h	\
../../src/gamecode/player/ctd_playernetworking.h	\
../../src/gamecode/player/networkingRoles/_RO_PlayerNetworking.h
	$(compile_source)

$(OBJ_DIR)/ctd_playerimplstandalone.o: ../../src/gamecode/player/ctd_playerimplstandalone.cpp	\
../../src/gamecode/player/ctd_playerimplstandalone.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h	\
../../src/gamecode/player/ctd_chatgui.h	\
../../src/gamecode/player/ctd_playerphysics.h	\
../../src/gamecode/player/ctd_playeranim.h	\
../../src/gamecode/player/ctd_playersound.h	\
../../src/gamecode/player/ctd_inputhandler.h	\
../../src/gamecode/visuals/ctd_camera.h	\
../../src/gamecode/player/ctd_inputhandler.inl
	$(compile_source)

$(OBJ_DIR)/ctd_playerphysics.o: ../../src/gamecode/player/ctd_playerphysics.cpp	\
../../src/gamecode/player/ctd_playerphysics.h	\
../../src/gamecode/player/ctd_playersound.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h
	$(compile_source)

$(OBJ_DIR)/ctd_playersound.o: ../../src/gamecode/player/ctd_playersound.cpp	\
../../src/gamecode/player/ctd_playersound.h	\
../../src/gamecode/player/ctd_playerimpl.h	\
../../src/gamecode/player/ctd_player.h
	$(compile_source)

$(OBJ_DIR)/ctd_spawnpoint.o: ../../src/gamecode/player/ctd_spawnpoint.cpp	\
../../src/gamecode/player/ctd_spawnpoint.h	\
../../src/gamecode/player/ctd_player.h
	$(compile_source)

$(OBJ_DIR)/ctd_3dsound.o: ../../src/gamecode/sound/ctd_3dsound.cpp	\
../../src/gamecode/sound/ctd_3dsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_ambientsound.o: ../../src/gamecode/sound/ctd_ambientsound.cpp	\
../../src/gamecode/sound/ctd_ambientsound.h
	$(compile_source)

$(OBJ_DIR)/ctd_fpsdisplay.o: ../../src/gamecode/tools/ctd_fpsdisplay.cpp	\
../../src/gamecode/tools/ctd_fpsdisplay.h
	$(compile_source)

$(OBJ_DIR)/ctd_inspector.o: ../../src/gamecode/tools/ctd_inspector.cpp	\
../../src/gamecode/tools/ctd_inspector.h	\
../../src/gamecode/visuals/ctd_camera.h
	$(compile_source)

$(OBJ_DIR)/ctd_playerinfodisplay.o: ../../src/gamecode/tools/ctd_playerinfodisplay.cpp	\
../../src/gamecode/tools/ctd_playerinfodisplay.h	\
../../src/gamecode/player/ctd_player.h
	$(compile_source)

$(OBJ_DIR)/ctd_basecmd.o: ../../src/gamecode/tools/console/ctd_basecmd.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdregistry.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdcd.o: ../../src/gamecode/tools/console/ctd_cmdcd.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdcd.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdconfiglist.o: ../../src/gamecode/tools/console/ctd_cmdconfiglist.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdconfiglist.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdconfigset.o: ../../src/gamecode/tools/console/ctd_cmdconfigset.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdconfigset.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdconnect.o: ../../src/gamecode/tools/console/ctd_cmdconnect.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdconnect.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdentityattrlist.o: ../../src/gamecode/tools/console/ctd_cmdentityattrlist.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdentityattrlist.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdentityattrset.o: ../../src/gamecode/tools/console/ctd_cmdentityattrset.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdentityattrset.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdentitydump.o: ../../src/gamecode/tools/console/ctd_cmdentitydump.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdentitydump.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdentitylist.o: ../../src/gamecode/tools/console/ctd_cmdentitylist.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdentitylist.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdexec.o: ../../src/gamecode/tools/console/ctd_cmdexec.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdexec.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdgetconfig.o: ../../src/gamecode/tools/console/ctd_cmdgetconfig.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdgetconfig.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdhelp.o: ../../src/gamecode/tools/console/ctd_cmdhelp.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdhelp.h	\
../../src/gamecode/tools/console/ctd_cmdregistry.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdidle.o: ../../src/gamecode/tools/console/ctd_cmdidle.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdidle.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdlevellist.o: ../../src/gamecode/tools/console/ctd_cmdlevellist.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdlevellist.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdlistentity.o: ../../src/gamecode/tools/console/ctd_cmdlistentity.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdlistentity.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdloadentities.o: ../../src/gamecode/tools/console/ctd_cmdloadentities.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdloadentities.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdloadfinalize.o: ../../src/gamecode/tools/console/ctd_cmdloadfinalize.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdloadfinalize.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdloadlevel.o: ../../src/gamecode/tools/console/ctd_cmdloadlevel.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdloadlevel.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdlogclose.o: ../../src/gamecode/tools/console/ctd_cmdlogclose.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdlogclose.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdlogcreate.o: ../../src/gamecode/tools/console/ctd_cmdlogcreate.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdlogcreate.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdls.o: ../../src/gamecode/tools/console/ctd_cmdls.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdls.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdphysics.o: ../../src/gamecode/tools/console/ctd_cmdphysics.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdphysics.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdpwd.o: ../../src/gamecode/tools/console/ctd_cmdpwd.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdpwd.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdregistry.o: ../../src/gamecode/tools/console/ctd_cmdregistry.cpp	\
../../src/gamecode/tools/console/ctd_cmdregistry.h	\
../../src/gamecode/tools/console/ctd_basecmd.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdserverstart.o: ../../src/gamecode/tools/console/ctd_cmdserverstart.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdserverstart.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdsetconfig.o: ../../src/gamecode/tools/console/ctd_cmdsetconfig.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdsetconfig.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdshowfps.o: ../../src/gamecode/tools/console/ctd_cmdshowfps.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdshowfps.h	\
../../src/gamecode/tools/ctd_fpsdisplay.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdshowplayerinfo.o: ../../src/gamecode/tools/console/ctd_cmdshowplayerinfo.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdshowplayerinfo.h	\
../../src/gamecode/tools/ctd_playerinfodisplay.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdshutdown.o: ../../src/gamecode/tools/console/ctd_cmdshutdown.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdshutdown.h	\
../../src/gamecode/tools/console/ctd_console.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdtime.o: ../../src/gamecode/tools/console/ctd_cmdtime.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdtime.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdunloadlevel.o: ../../src/gamecode/tools/console/ctd_cmdunloadlevel.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdunloadlevel.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdversion.o: ../../src/gamecode/tools/console/ctd_cmdversion.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdversion.h
	$(compile_source)

$(OBJ_DIR)/ctd_cmdwriteosgfile.o: ../../src/gamecode/tools/console/ctd_cmdwriteosgfile.cpp	\
../../src/gamecode/tools/console/ctd_basecmd.h	\
../../src/gamecode/tools/console/ctd_cmdwriteosgfile.h	\
../../src/gamecode/tools/console/ctd_cmdregistry.h
	$(compile_source)

$(OBJ_DIR)/ctd_console.o: ../../src/gamecode/tools/console/ctd_console.cpp	\
../../src/gamecode/tools/console/ctd_console.h	\
../../src/gamecode/tools/console/ctd_cmdregistry.h	\
../../src/gamecode/tools/console/ctd_basecmd.h
	$(compile_source)

$(OBJ_DIR)/ctd_camera.o: ../../src/gamecode/visuals/ctd_camera.cpp	\
../../src/gamecode/visuals/ctd_camera.h
	$(compile_source)

$(OBJ_DIR)/ctd_candle.o: ../../src/gamecode/visuals/ctd_candle.cpp	\
../../src/gamecode/visuals/ctd_candle.h	\
../../src/gamecode/visuals/ctd_pointlight.h	\
../../src/gamecode/visuals/ctd_lightmanager.h
	$(compile_source)

$(OBJ_DIR)/ctd_fog.o: ../../src/gamecode/visuals/ctd_fog.cpp	\
../../src/gamecode/visuals/ctd_fog.h
	$(compile_source)

$(OBJ_DIR)/ctd_lightmanager.o: ../../src/gamecode/visuals/ctd_lightmanager.cpp	\
../../src/gamecode/visuals/ctd_lightmanager.h
	$(compile_source)

$(OBJ_DIR)/ctd_mesh.o: ../../src/gamecode/visuals/ctd_mesh.cpp	\
../../src/gamecode/visuals/ctd_mesh.h
	$(compile_source)

$(OBJ_DIR)/ctd_observer.o: ../../src/gamecode/visuals/ctd_observer.cpp	\
../../src/gamecode/visuals/ctd_observer.h	\
../../src/gamecode/visuals/ctd_camera.h
	$(compile_source)

$(OBJ_DIR)/ctd_pointlight.o: ../../src/gamecode/visuals/ctd_pointlight.cpp	\
../../src/gamecode/visuals/ctd_pointlight.h	\
../../src/gamecode/visuals/ctd_lightmanager.h
	$(compile_source)

$(OBJ_DIR)/ctd_skybox.o: ../../src/gamecode/visuals/ctd_skybox.cpp	\
../../src/gamecode/visuals/ctd_skybox.h
	$(compile_source)

$(OBJ_DIR)/ctd_spotlight.o: ../../src/gamecode/visuals/ctd_spotlight.cpp	\
../../src/gamecode/visuals/ctd_spotlight.h	\
../../src/gamecode/visuals/ctd_lightmanager.h
	$(compile_source)

$(OBJ_DIR)/ctd_water.o: ../../src/gamecode/visuals/ctd_water.cpp	\
../../src/gamecode/visuals/ctd_water.h	\
../../src/gamecode/extern/Noise.h
	$(compile_source)

$(OBJ_DIR)/ctd_exceptionlogger.o: ../../src/gamecode/ctd_exceptionlogger.cpp	\
../../src/gamecode/ctd_main.h	\
../../src/gamecode/ctd_exceptionlogger.h
	$(compile_source)

$(OBJ_DIR)/ctd_gameutils.o: ../../src/gamecode/ctd_gameutils.cpp	\
../../src/gamecode/ctd_main.h	\
../../src/gamecode/ctd_gameutils.h
	$(compile_source)

$(OBJ_DIR)/ctd_main.o: ../../src/gamecode/ctd_main.cpp	\
../../src/gamecode/ctd_main.h
	$(compile_source)



