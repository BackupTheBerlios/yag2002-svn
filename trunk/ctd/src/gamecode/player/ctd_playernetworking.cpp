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
 # networking for chat member
 #
 # this class implements the networking functionality for chat member
 #
 #
 #   date of creation:  12/25/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playernetworking.h"
#include "ctd_playerimpl.h"
#include "ctd_playerimplClient.h"
#include "ctd_playerimplServer.h"
#include "ctd_player.h"

using namespace std;

CTD::Log* PlayerNetworking::s_chatLog = NULL;

PlayerNetworking::PlayerNetworking( CTD::BasePlayerImplementation* p_playerImpl ) :
_loadedPlayerEntity( NULL )
{   
    _p_playerImpl = p_playerImpl;

    // setup chat log
    if ( !s_chatLog )
    {
        s_chatLog = new CTD::Log;
        s_chatLog->addSink( "chatlog", CTD::Application::get()->getMediaPath() + "chat.log", CTD::Log::L_ERROR );
        s_chatLog->enableSeverityLevelPrinting( false );
        *s_chatLog << CTD::Log::LogLevel( CTD::Log::L_INFO );
    }

    // this constructor can be called either by player entity or networking system (in client or server mode)
    //  when called by player entity then it means that we are a local client, otherwise we are a remote client
    _remoteClient = false;
    if ( !p_playerImpl )
    {
        _remoteClient = true;

        CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "creating a new player instance ... " << endl;      

        // TODO: get the character config file over net!
        std::string playerconfig;
        if ( CTD::GameState::get()->getMode() == CTD::GameState::Server )
            playerconfig = "player/banana.server";
        else if ( CTD::GameState::get()->getMode() == CTD::GameState::Client )
            playerconfig = "player/banana.client";
        else assert( NULL && "something weird is happening here! no mode except server or client is allowed to use networking." );

        CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "loading player configuration file: " << playerconfig << endl;            
        std::stringstream postfix;
        static unsigned int postcnt = 0;
        postfix << "_" << postcnt;
        postcnt++;
        // we force creation of all entity types on server
        if ( !CTD::LevelManager::get()->loadEntities( playerconfig, &_loadedEntities, postfix.str() ) )
        {
            CTD::log << CTD::Log::LogLevel( CTD::Log::L_ERROR ) << "cannot find player configuration file: " << playerconfig << endl;            
            return;
        }

        // search for player entity
        std::vector< CTD::BaseEntity* >::iterator p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
        {
            for ( ; p_beg != p_end; p_beg++ )
            {
                if ( ( *p_beg )->getTypeName() == ENTITY_NAME_PLAYER )
                    break;
            }
            if ( p_beg == p_end )
            {
                CTD::log << CTD::Log::LogLevel( CTD::Log::L_ERROR ) << "cannot find player entity in file: " << playerconfig << endl;            
                return;
            }
            _loadedPlayerEntity = static_cast< CTD::EnPlayer* >( *p_beg );
            // for a remote client we must setup the player implementation before initializing
            _p_playerImpl = new CTD::PlayerImplClient( _loadedPlayerEntity );
            _p_playerImpl->setPlayerNetworking( this );
            _p_playerImpl->initialize();
            _loadedPlayerEntity->setPlayerImplementation( _p_playerImpl );
            // set loading prefix
            _p_playerImpl->setLoadingPostfix( postfix.str() );
        }

        // begin initialization of player and its components
        CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "initializing new player instance ... " << endl;
        {
            p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
            for ( ; p_beg != p_end; p_beg++ )
            {
                ( *p_beg )->initialize();
            }
        }

        _p_playerName[ 0 ]   = 0;
        _p_animFileName[ 0 ] = 0;
    }
    strcpy( _p_playerName, _p_playerImpl->getPlayerEntity()->getPlayerName().c_str() );
    _cmdAnimFlags     = 0;
}

PlayerNetworking::~PlayerNetworking()
{
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "player left: " << _p_playerName << endl;

    // remove ghost from simulation ( server and client )
    if ( isRemoteClient() ) 
    {    
        // PlayerNetworking has created the player implementation, so set its networking and other components to NULL in order to abvoid deleting it also by player's implementation
        _p_playerImpl->setPlayerNetworking( NULL );
        _p_playerImpl->setPlayerSound( NULL );
        _p_playerImpl->setPlayerAnimation( NULL );
        _p_playerImpl->setPlayerPhysics( NULL );

        // remove all associated entities
        std::vector< CTD::BaseEntity* >::iterator p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
        for ( ; p_beg != p_end; p_beg++ )
            CTD::EntityManager::get()->deleteEntity( *p_beg );
    }
    else
    {
        string enteringtext( string( "< " ) + _p_playerName + string ( " says goodbye >" ) );
        putChatText( enteringtext );
    }
}

void PlayerNetworking::PostObjectCreate()
{
    // complete setting up ghost ( remote client ) or server-side player
    if ( isRemoteClient() ) 
    {
        // now begin post-initialization of player and its components
        CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "post-initializing new player ..." << endl;
        {
            std::vector< CTD::BaseEntity* >::iterator p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
            for ( ; p_beg != p_end; p_beg++ )
            {
                ( *p_beg )->postInitialize();
            }
        }
        _loadedPlayerEntity->setPlayerName( _p_playerName );
    }
    else
    {
        string enteringtext( string( "< " ) + _p_playerName + string ( " says hello >" ) );
        putChatText( enteringtext );
    }

    CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << " player created: " << _p_playerName << endl;
}

void PlayerNetworking::initialize( const osg::Vec3f& pos, const string& playerName, const string& meshFileName )
{
    _positionX = pos._v[ 0 ]; 
    _positionY = pos._v[ 1 ];
    _positionZ = pos._v[ 2 ];
    strcpy( _p_playerName, playerName.c_str() );
    strcpy( _p_animFileName, meshFileName.c_str() );
}

void PlayerNetworking::putChatText( const string& text )
{
    static tChatMsg s_textBuffer;
    strcpy( s_textBuffer._text, text.c_str() );
    ALL_REPLICAS_FUNCTION_CALL( RPC_AddChatText( s_textBuffer ) );
}

void PlayerNetworking::RPC_AddChatText( tChatMsg chatMsg )
{
    chatMsg._text[ 255 ] = 0; // limit text length
    // server directs all messages into a log file!
    if ( CTD::GameState::get()->getMode() == CTD::GameState::Server ) 
        getChatLog() << chatMsg._text << endl;
    else
        _p_playerImpl->addChatMessage( chatMsg._text, _p_playerName );
}

void PlayerNetworking::update()
{
}

void PlayerNetworking::updateAnimationFlags( unsigned int cmdFlag )
{
    _cmdAnimFlags = cmdFlag;
}

void PlayerNetworking::getAnimationFlags( unsigned int& cmdFlag )
{
    cmdFlag = _cmdAnimFlags;
}

void PlayerNetworking::updatePosition( float x, float y, float z )
{
   _positionX = x;
   _positionY = y;
   _positionZ = z;
}

void PlayerNetworking::getPosition( float& x, float& y, float& z )
{
    x = _positionX;
    y = _positionY;
    z = _positionZ;
}

void PlayerNetworking::updateRotation( float yaw )
{
    _yaw = yaw;
}

void PlayerNetworking::getRotation( float& yaw )
{
    yaw =_yaw;
}
