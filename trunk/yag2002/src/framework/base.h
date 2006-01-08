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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/


#ifndef _BASE_H_
#define _BASE_H_

#ifdef HAVE_CONFIG_H 
  #include <config.h>
#endif

// YAF3D version
#define YAF3D_VERSION     "1.0.2"

// STL headers
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

// ms windows stuff
#ifdef WIN32
#  define NOMINMAX
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <shellapi.h>
#  endif
#endif


// SDL header
#include <SDL.h>

// osg headers
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/ColorMask>
#include <osg/ClipNode>
#include <osg/CoordinateSystemNode>
#include <osg/Depth>
#include <osg/Fog>
#include <osg/Material>
#include <osg/Math>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/LightSource>
#include <osg/ShapeDrawable>
#include <osg/Stencil>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TexEnvCombine>
#include <osg/TextureCubeMap>
#include <osg/Transform>
#include <osg/PositionAttitudeTransform>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/VertexProgram>
#include <osgDB/ReadFile>
#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>
#include <osgUtil/Optimizer>

#include <osgSDL/Viewer>

// Physics library header
#include <Newton.h>

// CEGUI headers
#include <CEGUI.h>

// fmod sound headers
#include <fmod.hpp>

#endif // _BASE_H_
