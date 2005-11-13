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

#ifndef _CTD_CHARSEVERVRC_H_
#define _CTD_CHARSEVERVRC_H_

#include <ctd_main.h>
#include "ctd_chatprotocol.h"

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

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Overriden protocol handler callback method for getting connection notification
        void                                        onConnection( const ChatConnectionConfig& config );

        //! Overriden protocol handler callback method for getting notification when joined to a new channel
        void                                        onJoinedChannel( const ChatConnectionConfig& config );

    protected:

        std::string                                 _logFile;

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

#endif // _CTD_CHARSEVERVRC_H_
