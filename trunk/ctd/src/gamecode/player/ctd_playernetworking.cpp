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
#include "ctd_playerphysics.h"
#include "ctd_player.h"

using namespace std;

CTD::Log* PlayerNetworking::s_chatLog = NULL;

PlayerNetworking::PlayerNetworking( CTD::BasePlayerImplementation* p_playerImpl ) :
_p_playerImpl( p_playerImpl ),
_loadedPlayerEntity( NULL ),
_remoteClient( false ),
_cmdAnimFlags( 0 ),
_positionX( 0 ),
_positionY( 0 ),
_positionZ( 0 ),
_yaw( 0 )
{   
    // this constructor can be called either by player entity or networking system (in client or server mode)
    //  when called by player entity then it means that we are a local client, otherwise we are a remote client
    if ( !p_playerImpl )
    {
        _remoteClient = true;
        _p_playerName[ 0 ] = 0;
    }
    else
    {
        strcpy( _p_playerName, _p_playerImpl->getPlayerEntity()->getPlayerName().c_str() );
    }
    _p_configFile[ 0 ] = 0;
    _cmdAnimFlags      = 0;

    // setup chat log
    if ( !s_chatLog )
    {
        std::string filename;
        if ( CTD::GameState::get()->getMode() == CTD::GameState::Server )
            filename = "server-chat.log";
        else
            filename = "client-chat.log";

        s_chatLog = new CTD::Log();
        s_chatLog->addSink( "chatlog", CTD::Application::get()->getMediaPath() + filename, CTD::Log::L_ERROR );
        s_chatLog->enableSeverityLevelPrinting( false );
        *s_chatLog << CTD::Log::LogLevel( CTD::Log::L_INFO );
        *s_chatLog << "log file created on " << CTD::getTimeStamp() << std::endl;
        *s_chatLog << "-----------" << std::endl;
    }
}

PlayerNetworking::~PlayerNetworking()
{
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "player left: " << _p_playerName << endl;

    // remove ghost from simulation ( server and client )
    if ( isRemoteClient() ) 
    {    
        // print a goodby message before destroying the player entity
        if ( CTD::GameState::get()->getMode() != CTD::GameState::Server )
        {
            string exitingtext( string( "< " ) + _p_playerName + string ( " says goodbye >" ) );
            _p_playerImpl->addChatMessage( exitingtext, "" );
        }

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

    *s_chatLog << CTD::getTimeStamp() << ": [" << _p_playerName << "] left the chat room " << std::endl;
}

void PlayerNetworking::PostObjectCreate()
{
    // complete setting up ghost ( remote client ) or server-side player
    if ( isRemoteClient() ) 
    {
        // create the player and its related entities
        createPlayer();

        // setup new connected client
        if ( CTD::GameState::get()->getMode() == CTD::GameState::Server )
        {
            // send intialization data
            static tInitializationData init;
            osg::Vec3f pos = _loadedPlayerEntity->getPlayerImplementation()->getPlayerPosition();
            _positionX = pos.x();
            _positionY = pos.y();
            _positionZ = pos.z();
            osg::Quat  rot = _loadedPlayerEntity->getPlayerImplementation()->getPlayerRotation();
            double angle;
            rot.getRotate( angle, osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
            _yaw = angle;
            init._posX = _positionX;
            init._posY = _positionY;
            init._posZ = _positionZ;
            init._rotZ = _yaw;

            ALL_REPLICAS_FUNCTION_CALL( RPC_Initialize( init ) );
        }
        else
        {
            string enteringtext( string( "< " ) + _p_playerName + string ( " says hello >" ) );
            _p_playerImpl->addChatMessage( enteringtext, "" );   
        }
    }

    *s_chatLog << CTD::getTimeStamp() << ": [" << _p_playerName << "] entered the chat room " << std::endl;
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << " player created: " << _p_playerName << endl;
}

void PlayerNetworking::createPlayer()
{
    std::string playerconfig = _p_configFile;
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_INFO ) << "player '" <<  _p_playerName << "' connected" << endl;

    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "loading player configuration file: " << playerconfig << endl;            
    std::stringstream postfix;
    static unsigned int postcnt = 0;
    postfix << "_" << postcnt;
    postcnt++;
    // load player related entities
    if ( !CTD::LevelManager::get()->loadEntities( playerconfig, &_loadedEntities, postfix.str() ) )
    {
        CTD::log << CTD::Log::LogLevel( CTD::Log::L_ERROR ) << "cannot find player configuration file: " << playerconfig << endl;            
        return;
    }

    // search for player entity and set it up
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
        if ( CTD::GameState::get()->getMode() == CTD::GameState::Server )
            _p_playerImpl = new CTD::PlayerImplServer( _loadedPlayerEntity );
        else
            _p_playerImpl = new CTD::PlayerImplClient( _loadedPlayerEntity );

        _p_playerImpl->setPlayerNetworking( this );
        _p_playerImpl->initialize();
        _loadedPlayerEntity->setPlayerImplementation( _p_playerImpl );
        // set loading prefix
        _p_playerImpl->setLoadingPostfix( postfix.str() );
    }

    // begin initialization of player and its components
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "initializing new player instance ... " << endl;
    {
        p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
        for ( ; p_beg != p_end; p_beg++ )
        {
            ( *p_beg )->initialize();
        }
    }

    // now begin post-initialization of player and its components
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "post-initializing new player ..." << endl;
    {
        p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
        for ( ; p_beg != p_end; p_beg++ )
        {
            ( *p_beg )->postInitialize();
        }
    }
    _loadedPlayerEntity->setPlayerName( _p_playerName );
}

void PlayerNetworking::initialize( const osg::Vec3f& pos, const string& playerName, const string& cfgFile )
{
    _positionX = pos._v[ 0 ]; 
    _positionY = pos._v[ 1 ];
    _positionZ = pos._v[ 2 ];
    strcpy( _p_playerName, playerName.c_str() );
    strcpy( _p_configFile, cfgFile.c_str() );
}

void PlayerNetworking::putChatText( const CEGUI::String& text )
{
    static tChatMsg s_textBuffer;
    text.copy( s_textBuffer._text );
    ALL_REPLICAS_FUNCTION_CALL( RPC_AddChatText( s_textBuffer ) );
}

void PlayerNetworking::RPC_AddChatText( tChatMsg chatMsg )
{
    chatMsg._text[ 255 ] = 0; // limit text length
    // server directs all messages into a log file!
    if ( CTD::GameState::get()->getMode() == CTD::GameState::Server ) 
        getChatLog() << chatMsg._text << endl;
    else
    {
        CEGUI::String msg( chatMsg._text );
        _p_playerImpl->addChatMessage( msg, _p_playerName );
    }
}

void PlayerNetworking::RPC_Initialize( tInitializationData initData )
{
    // server directs all messages into a log file!
    assert( ( CTD::GameState::get()->getMode() == CTD::GameState::Client ) && "this RPC must be called only for clients!" );

    // init player position set by server ( it's the job of server to init the player position and rotation )
    _p_playerImpl->setPlayerPosition( osg::Vec3f( initData._posX, initData._posY, initData._posZ ) );
    _p_playerImpl->setPlayerRotation( osg::Quat( initData._rotZ, osg::Vec3f( 0.0f, 0.0f, 1.0f ) ) );
    
    // reset physics body transformation
    osg::Matrixf mat;
    mat *= mat.rotate( _p_playerImpl->getPlayerRotation() );
    mat.setTrans( _p_playerImpl->getPlayerPosition() ); 
    _p_playerImpl->getPlayerPhysics()->setTransformation( mat );
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
