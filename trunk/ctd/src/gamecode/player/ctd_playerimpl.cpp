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
 # base clas for player implementations
 #
 #  the actual player behaviour is described in one of its implemnets
 #  depending on game mode ( server, client, or standalone )
 #
 #
 #   date of creation:  05/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playerimpl.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_inputhandler.h"
#include "ctd_chatgui.h"
#include "ctd_playernetworking.h"
#include "../visuals/ctd_camera.h"

using namespace osg;
using namespace std;

namespace CTD
{

BasePlayerImplementation::BasePlayerImplementation( EnPlayer* player ) :
_p_player( player ),
_p_playerSound( NULL ),
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_p_playerNetworking( NULL ),
_p_camera( NULL ),
_cameraMode( Spheric ),
_moveDir( Vec3f( 0, 1, 0 ) ),
_rotZ( 0 ),
_p_chatGui( NULL )
{
    // copy player's attributes
    _playerAttributes = _p_player->getPlayerAttributes();
    _rotZ = _p_player->getPlayerAttributes()._rot;
}

BasePlayerImplementation::~BasePlayerImplementation()
{
}

void BasePlayerImplementation::setNextCameraMode()
{
    if ( _cameraMode == Spheric )
        setCameraMode( Ego );
    else
        setCameraMode( Spheric );
}

void BasePlayerImplementation::setCameraPitchYaw( float pitch, float yaw )
{
    if ( _cameraMode == Spheric )
    {
        _p_camera->setLocalPitchYaw( pitch, yaw );
    }
    // in ego mode we turn the character instead of yawing the camera! turning is done in inputhandler
    else
    {
        _p_camera->setLocalPitch( pitch );
    }
}

void BasePlayerImplementation::setCameraMode( unsigned int mode )
{
    switch ( mode )
    {
        case Spheric:
        {
            _p_camera->setCameraOffsetPosition( _playerAttributes._camPosOffsetSpheric );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _playerAttributes._camRotOffsetSpheric.x()  ), osg::Vec3f( 0.0f, 1.0f, 0.0f ), // roll
                osg::DegreesToRadians( _playerAttributes._camRotOffsetSpheric.y()  ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                osg::DegreesToRadians( _playerAttributes._camRotOffsetSpheric.z()  ), osg::Vec3f( 0.0f, 0.0f, 1.0f )  // yaw
                );
            _p_camera->setCameraOffsetRotation( rot );
            _p_playerAnimation->enableRendering( true );
            GuiManager::get()->showMousePointer( false );
        } 
        break;

        case Ego:
        {
            _p_camera->setCameraOffsetPosition( _playerAttributes._camPosOffsetEgo );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _playerAttributes._camRotOffsetEgo.x()  ), osg::Vec3f( 0.0f, 1.0f, 0.0f ), // roll
                osg::DegreesToRadians( _playerAttributes._camRotOffsetEgo.y()  ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                osg::DegreesToRadians( _playerAttributes._camRotOffsetEgo.z()  ), osg::Vec3f( 0.0f, 0.0f, 1.0f )  // yaw
                );
            _p_camera->setCameraOffsetRotation( rot );
            _p_playerAnimation->enableRendering( false );
            GuiManager::get()->showMousePointer( true );
        }
        break;

        default:
            assert( NULL && "requesting for an invalid camera mode" );

    }
    _cameraMode = ( CameraMode )mode;
}

void BasePlayerImplementation::addChatMessage( const CEGUI::String& msg, const CEGUI::String& author )
{
    // this method is used by all clients (local and remote), hence the chat gui must be accessible to all clients
    PlayerChatGui* p_gui = PlayerChatGui::get();
    if ( p_gui ) // on exiting a level the gui may be destroyed at this point, catch this case here
        PlayerChatGui::get()->addMessage( msg, author );
}

void BasePlayerImplementation::distributeChatMessage( const std::string& msg )
{
    _p_playerNetworking->putChatText( msg );
}

} // namespace CTD
