/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # source file for building precompiled headers
 #
 #   date of creation:  02/15/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ################################################################*/


#ifndef _BASE_H_
#define _BASE_H_

#ifdef HAVE_CONFIG_H 
  #include <config.h>
#endif

// YAF3D version
#define YAF3D_VERSION     "1.3.0"

// STL headers
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

// ms windows stuff
#ifdef WIN32
#  define NOMINMAX
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <shellapi.h>
#    // disable the warnings about auto-creation of assignment operator
#    pragma warning ( disable: 4512 )
#  endif
#endif


// SDL header
#include <SDL.h>
#include <osgSDL/Viewer>

// osg headers
#include <osg/PositionAttitudeTransform>
#include <osg/CoordinateSystemNode>
#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>
#include <osg/MatrixTransform>
#include <osg/TextureCubeMap>
#include <osgUtil/Optimizer>
#include <osg/TexEnvCombine>
#include <osg/ShapeDrawable>
#include <osg/VertexProgram>
#include <osg/LightSource>
#include <osgDB/ReadFile>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/ColorMask>
#include <osg/Texture2D>
#include <osg/Transform>
#include <osg/ClipNode>
#include <osg/Material>
#include <osgText/Text>
#include <osg/Stencil>
#include <osg/Texture>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/Matrix>
#include <osg/Depth>
#include <osg/Math>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Fog>


// Physics library header
#include <Newton.h>

// CEGUI headers
#include <CEGUI.h>

// fmod sound headers
#include <fmod.hpp>

#endif // _BASE_H_
