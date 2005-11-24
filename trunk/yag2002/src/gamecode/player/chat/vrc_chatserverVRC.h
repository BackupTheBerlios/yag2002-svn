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
 # entity VRC chat server
 #
 #   date of creation:  10/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CHARSEVERVRC_H_
#define _VRC_CHARSEVERVRC_H_

#include <vrc_main.h>
#include "vrc_chatprotocol.h"

namespace vrc
{

#define ENTITY_NAME_CHATSRV_VRC    "ChatServerVRC"

class ChatManager;

//! VRC chat server
class EnChatServerVRC : public yaf3d::BaseEntity, public ChatProtocolCallback
{
    public:
                                                    EnChatServerVRC();

        virtual                                     ~EnChatServerVRC();

        //! Initialize 
        void                                        initialize();

        //! Overriden protocol handler callback method for getting notification when joined to a new channel
        void                                        onJoinedChannel( const ChatConnectionConfig& config );

        //! Overridden method for getting notified when someone left the channel.
        void                                        onLeftChannel( const ChatConnectionConfig& config );

        //! Overriden method for receiving the chat traffic.
        void                                        onReceive( const std::string& channel, const std::string& sender, const std::string& msg );

        //! Overriden method for getting nickname changes in channel.
        void                                        onNicknameChanged( const std::string& newname, const std::string& oldname );

        //! Overridden method for getting notified when someone has been kicked from a channel.
        void                                        onKicked( const std::string& channel, const std::string& kicker, const std::string& kicked );

    protected:

        std::string                                 _logFile;

        yaf3d::Log                                  _chatLog;

        ChatManager*                                _p_chatMgr;
};

//! Entity type definition used for type registry
class ChatServerVRCEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    ChatServerVRCEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_CHATSRV_VRC, yaf3d::BaseEntityFactory::Server )
                                                    {}

        virtual                                     ~ChatServerVRCEntityFactory() {}

        Macro_CreateEntity( EnChatServerVRC );
};

}

#endif // _VRC_CHARSEVERVRC_H_
