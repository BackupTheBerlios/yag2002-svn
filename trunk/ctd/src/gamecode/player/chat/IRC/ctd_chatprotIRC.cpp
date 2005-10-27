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
 # IRC chat client
 #  this class uses Georgy Yunaev's C++ lib http://libircclient.sourceforge.net
 #
 #   date of creation:  10/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_chatprotIRC.h"
#include "../ctd_chatmgr.h"
#include "libIRC/libircclient.h"

namespace CTD
{

#define IRC_CMD_LIST    "/help\n"\
                        "/nick [nickname]\n"\
                        "/join [still_not_implemented]"

//! Class for defining an IRC session context
class IRCSessionContext
{
    public:
                                                IRCSessionContext() :
                                                 _p_session( NULL ),
                                                 _p_chatClient( NULL )
                                                {
                                                }

        virtual                                 ~IRCSessionContext() {}

        std::string                             _channel;

        std::string                             _nickname;

        irc_session_t*                          _p_session;

        ChatNetworkingIRC*                      _p_chatClient;
};

// internal callback functions
//----
void event_connect( irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count )
{
    // auto-join after connection
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    if ( p_ctx->_channel.length() )
        irc_cmd_join( session, p_ctx->_channel.c_str(), 0 );

    // start the callback mechanism for connection
    p_ctx->_p_chatClient->connected();

    // store actual nick name
    char p_nickbuf[ 128 ];
    irc_target_get_nick( params[ 0 ], p_nickbuf, sizeof( p_nickbuf ) );
    p_ctx->_nickname = p_nickbuf;
    // notify for nickname change
    p_ctx->_p_chatClient->recvNicknameChange( p_ctx->_nickname, "" );
}

void event_join( irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count )
{
    if ( !origin )
        return;

    // hide IP address, in sake of security 
	irc_cmd_user_mode( session, "+x" );

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    if ( std::string( origin ) != p_ctx->_nickname )
    {
        p_ctx->_p_chatClient->recvMessage( p_ctx->_channel, origin, " joined the channel" );
    }
    else
    {
        p_ctx->_p_chatClient->joined( p_ctx->_channel, params[ 0 ] );
    }
}

void event_nick( irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count )
{
    if ( !origin )
        return;

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    if ( std::string( origin ) != p_ctx->_nickname )
        p_ctx->_p_chatClient->recvMessage( p_ctx->_channel, "* ", std::string( origin ) + " changed nickname to " + std::string( params[ 0 ] ) );
    else
    {
        p_ctx->_p_chatClient->recvMessage( p_ctx->_channel, "* ", " you changed your nickname to " + std::string( params[ 0 ] ) );
        
        // notify for nickname change
        p_ctx->_p_chatClient->recvNicknameChange( params[ 0 ], p_ctx->_nickname );

        p_ctx->_nickname = params[ 0 ];
    }
}

void event_numeric( irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count )
{
    if ( event > 400 )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "IRC protocol error " << 
            event << ": " <<
            params[0];

        std::string err;
        err += " " + ( count > 1 ) ? params[ 1 ] : "";
        err += " " + ( count > 2 ) ? params[ 2 ] : "";
        err += " " + ( count > 3 ) ? params[ 3 ] : "";
        log << Log::LogLevel( Log::L_ERROR ) << err << std::endl;
    }
}

void event_channel( irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count )
{
    char nickbuf[128];

    if ( count != 2 )
        return;

    std::string sender( origin ? origin : "someone" );
    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( session ) );
    p_ctx->_p_chatClient->recvMessage( p_ctx->_channel, sender, params[1] );

    if ( !origin )
        return;

    irc_target_get_nick( origin, nickbuf, sizeof( nickbuf ) );

    //! TBD

    //if ( !strcmp (params[1], "quit") )
    //  irc_cmd_quit (session, "of course, Master!");

    //if ( !strcmp (params[1], "help") )
    //{
    //  irc_cmd_msg (session, params[0], "quit, help, dcc chat, dcc send, ctcp");
    //}

    //if ( !strcmp (params[1], "ctcp") )
    //{
    //  irc_cmd_ctcp_request (session, nickbuf, "PING 223");
    //  irc_cmd_ctcp_request (session, nickbuf, "FINGER");
    //  irc_cmd_ctcp_request (session, nickbuf, "VERSION");
    //  irc_cmd_ctcp_request (session, nickbuf, "TIME");
    //}

    //if ( !strcmp (params[1], "dcc chat") )
    //{
    //  irc_dcc_t dccid;
    //  irc_dcc_chat (session, 0, nickbuf, dcc_recv_callback, &dccid);
    //  printf ("DCC chat ID: %d\n", dccid);
    //}

    //if ( !strcmp (params[1], "dcc send") )
    //{
    //  irc_dcc_t dccid;
    //  irc_dcc_sendfile (session, 0, nickbuf, "irctest.c", dcc_file_recv_callback, &dccid);
    //  printf ("DCC send ID: %d\n", dccid);
    //}

    //if ( !strcmp (params[1], "/topic") )
    //    irc_cmd_topic (session, params[0], 0);
    //else if ( strstr (params[1], "/topic ") == params[1] )
    //    irc_cmd_topic (session, params[0], params[1] + 6);

    //if ( strstr (params[1], "/mode ") == params[1] )
    //    irc_cmd_channel_mode (session, params[0], params[1] + 5);

    //if ( strstr (params[1], "/nick ") == params[1] )
    //    irc_cmd_nick (session, params[1] + 5);

    //if ( strstr (params[1], "/whois ") == params[1] )
    //    irc_cmd_whois (session, params[1] + 5);
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

ChatNetworkingIRC* ChatNetworkingIRC::createInstance()
{
    ChatNetworkingIRC* p_inst = new ChatNetworkingIRC;
    return p_inst;
}

void ChatNetworkingIRC::send( const std::string& msg, const std::string& channel )
{
    if ( !_p_session )
        throw ChatExpection( "Invalid network session" );

    if ( !msg.length() )
        throw ChatExpection( "Request for sending empty message" );

    if ( msg[ 0 ] == '/' )
    {
        // check for /msg commands, send them raw
        if ( ( msg.length() > 4 ) && !msg.compare( 0, 4, "/msg" ) )
        {
            irc_send_raw( _p_session, msg.c_str() );
            return;
        }

        std::vector< std::string > args;
        explode( msg, " ", &args );

        // all commands without arguments go here
        if ( args.size() == 1 )
        {
            if ( args[ 0 ] == "/help" )
            {
                recvMessage( channel, "* ", IRC_CMD_LIST );
                return;
            }
        }
        // all commands with one single argument go here
        else if ( args.size() == 2 )        
        {
            if ( args[ 0 ] == "/nick" )
            {
                irc_cmd_nick( _p_session, args[ 1 ].c_str() );
                return;
            }
            else if ( args[ 0 ] == "/join" )           
            {
                //! TODO: what is "key" about?
                irc_cmd_join( _p_session, args[ 1 ].c_str(), NULL );
                return;
            }
            else
            {
                recvMessage( channel, "* ", IRC_CMD_LIST );
                return;
            }
        }
    }
    else
    {
        irc_cmd_msg( _p_session, channel.c_str(), msg.c_str() );
    }
}

void ChatNetworkingIRC::getMemberList( const std::string& channel, std::vector< std::string >& list )
{
    //! TODO
    list.push_back( "!not implemented" );
}

void ChatNetworkingIRC::connected()
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
        p_beg->second->onConnection( *_p_config );
}

void ChatNetworkingIRC::joined( const std::string& channel, const std::string& name )
{
    CTD::ChatConnectionConfig cfg( *_p_config );
    cfg._nickname = name;
    cfg._channel  = channel;
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( ( p_beg->first == channel ) || ( p_beg->first == "*" ) )
            p_beg->second->onJoinedChannel( cfg );
}

void ChatNetworkingIRC::recvMessage( const std::string& channel, const std::string& sender, const std::string& msg )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        // check also for unfiltered callbacks ( '*' )
        if ( ( channel == p_beg->first ) || ( p_beg->first == "*" ) )
            p_beg->second->onReceive( channel, sender, msg );
    }
}

void ChatNetworkingIRC::recvNicknameChange( const std::string& newname, const std::string& oldname )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
        p_beg->second->onNicknameChanged( newname, oldname );
}

void ChatNetworkingIRC::createConnection( const ChatConnectionConfig& conf )
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
    // start up winsock
    WORD    version;
    WSADATA data;
    int     err;
    version = MAKEWORD( 2, 2 );
    err = WSAStartup( version, &data );
    if ( err != 0 ) 
    {
        throw ChatExpection( "Cannot setup ms winsock" );
    }
#endif

    irc_callbacks_t callbacks;
    memset( &callbacks, 0, sizeof( callbacks ) );
    // set up the callbacks
    callbacks.event_connect = event_connect;
    callbacks.event_nick    = event_nick;
    callbacks.event_join    = event_join;
    callbacks.event_numeric = event_numeric;
    callbacks.event_channel = event_channel;

    irc_session_t* p_session = NULL;

    p_session = irc_create_session( &callbacks );
    if ( !p_session )
    {
        throw ChatExpection( "Could not create networking session" );
    }

    // store the context information
    IRCSessionContext* p_ctx = new IRCSessionContext;
    p_ctx->_p_chatClient = this;
    p_ctx->_channel      = conf._channel;
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
        throw ChatExpection( "Could not connect: " + std::string( irc_strerror( irc_errno( p_session ) ) ) );
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
    // start the IRC protocol loop
    irc_run( _p_session );
}

void ChatNetworkingIRC::destroyConnection()
{
    // kill the thread
    cancel();

    IRCSessionContext* p_ctx = static_cast< IRCSessionContext* >( irc_get_ctx( _p_session ) );
    delete p_ctx;

    irc_destroy_session( _p_session );
    _p_session = NULL;

    // at last destroy ourself
    delete this;
}

} // namespace CTD
