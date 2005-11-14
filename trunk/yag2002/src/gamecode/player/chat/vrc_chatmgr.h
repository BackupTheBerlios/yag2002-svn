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
 # chat manager
 #
 #   date of creation:  10/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/


#ifndef _VRC_CHATMGR_H_
#define _VRC_CHATMGR_H_

#include "vrc_chatprotocol.h"

namespace vrc
{

//! Chat protocol names
#define VRC_PROTOCOL_NAME "VRC"
#define IRC_PROTOCOL_NAME "IRC"

class BaseChatProtocol;
class BasePlayerImplementation;

//! Class describing a chat connection related settings
class ChatConnectionConfig
{
    public:
                                                    ChatConnectionConfig() :
                                                     _port( 0 ),
                                                     _p_protocolHandler( NULL )
                                                    {
                                                    }

        virtual                                     ~ChatConnectionConfig() {}

        //! Reset the configuration
        void                                        reset()
                                                    {
                                                        _protocol  = "";
                                                        _serverURL = "";
                                                        _channel   = "";
                                                        _nickname  = "";
                                                        _username  = "";
                                                        _realname  = "";
                                                        _password  = "";
                                                        _port      = 0;
                                                    }

                                                    ChatConnectionConfig( const ChatConnectionConfig& cfg )
                                                    {
                                                        _protocol           = cfg._protocol;
                                                        _serverURL          = cfg._serverURL;
                                                        _channel            = cfg._channel;
                                                        _nickname           = cfg._nickname;
                                                        _username           = cfg._username;
                                                        _realname           = cfg._realname;
                                                        _password           = cfg._password;
                                                        _port               = cfg._port;
                                                        _p_protocolHandler  = cfg._p_protocolHandler;
                                                    }

        //! Equality operator
        bool                                        operator == ( const ChatConnectionConfig& right )
                                                    {
                                                        if (
                                                            ( _protocol == right._protocol ) &&
                                                            ( _channel == right._channel ) &&
                                                            ( _serverURL == right._serverURL )
                                                            )
                                                            return true;

                                                        return false;
                                                    }

        //! Chat protocol ( one of the availale protocols such as "VRC" or "IRC" )
        std::string                                 _protocol;

        //! Server URL ( VRC protocol does not need this )
        std::string                                 _serverURL;

        //! Channel on server ( VRC protocol does not need this )
        std::string                                 _channel;

        //! Nickname
        std::string                                 _nickname;

        //! Optional user name
        std::string                                 _username;

        //! Optional real name
        std::string                                 _realname;

        //! Optional password
        std::string                                 _password;

        //! Port ( is not needed for "VRC" protocol )
        unsigned int                                _port;

        //! Protocol handler
        BaseChatProtocol*                           _p_protocolHandler;
};

//! Class for chat related exceptions
class ChatExpection
{
    public:                                     
                                                    ChatExpection( const std::string& reason )
                                                    {
                                                        _reason = reason;
                                                    }

                                                    ~ChatExpection() {}

                                                    ChatExpection( const ChatExpection& e )
                                                    {
                                                        _reason = e._reason;
                                                    }

        const std::string&                          what() const
                                                    {
                                                        return _reason;
                                                    }

    protected:

                                                    ChatExpection();

        ChatExpection&                              operator = ( const ChatExpection& );

        std::string                                 _reason;
};

// Class for managing the chat functionality
class ChatGuiCtrl;
class ChatGuiBox;

class ChatManager : public ChatProtocolCallback
{
    public:

        //! Type for mapping < protocol name / protocol implementation >
        typedef std::map< std::string, BaseChatProtocol* >  ProtocolMap;

                                                    ChatManager();

        virtual                                     ~ChatManager();


        //! Activate / deactivate interaction with chat gui box
        void                                        activateBox( bool en );

        //! Enable / disable entire chat gui rendering
        void                                        show( bool en );

        //! Register a chat protocol, e.g. IRC or VRC. Use this method for all your protocols before "build"ing.
        //! Note: Registered protocol instances must be deleted by user(s) of this manager.
        bool                                        registerChatProtocol( const std::string& prot, BaseChatProtocol* p_prot );

        //! Build the chat system for client or standalone mode
        bool                                        buildClient();

        //! Build the chat system for server mode
        bool                                        buildServer();

        //! Create a chat setting up a connection ( and joining to a channel if a channel is given )
        void                                        createConnection( const ChatConnectionConfig& config );

        //! Close all connections
        void                                        closeConnections();

        //! Update the chat manager
        void                                        update( float deltaTime );

        //! Overriden protocol handler callback method for getting connection notification
        void                                        onConnection( const ChatConnectionConfig& config );

        //! Overriden protocol handler callback method for getting disconnection notification
        void                                        onDisconnection( const ChatConnectionConfig& config );

        //! Overriden protocol handler callback method for getting notification when someone joined to a new channel
        void                                        onJoinedChannel( const ChatConnectionConfig& config );

        //! Overriden protocol handler callback method for getting system messages
        void                                        onReceiveSystemMessage( const std::string& msg );

        //! Get all registered chat protocols
        ProtocolMap&                                getRegisteredProtocols();

    protected:

        //! Map of chat protocols
        ProtocolMap                                 _availableProtocols;

        //! Type for housekeeping of created chats
        typedef std::vector< std::pair< ChatConnectionConfig, BaseChatProtocol* > >  ChatConnections;

        //! Created chat connections
        ChatConnections                             _connections;                              

        //! Control gui
        ChatGuiCtrl*                                _p_chatGuiCtrl;

        //! Chat box gui
        ChatGuiBox*                                 _p_chatGuiBox;

        //! Flag for storing the state of chat box
        bool                                        _activeBox;

        //! Indicates server / client mode, the chat manager behaves slightly different in server mode ( no gui stuff )
        bool                                        _serverMode;

        //! For internal use ( checking for multiple build attempts )
        bool                                        _built;
};

} // namespace vrc
#endif // _VRC_CHATMGR_H_
