INCLUDES = $(CEGUI_CFLAGS) $(SDL_CFLAGS) $(OPENAL_CFLAGS) $(VORBIS_CFLAGS) $(OSGAL_CFLAGS) $(OSGSDL_CFLAGS) $(AM_CFLAGS)
lib_LTLIBRARIES=libCTDFramework.la

libCTDFramework_la_SOURCES = ctd_application.cpp \
							ctd_attributemanager.cpp \
							ctd_attributemanager.inl \
							ctd_base.cpp \
							ctd_baseentity.cpp \
							ctd_baseentity.inl \
							ctd_configuration.cpp \
							ctd_entitymanager.cpp \
							ctd_gamestate.cpp \
							ctd_guimanager.cpp \
							ctd_guimsgbox.cpp \
							ctd_guirenderer.cpp \
							ctd_guitexture.cpp \
							ctd_keymap.cpp \
							ctd_levelmanager.cpp \
							ctd_log.cpp \
							ctd_network.cpp \
							ctd_physics.cpp \
							ctd_physics_helpers.cpp \
							ctd_settings.cpp \
							ctd_settings.inl \
							ctd_singleton.inl \
							ctd_utils.cpp \
							ctdtinyxml/tinystr.cpp \
							ctdtinyxml/tinyxml.cpp \
							ctdtinyxml/tinyxmlerror.cpp \
							ctdtinyxml/tinyxmlparser.cpp


pkginclude_HEADERS = ctd_application.h \
					ctd_attributemanager.h \
					ctd_base.h \
					ctd_baseentity.h \
					ctd_configuration.h \
					ctd_entitymanager.h \
					ctd_framework.h \
					ctd_gamestate.h \
					ctd_guimanager.h \
					ctd_guimsgbox.h \
					ctd_guirenderer.h \
					ctd_guitexture.h \
					ctd_keymap.h \
					ctd_levelmanager.h \
					ctd_log.h \
					ctd_network.h \
					ctd_network_defs.h \
					ctd_physics.h \
					ctd_physics_helpers.h \
					ctd_settings.h \
					ctd_singleton.h \
					ctd_utils.h

noinst_HEADERS = ctdtinyxml/tinystr.h \
				ctdtinyxml/tinyxml.h


libCTDFramework_la_LDFLAGS = -version-info @CTDFWK_VERSION_INFO@
libCTDFramework_la_LIBADD =