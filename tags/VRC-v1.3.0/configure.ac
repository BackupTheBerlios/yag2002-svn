#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([YAF3D],[1.3.0])


# fix warning caused by SDL stuff
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([YAF3D], [1.3.0])
AC_CONFIG_SRCDIR([src/framework/levelmanager.h])
AC_CONFIG_HEADER([config.h])

# Templates
AH_TEMPLATE([_DEBUG], [Define if building with debug symbols])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

# configure options
AC_ARG_ENABLE(debug, AS_HELP_STRING(--enable-debug,Build with debugging symbols), , enable_debug=no)

if test "x$enable_debug" = "xno"
then
    CFLAGS="-O3 -g -W -Wall"
    CXXFLAGS="-fomit-frame-pointer -O3 -g -W -Wall -pedantic -ansi"
else
    CFLAGS="-g3 -O0"
    CXXFLAGS="-g3 -O0"
    AC_DEFINE([_DEBUG], 1)
fi

# Checks for libraries.
PKG_CHECK_MODULES(CEGUI, CEGUI >= 0.3.0)
PKG_CHECK_MODULES(OSGSDL, osgSDL >= 0)
PKG_CHECK_MODULES(CAL3D, cal3d >= 0.10.0)

# Check for SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

AM_CFLAGS="$AM_CFLAGS -DLINUX -D_NEWTON_USE_LIB -I/usr/local/include/fmodex"
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

YAF3D_CURRENT=1
YAF3D_REVISION=3
YAF3D_AGE=0
YAF3D_VERSION_INFO=$YAF3D_CURRENT:$YAF3D_REVISION:$YAF3D_AGE
AC_SUBST(YAF3D_VERSION_INFO)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/framework/Makefile
                 src/gamecode/Makefile
                 YAF3D.pc])
AC_OUTPUT
