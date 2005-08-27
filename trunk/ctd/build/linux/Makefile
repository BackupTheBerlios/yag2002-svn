#################################################################
## This Makefile Exported by MinGW Developer Studio
## Copyright (c) 2005 by Parinya Thipchart
#################################################################

ifneq (,$(findstring Release, $(CFG)))
  override CFG = Release
else
  override CFG = Debug
endif

PROJECT = framework
CC = "/usr/bin/g++"

ifeq ($(CFG),Debug)
  OBJ_DIR = Debug
  OUTPUT_DIR = Debug
  TARGET = libframework.so
  C_INCLUDE_DIRS = -I"../../src/framework" -I"/usr/local/include" -I"/usr/local/include/CEGUI" -I"/usr/include/SDL" -I"/usr/local/include/tinyxml"
  C_PREPROC = -DLINUX 
  CFLAGS = -pipe  -Wall -g2 -O0 -frtti -fexceptions -fPIC 
  LIB_DIRS = 
  LIBS = 
  LDFLAGS = -pipe -shared 
endif

ifeq ($(CFG),Release)
  OBJ_DIR = Release
  OUTPUT_DIR = Release
  TARGET = libframework.so
  C_INCLUDE_DIRS = -I"../../src/framework" -I"/usr/local/include" -I"/usr/local/include/CEGUI" -I"/usr/include/SDL" -I"/usr/local/include/tinyxml"
  C_PREPROC = -DLINUX 
  CFLAGS = -pipe  -Wall -g0 -O2 -frtti -fexceptions -fPIC 
  LIB_DIRS = 
  LIBS = 
  LDFLAGS = -pipe -shared -s 
endif

SRC_OBJS = \
  $(OBJ_DIR)/ctd_application.o	\
  $(OBJ_DIR)/ctd_attributemanager.o	\
  $(OBJ_DIR)/ctd_base.o	\
  $(OBJ_DIR)/ctd_baseentity.o	\
  $(OBJ_DIR)/ctd_configuration.o	\
  $(OBJ_DIR)/ctd_entitymanager.o	\
  $(OBJ_DIR)/ctd_gamestate.o	\
  $(OBJ_DIR)/ctd_guimanager.o	\
  $(OBJ_DIR)/ctd_guimsgbox.o	\
  $(OBJ_DIR)/ctd_guirenderer.o	\
  $(OBJ_DIR)/ctd_guitexture.o	\
  $(OBJ_DIR)/ctd_keymap.o	\
  $(OBJ_DIR)/ctd_levelmanager.o	\
  $(OBJ_DIR)/ctd_log.o	\
  $(OBJ_DIR)/ctd_main.o	\
  $(OBJ_DIR)/ctd_network.o	\
  $(OBJ_DIR)/ctd_physics.o	\
  $(OBJ_DIR)/ctd_physics_helpers.o	\
  $(OBJ_DIR)/ctd_settings.o	\
  $(OBJ_DIR)/ctd_utils.o


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
	@echo Deleting intermediate files for 'framework - $(CFG)'
	-@rm -rf "$(OBJ_DIR)"
	-@rm -rf "$(OUTPUT_DIR)/$(TARGET)"
	-@rmdir "$(OUTPUT_DIR)"

clean:
	@echo Deleting intermediate files for 'framework - $(CFG)'
	-@rm -rf "$(OBJ_DIR)"

print_header:
	@echo ----------Configuration: framework - Linux $(CFG)----------

directories:
	-@if [ ! -d "$(OUTPUT_DIR)" ]; then mkdir "$(OUTPUT_DIR)"; fi
	-@if [ ! -d "$(OBJ_DIR)" ]; then mkdir "$(OBJ_DIR)"; fi

$(OBJ_DIR)/ctd_application.o: ../../src/framework/ctd_application.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_levelmanager.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_entitymanager.h	\
../../src/framework/ctd_baseentity.h	\
../../src/framework/ctd_attributemanager.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_attributemanager.inl	\
../../src/framework/ctd_baseentity.inl	\
../../src/framework/ctd_physics.h	\
../../src/framework/ctd_guimanager.h	\
../../src/framework/ctd_guirenderer.h	\
../../src/framework/ctd_utils.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_configuration.h	\
../../src/framework/ctd_settings.h	\
../../src/framework/ctd_settings_inl.h	\
../../src/framework/ctd_keymap.h	\
../../src/framework/ctd_gamestate.h	\
../../src/framework/ctd_network.h	\
../../src/framework/ctd_network_defs.h
	$(compile_source)

$(OBJ_DIR)/ctd_attributemanager.o: ../../src/framework/ctd_attributemanager.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_attributemanager.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_attributemanager.inl
	$(compile_source)

$(OBJ_DIR)/ctd_base.o: ../../src/framework/ctd_base.cpp	\
../../src/framework/ctd_base.h
	$(compile_source)

$(OBJ_DIR)/ctd_baseentity.o: ../../src/framework/ctd_baseentity.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_baseentity.h	\
../../src/framework/ctd_attributemanager.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_attributemanager.inl	\
../../src/framework/ctd_baseentity.inl	\
../../src/framework/ctd_entitymanager.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl
	$(compile_source)

$(OBJ_DIR)/ctd_configuration.o: ../../src/framework/ctd_configuration.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_configuration.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_settings.h	\
../../src/framework/ctd_settings_inl.h	\
../../src/framework/ctd_application.h
	$(compile_source)

$(OBJ_DIR)/ctd_entitymanager.o: ../../src/framework/ctd_entitymanager.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_entitymanager.h	\
../../src/framework/ctd_baseentity.h	\
../../src/framework/ctd_attributemanager.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_attributemanager.inl	\
../../src/framework/ctd_baseentity.inl	\
../../src/framework/ctd_utils.h
	$(compile_source)

$(OBJ_DIR)/ctd_gamestate.o: ../../src/framework/ctd_gamestate.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_gamestate.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_application.h
	$(compile_source)

$(OBJ_DIR)/ctd_guimanager.o: ../../src/framework/ctd_guimanager.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_guimanager.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_guirenderer.h	\
../../src/framework/ctd_utils.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_configuration.h	\
../../src/framework/ctd_settings.h	\
../../src/framework/ctd_settings_inl.h	\
../../src/framework/ctd_keymap.h	\
../../src/framework/ctd_log.h
	$(compile_source)

$(OBJ_DIR)/ctd_guimsgbox.o: ../../src/framework/ctd_guimsgbox.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_utils.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_guimsgbox.h	\
../../src/framework/ctd_guimanager.h	\
../../src/framework/ctd_guirenderer.h
	$(compile_source)

$(OBJ_DIR)/ctd_guirenderer.o: ../../src/framework/ctd_guirenderer.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_guitexture.h	\
../../src/framework/ctd_guirenderer.h
	$(compile_source)

$(OBJ_DIR)/ctd_guitexture.o: ../../src/framework/ctd_guitexture.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_guitexture.h	\
../../src/framework/ctd_guirenderer.h
	$(compile_source)

$(OBJ_DIR)/ctd_keymap.o: ../../src/framework/ctd_keymap.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_keymap.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_application.h
	$(compile_source)

$(OBJ_DIR)/ctd_levelmanager.o: ../../src/framework/ctd_levelmanager.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_physics.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_levelmanager.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_attributemanager.h	\
../../src/framework/ctd_attributemanager.inl	\
../../src/framework/ctd_entitymanager.h	\
../../src/framework/ctd_baseentity.h	\
../../src/framework/ctd_baseentity.inl	\
../../src/framework/ctd_guimanager.h	\
../../src/framework/ctd_guirenderer.h	\
../../src/framework/ctd_utils.h	\
../../src/framework/ctd_gamestate.h
	$(compile_source)

$(OBJ_DIR)/ctd_log.o: ../../src/framework/ctd_log.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_log.h
	$(compile_source)

$(OBJ_DIR)/ctd_main.o: ../../src/framework/ctd_main.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl
	$(compile_source)

$(OBJ_DIR)/ctd_network.o: ../../src/framework/ctd_network.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_network.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_network_defs.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_utils.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_configuration.h	\
../../src/framework/ctd_settings.h	\
../../src/framework/ctd_settings_inl.h
	$(compile_source)

$(OBJ_DIR)/ctd_physics.o: ../../src/framework/ctd_physics.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_physics.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_physics_helpers.h
	$(compile_source)

$(OBJ_DIR)/ctd_physics_helpers.o: ../../src/framework/ctd_physics_helpers.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_physics_helpers.h	\
../../src/framework/ctd_physics.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl
	$(compile_source)

$(OBJ_DIR)/ctd_settings.o: ../../src/framework/ctd_settings.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_settings.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_settings_inl.h
	$(compile_source)

$(OBJ_DIR)/ctd_utils.o: ../../src/framework/ctd_utils.cpp	\
../../src/framework/ctd_base.h	\
../../src/framework/ctd_application.h	\
../../src/framework/ctd_singleton.h	\
../../src/framework/ctd_singleton.inl	\
../../src/framework/ctd_log.h	\
../../src/framework/ctd_utils.h
	$(compile_source)



