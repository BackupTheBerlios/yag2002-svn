INCLUDES = -I$(top_srcdir)/src/framework $(CEGUI_CFLAGS) $(SDL_CFLAGS) $(OSGSDL_CFLAGS) $(CAL3D_CFLAGS) $(AM_CFLAGS)

noinst_PROGRAMS = vrc
vrc_LDFLAGS = -L$(top_builddir)/src/framework/.libs 

vrc_LDADD = -lpthread -lYAF3D $(CEGUI_LIBS) $(SDL_LIBS) $(OPENAL_LIBS) $(VORBIS_LIBS)  $(OSGAL_LIBS) $(OSGSDL_LIBS) $(AM_LDFLAGS) \
$(OSGCAL_LIBS) $(CAL3D_LIBS) -losgCal -lNewton -lRNReplicaNet -lRNXPSession -lRNXPSockets -lRNXPURL -lRNPlatform -lRNXPCompression -lfmodex -lspeex 

vrc_SOURCES = vrc_gameutils.cpp \
			vrc_main.cpp \
			extern/Noise.cpp \
			networking/_Def_replicaregs.cpp \
			gui/vrc_dialoglevelselect.cpp \
			gui/vrc_dialogplayercfg.cpp \
			gui/vrc_dialogsettings.cpp \
			gui/vrc_microinput.cpp \
			gui/vrc_intro.cpp \
			gui/vrc_menu.cpp \
			gui/vrc_mapview.cpp \
			npc/fish/vrc_npcfish.cpp \
			scripting/vrc_script.cpp \
			physics/vrc_geomemitter.cpp \
			physics/vrc_physicsbase.cpp \
			physics/vrc_physicsbox.cpp \
			physics/vrc_physicscylinder.cpp \
			physics/vrc_physicssphere.cpp \
			platform/vrc_platform.cpp \
			player/vrc_inputhandler.cpp \
			player/vrc_player.cpp \
			player/vrc_playeranim.cpp \
			player/vrc_playerimpl.cpp \
			player/vrc_playerimplclient.cpp \
			player/vrc_playerimplserver.cpp \
			player/vrc_playerimplstandalone.cpp \
			player/vrc_playernetworking.cpp \
			player/vrc_playerphysics.cpp \
			player/vrc_playersound.cpp \
			player/vrc_playernamedisp.cpp \
			player/vrc_spawnpoint.cpp \
			player/chat/vrc_chatmgr.cpp \
			player/chat/vrc_chatguibox.cpp \
			player/chat/vrc_chatguiboxtab.cpp \
			player/chat/vrc_chatguictrl.cpp \
			player/chat/vrc_chatprotocol.cpp \
			player/chat/vrc_chatserverVRC.cpp \
			player/chat/IRC/vrc_chatprotIRC.cpp \
			player/chat/IRC/libIRC/libircclient.c \
			player/chat/VRC/vrc_chatprotVRC.cpp \
			player/chat/VRC/networkingRoles/_RO_PlayerChat.cpp \
			player/networkingRoles/_RO_PlayerNetworking.cpp \
			sound/vrc_3dsound.cpp \
			sound/vrc_ambientsound.cpp \
			tools/vrc_fpsdisplay.cpp \
			tools/vrc_inspector.cpp \
			tools/vrc_playerinfodisplay.cpp \
			tools/console/vrc_basecmd.cpp \
			tools/console/vrc_cmdcd.cpp \
			tools/console/vrc_cmdconfiglist.cpp \
			tools/console/vrc_cmdconfigset.cpp \
			tools/console/vrc_cmdconfigget.cpp \
			tools/console/vrc_cmdconnect.cpp \
			tools/console/vrc_cmdentityattrlist.cpp \
			tools/console/vrc_cmdentityattrset.cpp \
			tools/console/vrc_cmdentitydump.cpp \
			tools/console/vrc_cmdentitylist.cpp \
			tools/console/vrc_cmdexec.cpp \
			tools/console/vrc_cmdhelp.cpp \
			tools/console/vrc_cmdidle.cpp \
			tools/console/vrc_cmdlevellist.cpp \
			tools/console/vrc_cmdloadentities.cpp \
			tools/console/vrc_cmdloadfinalize.cpp \
			tools/console/vrc_cmdloadlevel.cpp \
			tools/console/vrc_cmdlogclose.cpp \
			tools/console/vrc_cmdlogcreate.cpp \
			tools/console/vrc_cmdls.cpp \
			tools/console/vrc_cmdphysicsrender.cpp \
			tools/console/vrc_cmdphysicsserialize.cpp \
			tools/console/vrc_cmdpwd.cpp \
			tools/console/vrc_cmdregistry.cpp \
			tools/console/vrc_cmdserverstart.cpp \
			tools/console/vrc_cmdshowfps.cpp \
			tools/console/vrc_cmdshowplayerinfo.cpp \
			tools/console/vrc_cmdshutdown.cpp \
			tools/console/vrc_cmdtime.cpp \
			tools/console/vrc_cmdunloadlevel.cpp \
			tools/console/vrc_cmdversion.cpp \
			tools/console/vrc_cmdwriteosgfile.cpp \
			tools/console/vrc_console.cpp \
			tools/console/vrc_consoleiocin.cpp \
			tools/console/vrc_consoleiogui.cpp \
			tools/console/vrc_cmdrenderingmode.cpp \
			visuals/vrc_camera.cpp \
			visuals/vrc_candle.cpp \
			visuals/vrc_fog.cpp \
			visuals/vrc_lightmanager.cpp \
			visuals/vrc_mesh.cpp \
			visuals/vrc_observer.cpp \
			visuals/vrc_pointlight.cpp \
			visuals/vrc_skybox.cpp \
			visuals/vrc_spotlight.cpp \
			visuals/vrc_water.cpp \
			voice/vrc_codec.cpp \
			voice/vrc_netvoice.cpp \
			voice/vrc_networksoundimpl.cpp \
			voice/vrc_voiceinput.cpp \
			voice/vrc_voicenetwork.cpp \
			voice/vrc_voicereceiver.cpp \
			voice/vrc_voicesender.cpp \
			voice/vrc_voicetestutils.cpp

install-exec-local:
	$(INSTALL_PROGRAM) $(top_builddir)/src/gamecode/vrc $(top_srcdir)/bin/linux
