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
YAF3D_IMPL_ENTITYFACTORY( ChatServerVRCEntityFactory )


EnChatServerVRC::EnChatServerVRC():
_logFile( "chatsrvVRC" ),
_p_chatMgr( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "logFile", _logFile );
}

EnChatServerVRC::~EnChatServerVRC()
{
    _chatLog << yaf3d::getTimeStamp() << "shutting down chat server" << std::endl;
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
    BaseChatProtocol* p_inst = p_protVRC->createInstance();
    p_inst->registerProtocolCallback( this, "*" );

    // create a timestamp for log file name    
    std::string tmp( yaf3d::getFormatedDate() );
    std::string datestamp;
    for ( std::size_t cnt = 0; cnt < tmp.length(); ++cnt )
    {
        if ( ( tmp[ cnt ] != '\\' ) && ( tmp[ cnt ] != ':' ) && ( tmp[ cnt ] != '/' ) )
            datestamp += tmp[ cnt ];
    }

    // setup chat log
    _chatLog.addSink( "chatlog", yaf3d::Application::get()->getMediaPath() + datestamp + "-" + _logFile + ".log", yaf3d::Log::L_ERROR );
    _chatLog.enableSeverityLevelPrinting( false );
    _chatLog.enableSeverityLevelPrinting( false );
    _chatLog << yaf3d::Log::LogLevel( yaf3d::Log::L_INFO );
    _chatLog << "log file created on " << yaf3d::getTimeStamp() << std::endl;
    _chatLog << "-----------" << std::endl;
}

void EnChatServerVRC::onJoinedChannel( const ChatConnectionConfig& config )
{
    _chatLog << yaf3d::getFormatedDateAndTime() << " " << config._nickname << " joined to VRC chat" << std::endl;
}

void EnChatServerVRC::onLeftChannel( const ChatConnectionConfig& config )
{
    _chatLog << yaf3d::getFormatedDateAndTime() << " " << config._nickname << " left chat channel" << std::endl;
}

void EnChatServerVRC::onReceive( const std::string& channel, const std::string& sender, const std::string& msg )
{
    _chatLog << "[" + yaf3d::getFormatedTime() + "] " + sender << ": " << msg << std::endl;
}

void EnChatServerVRC::onNicknameChanged( const std::string& newname, const std::string& oldname )
{
    _chatLog << yaf3d::getFormatedDateAndTime() << " " <<  oldname << " changed nickname to " << newname << std::endl;
}

void EnChatServerVRC::onKicked( const std::string& channel, const std::string& kicker, const std::string& kicked )
{
    _chatLog << yaf3d::getFormatedDateAndTime() << " " << kicker << " kicked " << kicked << " from channel " << std::endl;
}

} // namespace vrc
