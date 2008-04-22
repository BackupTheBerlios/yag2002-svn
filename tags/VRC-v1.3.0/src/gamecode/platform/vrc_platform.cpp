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
 # entity Platform
 #
 #   date of creation:  02/18/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_platform.h"

namespace vrc
{

//! Implement and register the platform entity factory
YAF3D_IMPL_ENTITYFACTORY( PlatformEntityFactory )


EnPlatform::EnPlatform():
_speed(0.5f),
_loop(true)
{
    // register entity attributes
    getAttributeManager().addAttribute( "meshFile", _meshFile );
    getAttributeManager().addAttribute( "position", _position );
    getAttributeManager().addAttribute( "speed",    _speed    );
    getAttributeManager().addAttribute( "loop",     _loop     );
}

EnPlatform::~EnPlatform()
{
}

void EnPlatform::initialize()
{
    osg::Node* p_mesh = yaf3d::LevelManager::get()->loadMesh( _meshFile );
    if ( !p_mesh ) 
    {
        log_error << "*** error loading mesh file" << std::endl;
        return;
    }
    // now we add the new mesh into our transformable scene group
    addToTransformationNode( p_mesh );
    setPosition( _position );

    // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerUpdate( this );   
}

void EnPlatform::updateEntity( float deltaTime )
{
    //! experimental
    static float a = 0;
    a += deltaTime;
    float x = sin( a );
    float y = cos( a );
    setPosition( osg::Vec3f( _position._v[ 0 ] + x, _position._v[ 1 ] + y, _position._v[ 2 ] ) );

    osg::Quat quat;
    quat.makeRotate( x, y, 0, 1 );
    setRotation( quat );
}

} // namespace vrc
