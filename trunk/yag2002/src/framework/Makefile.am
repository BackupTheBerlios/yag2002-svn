INCLUDES = -DYAF3D_HAS_CONSOLE $(CEGUI_CFLAGS) $(SDL_CFLAGS) $(OPENAL_CFLAGS) $(VORBIS_CFLAGS) $(OSGAL_CFLAGS) $(OSGSDL_CFLAGS) $(AM_CFLAGS)
lib_LTLIBRARIES=libYAF3D.la

libYAF3D_la_SOURCES = 	application.cpp \
			attributemanager.cpp \
			attributemanager.inl \
			base.cpp \
			baseentity.cpp \
			configuration.cpp \
			entitymanager.cpp \
			gamestate.cpp \
			guimanager.cpp \
			guimsgbox.cpp \
			guirenderer.cpp \
			guitexture.cpp \
			keymap.cpp \
			levelmanager.cpp \
			log.cpp \
			network.cpp \
			physics.cpp \
			physics_helpers.cpp \
			settings.cpp \
			settings.inl \
			soundmanager.cpp\
			utils.cpp \
			yaf3dtinyxml/tinystr.cpp \
			yaf3dtinyxml/tinyxml.cpp \
			yaf3dtinyxml/tinyxmlerror.cpp \
			yaf3dtinyxml/tinyxmlparser.cpp


pkginclude_HEADERS = 	application.h \
			attributemanager.h \
			attributemanager.inl \
			base.h \
			baseentity.h \
			baseentity.inl \
			configuration.h \
			entitymanager.h \
			gamestate.h \
			guimanager.h \
			guimsgbox.h \
			guirenderer.h \
			guitexture.h \
			keymap.h \
			levelmanager.h \
			log.h \
			network.h \
			network_defs.h \
			physics.h \
			physics_helpers.h \
			settings.h \
			singleton.h \
			singleton.inl \
			soundmanager.h\
			utils.h \
			yaf3dframework.h \
			yaf3dtinyxml/tinystr.h \
			yaf3dtinyxml/tinyxml.h



noinst_HEADERS = 	yaf3dtinyxml/tinystr.h \
			yaf3dtinyxml/tinyxml.h


libYAF3D_la_LDFLAGS = -version-info @YAF3D_VERSION_INFO@
libYAF3D_la_LIBADD =