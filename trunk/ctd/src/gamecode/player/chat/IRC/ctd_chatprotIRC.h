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


#ifndef _CTD_CHATPROTIRC_H_
#define _CTD_CHATPROTIRC_H_

#include "../ctd_chatprotocol.h"
#include "libIRC/libircclient.h"


namespace CTD
{

// Chat client for IRC protocol
class ChatNetworkingIRC : public OpenThreads::Thread, public BaseChatProtocol
{
    public:

                                                    ChatNetworkingIRC();

        virtual                                     ~ChatNetworkingIRC();


        //! This method is used for creating an instance of protocol handler.
        ChatNetworkingIRC*                          createInstance();

        //! Connect to an IRC server
        void                                        createConnection( const ChatConnectionConfig& conf );

        //! Destroy connection to server
        void                                        destroyConnection();

        //! Send a chat message
        void                                        send( const std::string& msg, const std::string& channel );

        //! This method is called uppon successful connection
        void                                        connected();

        //! Get list of chat members in given channel.
        void                                        getMemberList( const std::string& channel, std::vector< std::string >& list );

        //! This method is called uppon joining to a channel
        void                                        joined( const std::string& channel, const std::string& name );

        //! Method for distributing new incoming message to all registered callbacks.
        //! In normal case only internal functions make use of this method.
        void                                        recvMessage( const std::string& channel, const std::string& sender, const std::string& msg );

        //! Method for distributing a nickname change in channel to all registered callbacks.
        //! In normal case only internal functions make use of this method.
        void                                        recvNicknameChange( const std::string& newname, const std::string& oldname );

    protected:

        //! Thread's run method
        void                                        run();

        irc_session_t*                              _p_session;

        ChatConnectionConfig*                       _p_config;
};

} // namespace CTD
#endif // _CTD_CHATPROTIRC_H_
