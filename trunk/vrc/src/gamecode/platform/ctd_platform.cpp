/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/
//--------------------------------------------------------//
//           memory lead detection definitions            //
//--------------------------------------------------------//
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__) //
#define new DEBUG_NEW                                     //
#undef THIS_FILE                                          //
static char THIS_FILE[] = __FILE__;                       //
//--------------------------------------------------------//

#include <ctd_base.h>
#include "ctd_platform.h"
#include <ctd_levelmanager.h>

using namespace std;
using namespace CTD; 
using namespace Producer; 


//! Implement and register the platform entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlatformEntityFactory );


EnPlatform::EnPlatform():
_speed(0.5f),
_loop(true)
{
    EntityManager::get()->registerUpdate( this );   // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "meshFile", _meshFile );
    _attributeManager.addAttribute( "position", _position );
    _attributeManager.addAttribute( "speed",    _speed    );
    _attributeManager.addAttribute( "loop",     _loop     );
}

EnPlatform::~EnPlatform()
{
    // deregister entity, it is not necessary for entities which 'die' at application exit time
    //  as the entity manager clears the entity list on app exit
    EntityManager::get()->registerUpdate( this, false );
}

void EnPlatform::initialize()
{
    osg::Node* p_mesh = LevelManager::get()->loadMesh( _meshFile );
    if ( !p_mesh ) 
    {
        cout << "*** error loading mesh file" << endl;
        return;
    }
    // now we add the new mesh into our transformable scene group
    addToTransformableNode( p_mesh );
    setPosition( _position );
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
