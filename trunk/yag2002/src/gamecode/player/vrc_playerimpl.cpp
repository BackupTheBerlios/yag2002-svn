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

#include <vrc_main.h>
#include "vrc_playerimpl.h"
#include "vrc_playerphysics.h"
#include "vrc_playeranim.h"
#include "vrc_playersound.h"
#include "vrc_inputhandler.h"
#include "vrc_playernetworking.h"
#include "chat/vrc_chatmgr.h"
#include "chat/VRC/vrc_chatprotVRC.h"
#include "chat/IRC/vrc_chatprotIRC.h"
#include "../visuals/vrc_camera.h"

namespace vrc
{

ChatManager* BasePlayerImplementation::s_chatMgr = NULL;

BasePlayerImplementation::BasePlayerImplementation( EnPlayer* player ) :
_cameraMode( Ego ),
_p_player( player ),
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_p_playerSound( NULL ),
_p_camera( NULL ),
_p_playerNetworking( NULL ),
_moveDir( osg::Vec3f( 0, 1.0f, 0 ) ),
_rotZ( 0 )
{
    // copy player's attributes
    _playerAttributes = _p_player->getPlayerAttributes();
    _rotZ = _p_player->getPlayerAttributes()._rot;
}

BasePlayerImplementation::~BasePlayerImplementation()
{
}

// get the shared chat manager
ChatManager* BasePlayerImplementation::getChatManager()
{
    return s_chatMgr;
}

void BasePlayerImplementation::destroyChatManager()
{
    if ( s_chatMgr )
    {
        // delete all chat protocol implementations
        ChatManager::ProtocolMap& prots = s_chatMgr->getRegisteredProtocols();
        ChatManager::ProtocolMap::iterator p_beg = prots.begin(), p_end = prots.end();
        for( ; p_beg != p_end; ++p_beg )
            delete p_beg->second;

        // delete chat manager
        delete s_chatMgr;
        s_chatMgr = NULL;
    }
}

bool BasePlayerImplementation::createChatManager()
{
    assert( ( s_chatMgr == NULL ) && "chat manager already created!" );
    s_chatMgr = new ChatManager;

    // register the IRC protocol
    ChatNetworkingIRC* p_protIRC = new ChatNetworkingIRC;
    s_chatMgr->registerChatProtocol( IRC_PROTOCOL_NAME, p_protIRC );

    // create VRC protocol only for client and server mode, not for standalone!
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Standalone )
    {
        ChatNetworkingVRC* p_protVRC = new ChatNetworkingVRC;
        s_chatMgr->registerChatProtocol( VRC_PROTOCOL_NAME, p_protVRC );        
    }

    // build the chat system
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        if ( !s_chatMgr->buildServer() )
            return false;
    }
    else
    {
        if ( !s_chatMgr->buildClient() )
            return false;
    }

    return true;
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
        }
        break;

        default:
            assert( NULL && "requesting for an invalid camera mode" );

    }
    _cameraMode = static_cast< CameraMode >( mode );
}

const std::string& BasePlayerImplementation::getPlayerName() const
{
    return _playerName;
}

void BasePlayerImplementation::setPlayerName( const std::string& name )
{
    // if networking exists then the player name change must be done via networking ( distributing the changed name )
    if ( _p_playerNetworking )
    {
        _p_playerNetworking->updatePlayerName( name );
        _playerName = name;
    }
    else
    {
        _playerName = name;
    }
}

void BasePlayerImplementation::changePlayerName( const std::string& name )
{
    // this method is called only by networking component in order to notify a changed player name ( remote and local players )
    _playerName = name;
}

} // namespace vrc
