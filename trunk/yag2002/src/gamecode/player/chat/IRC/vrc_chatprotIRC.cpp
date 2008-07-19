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
 # IRC chat client
 #  this class uses Georgy Yunaev's C++ lib http://libircclient.sourceforge.net
 #
 #   date of creation:  10/13/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_chatprotIRC.h"
#include "../vrc_chatmgr.h"
#include "libIRC/libircclient.h"
#include "libIRC/libirc_rfcnumeric.h"


//! Maximal length of incoming text characters
#define MAX_INCOMING_TEXT_LEN   128

namespace vrc
{

#define IRC_CMD_LIST    "\n"\
                        "/help\n"\
                        "/part\n"\
                        "/names\n"\
                        "/msg   [arguments]\n"\
                        "/whois [nickname]\n"\
                        "/nick  [nickname]\n"\
                        "/join  [#channel]"

//! Class for defining an IRC session context
class IRCSessionContext
{
    public:
                                                IRCSessionContext() :
                                                 _p_session( NULL ),
                                                 _p_handler( NULL )
                                                {
                                                }

        virtual                                 ~IRCSessionContext() {}

        std::string                             _channel;

        std::string                             _nickname;

        irc_session_t*                          _p_session;

        ChatNetworkingIRC*                      _p_handler;
};

// internal callback functions
//----
void event_connect( irc_session_t * session, const char * /*event*/, const char * /*origin*/, const char ** params, unsigned int /*count*/ )
{
    // auto-join after connection
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    if ( p_ctx->_channel.length() )
        irc_cmd_join( session, p_ctx->_channel.c_str(), 0 );

    // start the callback mechanism for connection
    p_ctx->_p_handler->connected();

    // store actual nick name
    char p_nickbuf[ 128 ];
    irc_target_get_nick( params[ 0 ], p_nickbuf, sizeof( p_nickbuf ) );
    p_ctx->_nickname = p_nickbuf;
    // notify for nickname change
    p_ctx->_p_handler->recvNicknameChange( p_ctx->_channel, p_ctx->_nickname, "" );
}

void event_join( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int /*count*/ )
{
    if ( !origin )
        return;

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    p_ctx->_p_handler->joined( params[ 0 ], origin );
}

void event_quit( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int /*count*/ )
{
    if ( !origin )
        return;

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    p_ctx->_p_handler->left( "", origin ); // let the channel name blank so it means the user quitted
}

void event_nick( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int /*count*/ )
{
    if ( !origin )
        return;

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    if ( std::string( origin ) == p_ctx->_nickname )
    {
        // notify for nickname change, we identify an own nick change by setting old and new names to new name
        p_ctx->_p_handler->recvNicknameChange( params[ 0 ], params[ 0 ], params[ 0 ] );
        p_ctx->_nickname = params[ 0 ];
    }
    else
    {
        p_ctx->_p_handler->recvNicknameChange( params[ 0 ], params[ 0 ], origin );
    }
}

void event_kick( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int count )
{
    if ( !origin || ( count < 1 ) )
        return;

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    p_ctx->_p_handler->recvKicked( params[ 0 ], std::string( origin ), std::string( params[ 1 ] ) );
}

void event_part( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int /*count*/ )
{
    if ( !origin )
        return;

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    p_ctx->_p_handler->left( params[ 0 ], std::string( origin ) );
}

void event_numeric( irc_session_t * session, unsigned int event, const char * /*origin*/, const char ** params, unsigned int count )
{
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );

    // handle /names stuff
    if ( event == LIBIRC_RFC_RPL_NAMREPLY )
    {
        std::vector< std::string > names;
        std::string namestring( params[ 3 ] );

        yaf3d::explode( namestring, " ", &names );
        for ( size_t cnt = 0; cnt < names.size(); ++cnt )
            p_ctx->_p_handler->_nickNames[ names[ cnt ] ] = names[ cnt ];
    }
    // this signalized the end of name list transmission
    else if ( event == LIBIRC_RFC_RPL_ENDOFNAMES )
    {
        p_ctx->_p_handler->recvMemberList( params[ 1 ] );
    }
    // handle /whois stuff
    else if ( event == LIBIRC_RFC_RPL_WHOISUSER )
    {
        if ( count > 1 )
        {
            std::string header( "WHOIS " );
            header += params[ 1 ];
            p_ctx->_p_handler->recvSystemMessage( header );
            for ( unsigned int cnt = 2; cnt < count; cnt ++ )
                p_ctx->_p_handler->recvSystemMessage( params[ cnt ] );
        }
    }
    else if ( event == LIBIRC_RFC_RPL_ENDOFWHOIS )
    {
        p_ctx->_p_handler->recvSystemMessage( "--------" );
    }
    // just output system message
    else if ( event > 400 )
    {
        std::string msg;

        if ( count > 0 )
            msg += std::string( params[ 0 ] );

        if ( count > 1 )
            msg += " " + std::string( params[ 1 ] );

        if ( count > 2 )
            msg += " " + std::string( params[ 2 ] );

        p_ctx->_p_handler->recvSystemMessage( msg );
    }
}

void event_notice( irc_session_t * session, const char * /*event*/, const char * /*origin*/, const char ** params, unsigned int count )
{
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    std::string msg;
    if ( count > 0 )
        msg += std::string( params[ 0 ] ) + ": ";
    if ( count >= 1 )
        msg += std::string( params[ 1 ] );

    p_ctx->_p_handler->recvSystemMessage( msg );
}

void event_privmsg( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int count )
{
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    std::string msg( "PrivMsg " );
    if ( count > 0 )
        msg += "from " + std::string( origin ) + ": ";
    if ( count > 1 )
        msg += std::string( params[ 1 ] );

    p_ctx->_p_handler->recvSystemMessage( msg );
}

void event_channel( irc_session_t * session, const char * /*event*/, const char * origin, const char ** params, unsigned int count )
{
    char nickbuf[128];

    if ( count != 2 )
        return;

    std::string sender( origin ? origin : "{someone}" );
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    
    // the string conversion in CEGUI can fail, we skip the message in this situation
    try
    {
        // a hack for getting utf8 into cegui string
        const CEGUI::utf8* p_buf = reinterpret_cast< const CEGUI::utf8* >( params[ 1 ] );
        std::size_t len = strlen( params[ 1 ] );
        CEGUI::String  msg( len, 0 );
        msg.replace( 0, len, p_buf , len );

        p_ctx->_p_handler->recvMessage( params[ 0 ], sender, msg );
    }
    catch ( ... )
    {
        p_ctx->_p_handler->recvMessage( params[ 0 ], sender, "*** {cannot deliver text!}" );
    }

    if ( !origin )
        return;

    irc_target_get_nick( origin, nickbuf, sizeof( nickbuf ) );
}
//----


ChatNetworkingIRC::ChatNetworkingIRC() :
_p_session( NULL ),
_p_config( NULL )
{
}

ChatNetworkingIRC::~ChatNetworkingIRC()
{
    if ( _p_session )
        destroyConnection();

    if ( _p_config )
        delete _p_config;
}

BaseChatProtocol* ChatNetworkingIRC::createInstance()
{
    ChatNetworkingIRC* p_inst = new ChatNetworkingIRC;
    return p_inst;
}

void ChatNetworkingIRC::send( const CEGUI::String& msg, const std::string& channel, const std::string& recipient )
{
    // ignore input if no valid session exists
    if ( !_p_session )
        return;

    if ( !msg.length() )
        return;

    std::string smsg( reinterpret_cast< const char* >( msg.c_str() ) );
    if ( ( smsg[ 0 ] == '/' ) || recipient.length() )
    {
        std::vector< std::string > args;
        yaf3d::explode( smsg, " ", &args );

        // check for /msg commands, send them raw
        if ( ( args.size() > 2 ) && ( args[ 0 ] == "/msg" ) )
        {
            // assemble the /msg command argument
            std::string text;
            for ( std::size_t cnt = 2; cnt < args.size(); ++cnt )
                text += ( " " + args[ cnt ] );

            irc_cmd_msg( _p_session, args[ 1 ].c_str(), text.c_str() );
            return;
        }
        else if ( recipient.length() )
        {
            irc_cmd_msg( _p_session, recipient.c_str(), msg.c_str() );
            return;
        }

        // all commands without arguments go here
        if ( args.size() == 1 )
        {
            if ( args[ 0 ] == "/names" )
            {
                requestMemberList( channel );
            }
            else if ( args[ 0 ] == "/part" )
            {
                requestLeaveChannel( channel );
            }
            else
            {
                recvMessage( channel, "* ", IRC_CMD_LIST );
            }

        }
        // all commands with one single argument go here
        else if ( args.size() == 2 )
        {
            if ( args[ 0 ] == "/nick" )
            {
                irc_cmd_nick( _p_session, args[ 1 ].c_str() );
            }
            else if ( args[ 0 ] == "/join" )
            {
                //! TODO: what is "key" about?
                irc_cmd_join( _p_session, args[ 1 ].c_str(), NULL );
            }
            else if ( args[ 0 ] == "/whois" )
            {
                irc_cmd_whois( _p_session, args[ 1 ].c_str() );
            }
            else
            {
                recvMessage( channel, "* ", IRC_CMD_LIST );
            }
        }
    }
    else
    {
        CEGUI::String message( msg );

        // are we whispering?
        if ( recipient.length() )
        {
            CEGUI::String whisperTo( recipient );
            message = "/msg " + whisperTo + " " + msg;
        }

        irc_cmd_msg( _p_session, channel.c_str(), reinterpret_cast< const char* >( message.c_str() ) );
    }
}

void ChatNetworkingIRC::requestLeaveChannel( const std::string& channel )
{
    irc_cmd_part( _p_session, channel.c_str() );
}

void ChatNetworkingIRC::requestMemberList( const std::string& channel )
{
    _nickNames.clear();
    irc_cmd_names( _p_session, channel.c_str() );
}

void ChatNetworkingIRC::getMemberList( const std::string& /*channel*/, std::vector< std::string >& list )
{
    // currently 'channel' is unused in IRC protocol!
    std::map< std::string, std::string >::iterator p_beg = _nickNames.begin(), p_end = _nickNames.end();
    for ( ; p_beg != p_end; ++p_beg )
        list.push_back( p_beg->first );
}

void ChatNetworkingIRC::connected()
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->onConnection( *_p_config );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::left( const std::string& channel, const std::string& name )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    //! NOTE: if channel is empty then it means that a user quitted, if it contains a channel name then it means that the user left only that channel
    vrc::ChatConnectionConfig cfg( *_p_config );
    cfg._nickname = name;
    cfg._channel  = channel;
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( !channel.length() || ( p_beg->first == channel ) || ( p_beg->first == "*" ) )
            p_beg->second->onLeftChannel( cfg );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::recvKicked( const std::string& channel, const std::string& kicker, const std::string& kicked )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    ProtocolCallbackList localcopy = _protocolCallbacks;
    ProtocolCallbackList::iterator p_beg = localcopy.begin(), p_end = localcopy.end();
    // send the kick notification unfiltered
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->onKicked( channel, kicker, kicked );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::joined( const std::string& channel, const std::string& name )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    vrc::ChatConnectionConfig cfg( *_p_config );
    cfg._nickname = name;
    cfg._channel  = channel;
    // work on local copy of callback list, as during callbacks new registrations can follow
    ProtocolCallbackList localcopy = _protocolCallbacks;
    ProtocolCallbackList::iterator p_beg = localcopy.begin(), p_end = localcopy.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( ( p_beg->first == channel ) || ( p_beg->first == "*" ) )
            p_beg->second->onJoinedChannel( cfg );

    requestMemberList( channel );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::recvMessage( const std::string& channel, const std::string& sender, const CEGUI::String& msg )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        // check also for unfiltered callbacks ( '*' )
        if ( ( channel == p_beg->first ) || ( p_beg->first == "*" ) )
            p_beg->second->onReceive( channel, sender, msg );
    }

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::recvMemberList( const std::string& channel )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        // check also for unfiltered callbacks ( '*' )
        if ( ( channel == p_beg->first ) || ( p_beg->first == "*" ) )
            p_beg->second->onReceiveMemberList( channel );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::recvSystemMessage( const std::string& msg )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->onReceiveSystemMessage( msg );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::recvNicknameChange( const std::string& /*channel*/, const std::string& newname, const std::string& oldname )
{
    // this call is in another thread context then the drawing context, so use the draw mutex!
    yaf3d::Application::get()->getDrawMutex().lock();

    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->onNicknameChanged( newname, oldname );

    yaf3d::Application::get()->getDrawMutex().unlock();
}

void ChatNetworkingIRC::createConnection( const ChatConnectionConfig& conf ) throw ( ChatExpection )
{
    if ( conf._port == 0 )
    {
        throw ChatExpection( "Invalid Port" );
    }
    if ( !conf._serverURL.length() )
    {
        throw ChatExpection( "Invalid Server URL" );
    }
    if ( !conf._channel.length() )
    {
        throw ChatExpection( "Invalid Channel" );
    }

#ifdef WIN32
    {
        // start up winsock only once
        static bool _wsaInitialized = false;
        if ( !_wsaInitialized )
        {
            _wsaInitialized = true;
            WORD    version;
            WSADATA data;
            int     err;
            version = MAKEWORD( 2, 2 );
            err = WSAStartup( version, &data );
            if ( err != 0 )
            {
                throw ChatExpection( "Cannot setup ms winsock" );
            }
        }
    }
#endif

    irc_callbacks_t callbacks;
    memset( &callbacks, 0, sizeof( callbacks ) );
    // set up the callbacks
    callbacks.event_connect = event_connect;
    callbacks.event_nick    = event_nick;
    callbacks.event_join    = event_join;
    callbacks.event_quit    = event_quit;
    callbacks.event_numeric = event_numeric;
    callbacks.event_channel = event_channel;
    callbacks.event_kick    = event_kick;
    callbacks.event_part    = event_part;
    callbacks.event_notice  = event_notice;
    callbacks.event_privmsg = event_privmsg;

    irc_session_t* p_session = NULL;

    p_session = irc_create_session( &callbacks );
    if ( !p_session )
    {
        throw ChatExpection( "Could not create networking session" );
    }

    // store the context information
    IRCSessionContext* p_ctx = new IRCSessionContext;
    p_ctx->_p_handler = this;
    p_ctx->_channel   = conf._channel;
    irc_set_ctx( p_session, p_ctx );
    _p_session = p_session;

    irc_option_set( p_session, LIBIRC_OPTION_STRIPNICKS );

    // initiate the IRC server connection
    if ( irc_connect(
        p_session,
        conf._serverURL.c_str(),
        conf._port,
        conf._password.length() ? conf._password.c_str() : NULL,
        conf._nickname.c_str(),
        conf._username.length() ? conf._username.c_str() : NULL,
        conf._realname.length() ? conf._realname.c_str() : NULL ) )
    {
        throw ChatExpection( std::string( irc_strerror( irc_errno( p_session ) ) ) );
    }

    // store the configuration
    _p_config  = new ChatConnectionConfig;
    *_p_config = conf;
    _p_config->_p_protocolHandler = this;

    // start the thread -- the method run() below is called in a new created thread
    start();
}

void ChatNetworkingIRC::run()
{
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( _p_session ) );
    // passing through an exception silently kills the process, very subtile source of error!
    try
    {
        // start the IRC protocol loop
        if ( irc_run( _p_session ) )
        {
            std::string ircserver = _p_config ? _p_config->_serverURL : "Unknown";
            log_warning << "*** irc client has problems connection to irc server '" << ircserver << "'" << std::endl;
            recvSystemMessage( "*** irc client has problems connection to irc server '" + ircserver + "'" );
        }
    }
    catch ( ... )
    {
        log_error << "*** internal error occured in ChatNetworkingIRC::run" << std::endl;

        // notify about the problem
        recvSystemMessage( std::string( "Lost connection to Server '" ) + _p_config->_serverURL + "' because of an unexpected failure!" );

        irc_destroy_session( _p_session );
        _p_session = NULL;
        return;
    }
    delete p_ctx;
    irc_destroy_session( _p_session );
    _p_session = NULL;
}

void ChatNetworkingIRC::destroyConnection()
{
    irc_disconnect( _p_session );
    // wait until thread is shut down
    while( isRunning() );
}

} // namespace vrc
