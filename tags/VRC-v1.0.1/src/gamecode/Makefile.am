INCLUDES = -I$(top_srcdir)/src/framework $(CEGUI_CFLAGS) $(SDL_CFLAGS) $(OPENAL_CFLAGS) $(VORBIS_CFLAGS) $(OSGAL_CFLAGS) $(OSGSDL_CFLAGS) $(CAL3D_CFLAGS) $(AM_CFLAGS)

noinst_PROGRAMS = vrc
vrc_LDFLAGS = -L$(top_builddir)/src/framework/.libs 

vrc_LDADD = -lpthread -lCTDFramework $(CEGUI_LIBS) $(SDL_LIBS) $(OPENAL_LIBS) $(VORBIS_LIBS)  $(OSGAL_LIBS) $(OSGSDL_LIBS) $(AM_LDFLAGS) \
$(OSGCAL_LIBS) $(CAL3D_LIBS) -losgCal -lNewton -lRNReplicaNet -lRNXPSession -lRNXPURL -lRNXPCompression -lRNXPSockets -lRNPlatform


vrc_SOURCES = ctd_exceptionlogger.cpp \
			ctd_gameutils.cpp \
			ctd_main.cpp \
			extern/Noise.cpp \
			gui/ctd_dialoglevelselect.cpp \
			gui/ctd_dialogplayercfg.cpp \
			gui/ctd_dialogsettings.cpp \
			gui/ctd_intro.cpp \
			gui/ctd_menu.cpp \
			physics/ctd_geomemitter.cpp \
			physics/ctd_physicsbase.cpp \
			physics/ctd_physicsbox.cpp \
			physics/ctd_physicscylinder.cpp \
			physics/ctd_physicssphere.cpp \
			platform/ctd_platform.cpp \
			player/ctd_chatgui.cpp \
			player/ctd_inputhandler.cpp \
			player/ctd_player.cpp \
			player/ctd_playeranim.cpp \
			player/ctd_playerimpl.cpp \
			player/ctd_playerimplclient.cpp \
			player/ctd_playerimplserver.cpp \
			player/ctd_playerimplstandalone.cpp \
			player/ctd_playernetworking.cpp \
			player/ctd_playerphysics.cpp \
			player/ctd_playersound.cpp \
			player/ctd_spawnpoint.cpp \
			player/networkingRoles/_Def_Chat.cpp \
			player/networkingRoles/_RO_PlayerNetworking.cpp \
			sound/ctd_3dsound.cpp \
			sound/ctd_ambientsound.cpp \
			tools/ctd_fpsdisplay.cpp \
			tools/ctd_inspector.cpp \
			tools/ctd_playerinfodisplay.cpp \
			tools/console/ctd_basecmd.cpp \
			tools/console/ctd_cmdcd.cpp \
			tools/console/ctd_cmdconfiglist.cpp \
			tools/console/ctd_cmdconfigset.cpp \
			tools/console/ctd_cmdconnect.cpp \
			tools/console/ctd_cmdentityattrlist.cpp \
			tools/console/ctd_cmdentityattrset.cpp \
			tools/console/ctd_cmdentitydump.cpp \
			tools/console/ctd_cmdentitylist.cpp \
			tools/console/ctd_cmdexec.cpp \
			tools/console/ctd_cmdgetconfig.cpp \
			tools/console/ctd_cmdhelp.cpp \
			tools/console/ctd_cmdidle.cpp \
			tools/console/ctd_cmdlevellist.cpp \
			tools/console/ctd_cmdlistentity.cpp \
			tools/console/ctd_cmdloadentities.cpp \
			tools/console/ctd_cmdloadfinalize.cpp \
			tools/console/ctd_cmdloadlevel.cpp \
			tools/console/ctd_cmdlogclose.cpp \
			tools/console/ctd_cmdlogcreate.cpp \
			tools/console/ctd_cmdls.cpp \
			tools/console/ctd_cmdphysics.cpp \
			tools/console/ctd_cmdpwd.cpp \
			tools/console/ctd_cmdregistry.cpp \
			tools/console/ctd_cmdserverstart.cpp \
			tools/console/ctd_cmdsetconfig.cpp \
			tools/console/ctd_cmdshowfps.cpp \
			tools/console/ctd_cmdshowplayerinfo.cpp \
			tools/console/ctd_cmdshutdown.cpp \
			tools/console/ctd_cmdtime.cpp \
			tools/console/ctd_cmdunloadlevel.cpp \
			tools/console/ctd_cmdversion.cpp \
			tools/console/ctd_cmdwriteosgfile.cpp \
			tools/console/ctd_console.cpp \
			visuals/ctd_camera.cpp \
			visuals/ctd_candle.cpp \
			visuals/ctd_fog.cpp \
			visuals/ctd_lightmanager.cpp \
			visuals/ctd_mesh.cpp \
			visuals/ctd_observer.cpp \
			visuals/ctd_pointlight.cpp \
			visuals/ctd_skybox.cpp \
			visuals/ctd_spotlight.cpp \
			visuals/ctd_water.cpp

install-exec-local:
	$(INSTALL_PROGRAM) $(top_builddir)/src/gamecode/vrc $(top_srcdir)/bin/linux
