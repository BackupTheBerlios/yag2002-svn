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
 # VRC built-in chat protocol
 #
 #   date of creation:  10/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/


#ifndef _VRC_CHATPROTVRC_H_
#define _VRC_CHATPROTVRC_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_PlayerChat.h"
#include "../vrc_chatprotocol.h"

class ChatNetworkingVRC;

//!  Networking implementation for VRC protocol
class ImplChatNetworkingVRC : _RO_DO_PUBLIC_RO( ImplChatNetworkingVRC ), public yaf3d::SessionNotifyCallback
{

    public:

        explicit                                    ImplChatNetworkingVRC( ChatNetworkingVRC* p_nw = NULL );

        virtual                                     ~ImplChatNetworkingVRC();

        //! Post new chat text. If 'recipient' is not empty then the message will be private ( whisper )
        void                                        postChatText( const CEGUI::String& text, const std::string& recipient );

        //! Leave chat
        void                                        leave();

        //! Get list of chat members in given channel.
        void                                        getMemberList( const std::string& channel, std::vector< std::string >& list );

        // ReplicaNet overrides
        //-----------------------------------------------------------------------------------//

        //! Object can now be initialized in scene ( on clients )
        void                                        PostObjectCreate();

        // vrc networking interface overrides
        //-----------------------------------------------------------------------------------//

        //! Overridden method for getting notification when a client joins to the network
        void                                        onSessionJoined( int sessionID );

        //! Overridden method for getting notification when a client leaves the network
        void                                        onSessionLeft( int sessionID );

        // RPCs
        //-----------------------------------------------------------------------------------//

        //! Request for joining to channel
        void                                        RPC_RequestJoin( tChatData chatdata );

        //! Notifying all clients that a new chat client joined
        void                                        RPC_ClientJoined( tChatData chatdata );

        //! Notify the server that a client is leaving
        void                                        RPC_RequestLeave( tChatData chatdata );

        //! Notify all clients that a client has left
        void                                        RPC_ClientLeft( tChatData chatdata );

        //! Called when server grants join request
        void                                        RPC_InitializeClient( tChatData chatdata );

        //! Request the server for changing nickname
        void                                        RPC_RequestChangeNickname( tChatData chatdata );

        //! Some nickname has been changed
        void                                        RPC_ChangedNickname( tChatData chatdata );

        //! Requests the server for transmitting the chat member list
        void                                        RPC_RequestMemberList( tChatData chatdata );

        //! Client gets chat member list ( one RPC per member )
        void                                        RPC_RecvMemberList( tChatData chatdata );

        //! Post chat text
        void                                        RPC_PostChatText( tChatMsg chatMsg );

        //! New incomming chat text
        void                                        RPC_RecvChatText( tChatMsg chatMsg );

    protected:

        //! Create a unique nickname out of requested one by adding an appropriate postfix number
        std::string                                 makeUniqueNickname( const std::string& reqnick );

        //! Protocol handler instance
        ChatNetworkingVRC*                          _p_protVRC;

        //! Session ID of chat server object
        int                                         _serverSID;

        //! Session ID of replicated server object ( ghost )
        int                                         _clientSID;

        //! Container for storing various things
        tChatData                                   _config;

        //! A map of existing < session ids / nicknames >, it is also used for creating unique nicks on server
        std::map< int, std::string >                _nickNames;

    friend class _MAKE_RO( ImplChatNetworkingVRC );
};

//!  Handler for VRC protocol
class ChatNetworkingVRC : public vrc::BaseChatProtocol
{

    public:

                                                    ChatNetworkingVRC();

        virtual                                     ~ChatNetworkingVRC();

        //! This method is used for creating an instance of an registered protocol handler.
        vrc::BaseChatProtocol*                      createInstance();

        //! Create a new connection to a chat server
        void                                        createConnection( const vrc::ChatConnectionConfig& conf ) throw ( vrc::ChatExpection );

        //! This method is called uppon establishing the connection
        void                                        connected();

        //! This method when the client is disconnected ( e.g. because of networking errors )
        void                                        disconnected();

        //! Request for getting list of chat members in given channel.
        void                                        requestMemberList( const std::string& channel );

        //! Get list of chat members in given channel. Call this method after callback method onReceiveMemberList has been called by protocol handler.
        void                                        getMemberList( const std::string& channel, std::vector< std::string >& list );

        //! This method is called uppon joining to a channel
        void                                        joined( const std::string& channel, const std::string& name );

        //! This method is called when a client ( chat memeber ) has left the chat 
        void                                        left( const std::string& channel, const std::string& name );

        //! Destroy connection
        void                                        destroyConnection();

        //! Send a chat message to a channel
        void                                        send( const CEGUI::String& msg, const std::string& channel, const std::string& recipient );

        //! Set networking implementation instance, is needed for creating new protocol handler instances
        void                                        setNetworkingImpl( ImplChatNetworkingVRC* p_nwImpl )
                                                    {
                                                        _p_nwImpl = p_nwImpl;
                                                    }
        //! Get networking implementation instance
        ImplChatNetworkingVRC*                      getNetworkingImpl()
                                                    {
                                                        return _p_nwImpl;
                                                    }

        //! Method for distributing new incoming message to all registered callbacks.
        //! In normal case only internal functions make use of this method.
        void                                        recvMessage( const std::string& channel, const std::string& sender, const CEGUI::String& msg );

        //! Method for distributing a nickname change in channel to all registered callbacks.
        //! In normal case only internal functions make use of this method.
        void                                        recvNicknameChange( const std::string& newname, const std::string& oldname );

        //! Method for distributing member list changes in channel to all registered callbacks.
        void                                        recvMemberList( const std::string& channel );

    protected:

        ImplChatNetworkingVRC*                      _p_nwImpl;

        vrc::ChatConnectionConfig*                  _p_config;
};

#endif // _VRC_CHATPROTVRC_H_
