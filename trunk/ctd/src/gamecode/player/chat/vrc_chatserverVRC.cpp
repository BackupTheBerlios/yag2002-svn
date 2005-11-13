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
 # entity Platform
 #
 #   date of creation:  02/18/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_chatserverVRC.h"
#include "vrc_chatmgr.h"
#include "VRC/vrc_chatprotVRC.h"

namespace vrc
{

//! Implement and register the VRC chat server entity factory
YAF3D_IMPL_ENTITYFACTORY( ChatServerVRCEntityFactory );


EnChatServerVRC::EnChatServerVRC():
_logFile( "chatsrvVRC.yaf3d::log" ),
_p_chatMgr( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "logFile", _logFile );
}

EnChatServerVRC::~EnChatServerVRC()
{
}

void EnChatServerVRC::initialize()
{
    assert( ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server ) && "this entity must only be used in server mode!" );
    _p_chatMgr = new ChatManager;

    // create VRC protocol
    ChatNetworkingVRC* p_protVRC = new ChatNetworkingVRC;
    _p_chatMgr->registerChatProtocol( "VRC", p_protVRC );        

    // build the chat system
    _p_chatMgr->buildServer();
    ChatNetworkingVRC* p_inst = p_protVRC->createInstance();
    p_inst->registerProtocolCallback( this, "*" );


    // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerUpdate( this );   
}

void EnChatServerVRC::onConnection( const ChatConnectionConfig& config )
{
}

void EnChatServerVRC::onJoinedChannel( const ChatConnectionConfig& config )
{
}

void EnChatServerVRC::updateEntity( float deltaTime )
{
}

}
