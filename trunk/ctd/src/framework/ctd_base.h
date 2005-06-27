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
 # source file for building precompiled headers
 #
 #   date of creation:  02/15/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/


#ifndef _CTD_BASE_H_
#define _CTD_BASE_H_

// ms windows stuff
#ifdef WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <stdlib.h>
#  include <windows.h>
#endif

// CEGUI headers
#include <CEGUI.h>

// osg headers
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/ColorMask>
#include <osg/ClipNode>
#include <osg/CoordinateSystemNode>
#include <osg/Depth>
#include <osg/fog>
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

#include <osgAL/SoundNode>
#include <osgAL/SoundRoot>
#include <osgAL/SoundManager>
#include <osgAL/SoundState>

#include <osgDB/ReadFile>

#include <osgGA/guiactionadapter>
#include <osgGA/guieventadapter>

#include <osgProducer/Viewer>

#include <osgUtil/Optimizer>

// Physics library header
#include <Newton.h>

// STL headers
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <assert.h>

// VRC version
#define VRC_VERSION     "0.5"

#endif // _CTD_BASE_H_
