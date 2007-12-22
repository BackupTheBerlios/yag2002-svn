/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ****************************************************************/

/*###############################################################
 # main header used for precompiled headers
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _MAIN_H_
#define _MAIN_H_

//! Editor version
#define STORYEDITOR_VERSION     "0.1.0"
//! Config file name
#define TOOL_CONFIG_FILE        "editor.cfg"

//! STL headers
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

//! MS windows stuff
#ifdef WIN32
#  define NOMINMAX
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <shellapi.h>
#  endif
#endif

//! OGL header
#include <GL/gl.h>
#include <GL/glu.h>

//! wxWidget's main headers
#include <wx/wxprec.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/statusbr.h>
#include <wx/glcanvas.h>
#include <wx/toolbar.h>
#include <wx/frame.h>

//! 3rd party headers
#include <3rdparty/eigen/src/matrix.h>
#include <3rdparty/eigen/src/projective.h>
#include <3rdparty/eigen/src/vector.h>
#include <3rdparty/eigen/src/util.h>
#include <3rdparty/freetype/include/ft2build.h>
#include <3rdparty/FTGL/include/FTGL.h>
#include <3rdparty/FTGL/include/FTGLPixmapFont.h>

//! Internal stuff
#include <core/log.h>
#include <core/core.h>
#include <core/utils.h>
#include <core/smartptr.h>
#include <core/singleton.h>
#include <core/elementfactory.h>

#endif // _MAIN_H_
