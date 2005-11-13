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

#include <ctd_main.h>
#include "ctd_chatprotocol.h"
#include "ctd_chatmgr.h"

namespace vrc
{

void BaseChatProtocol::registerProtocolCallback( ChatProtocolCallback* p_cb, const std::string& channelname )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        if ( ( p_beg->second == p_cb ) &&( p_beg->first == channelname ) )
        {
            throw ChatExpection( "Callback for channel '" + channelname + "' already exists!" );
        }
    }
    _protocolCallbacks.push_back( std::make_pair( channelname, p_cb ) );
}

void BaseChatProtocol::deregisterProtocolCallback( const std::string& channelname )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        if ( p_beg->first == channelname )
        {
            _protocolCallbacks.erase( p_beg );
            return;
        }
    }

    throw ChatExpection( "Callback for channel '" + channelname + "' does not exist." );
}

void BaseChatProtocol::deregisterProtocolCallback( const ChatProtocolCallback* p_cb )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        if ( p_beg->second == p_cb )
        {
            _protocolCallbacks.erase( p_beg );
            return;
        }
    }

    throw ChatExpection( "Callback does not exist." );
}

} // namespace vrc
