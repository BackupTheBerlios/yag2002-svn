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
 # base class for chat protocols
 #
 #   date of creation:  10/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/


#ifndef _CTD_CHATPROTOCOLS_H_
#define _CTD_CHATPROTOCOLS_H_


namespace CTD
{

class BaseChatProtocol;
class ChatConnectionConfig;

//! Registration class for getting msg-receive callbacks
class ChatProtocolCallback
{
    public:
                                                    ChatProtocolCallback() {}

        virtual                                     ~ChatProtocolCallback() {}

        //! Override this method in derived classes in order to get connection notification.
        virtual void                                onConnection( const ChatConnectionConfig& config ) {}

        //! Override this method in derived classes in order to get disconnection notification.
        virtual void                                onDisconnection( const ChatConnectionConfig& config ) {}

        //! Override this method in derived classes in order to get notification when someone joined to a channel.
        virtual void                                onJoinedChannel( const ChatConnectionConfig& cfg ) {}

        //! Override this method in derived classes in order to get notification when someone left the channel.
        virtual void                                onLeftChannel( const ChatConnectionConfig& cfg ) {}

        //! Override this method in derived classes in order to receive the chat traffic.
        virtual void                                onReceive( const std::string& channel, const std::string& sender, const std::string& msg ) {}

        //! Override this to get a callback whenever a nickname changed in channel.
        //! If oldname is empty then newname is the initial one.
        virtual void                                onNicknameChanged( const std::string& newname, const std::string& oldname ) {}

        //! Override this method in derived classes in order to receive member list changes.
        virtual void                                onReceiveMemberList( const std::string& channel ) {}

    private:

        //! Avoid assignment and copying
                                                    ChatProtocolCallback( const ChatProtocolCallback& );

        ChatProtocolCallback&                       operator = ( const ChatProtocolCallback& );
};

//! Base class for all kinds of chat protocols
class BaseChatProtocol
{
    public:
                                                    BaseChatProtocol() {}

        virtual                                     ~BaseChatProtocol() {}

        //! This method is used for creating an instance of an registered protocol handler.
        virtual BaseChatProtocol*                   createInstance() = 0;

        //! Create a new connection to a chat server
        virtual void                                createConnection( const ChatConnectionConfig& conf ) = 0;

        //! Destroy connection
        virtual void                                destroyConnection() = 0;

        //! Send a chat message to a channel
        virtual void                                send( const std::string& msg, const std::string& channel ) = 0;

        //! Get list of chat members in given channel.
        virtual void                                getMemberList( const std::string& channel, std::vector< std::string >& list ) = 0;

        //! Register a protocol callback for given channel filter. Use asterix '*' for channel name to get unfiltered callbacks.
        //! It is possible to register the same callback object for several channels.
        void                                        registerProtocolCallback( ChatProtocolCallback* p_cb, const std::string& channelname = "*" );

        //! Deregister protocol callback for given channel. The callback object is deleted.
        void                                        deregisterProtocolCallback( const std::string& channelname );

        //! Deregister a protocol callback object and all of its associated channels. The callback objects are deleted.
        void                                        deregisterProtocolCallback( const ChatProtocolCallback* p_cb );

    protected:

        //! Type for protocol callback list ( channel name / callback object )
        typedef std::vector< std::pair< std::string, ChatProtocolCallback* > > ProtocolCallbackList;

        //! Protocol callback list
        ProtocolCallbackList                        _protocolCallbacks;

    private:

        //! Avoid assignment and copying
                                                    BaseChatProtocol( const BaseChatProtocol& );

        BaseChatProtocol&                           operator = ( const BaseChatProtocol& );
};

} // namespace CTD
#endif // _CTD_CHATPROTOCOLS_H_