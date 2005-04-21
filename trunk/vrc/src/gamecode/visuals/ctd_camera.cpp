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
 # entity camera
 #  this entity can be attached and used by other cameras 
 #
 #   date of creation:  04/21/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_levelmanager.h>
#include <ctd_log.h>
#include "ctd_camera.h"

using namespace std;
using namespace osg; 

namespace CTD
{

//! Implement and register the camera entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( CameraEntityFactory );

EnCamera::EnCamera() :
_fov( 60.00 ),
_nearClip( 0.1f ),
_farClip( 1000.0f ),
_backgroudColor( Vec3f( 0.2f, 0.2f, 0.2f ) ),
_needUpdate ( false )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "position"          , _position         );
    _attributeManager.addAttribute( "rotation"          , _rotation         );
    _attributeManager.addAttribute( "fov"               , _fov              );
    _attributeManager.addAttribute( "nearClip"          , _nearClip         );
    _attributeManager.addAttribute( "farClip"           , _farClip          );
    _attributeManager.addAttribute( "backgroundColor"   , _backgroudColor   );
}

EnCamera::~EnCamera()
{
}

void EnCamera::initialize()
{
    unsigned int width, height;
    Application::get()->getScreenSize( width, height );

    // setup camera
    _p_cam = Application::get()->getViewer()->getCamera( 0 );
    _p_cam->setLensPerspective( _fov, _fov * ( float( height ) / float( width ) ), _farClip, _nearClip );
    _p_cam->setClearColor( _backgroudColor.x(), _backgroudColor.y(), _backgroudColor.z(), 1.0 );

    _curPosition = _position;
    _curRotation = osg::Vec3f( 
                                osg::DegreesToRadians( _rotation.x() ),
                                osg::DegreesToRadians( _rotation.y() ),
                                osg::DegreesToRadians( _rotation.z() )
                              );

    setCameraTranslation( _curPosition );
    setCameraRotation( _curRotation );
}

void EnCamera::updateEntity( float deltaTime )
{
    // update camera matrix if needed
    if ( _needUpdate )
    {
        _p_cam->setViewByMatrix( _camMatrix );
        _needUpdate = false;
    }
}

} // namespace CTD
