#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([CTDFramework], [0.5.0])

# fix warning caused by SDL stuff
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([CTDFramework], [0.5.0])
AC_CONFIG_SRCDIR([src/framework/ctd_levelmanager.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES(CEGUI, CEGUI >= 0.3.0)
PKG_CHECK_MODULES(VORBIS, vorbis >= 1.1.0)
PKG_CHECK_MODULES(OPENAL, openalpp >= 0.2)
PKG_CHECK_MODULES(OSGAL, osgal >= 0)
PKG_CHECK_MODULES(OSGSDL, osgSDL >= 0)
PKG_CHECK_MODULES(CAL3D, cal3d >= 0.10.0)

# Check for SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

AM_CFLAGS="$AM_CFLAGS -DLINUX"
AM_LDFLAGS="$AM_LDFLAGS"
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_STAT
AC_CHECK_FUNCS([getcwd memset sqrt])

CTDFWK_CURRENT=5
CTDFWK_REVISION=0
CTDFWK_AGE=0
CTDFWK_VERSION_INFO=$CTDFWK_CURRENT:$CTDFWK_REVISION:$CTDFWK_AGE
AC_SUBST(CTDFWK_VERSION_INFO)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/framework/Makefile
                 src/gamecode/Makefile
                 CTDFramework.pc])
AC_OUTPUT
