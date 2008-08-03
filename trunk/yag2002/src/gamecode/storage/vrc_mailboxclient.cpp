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
 # class for mailbox functionality used by storage client
 #
 #   date of creation:  07/29/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_mailboxclient.h"
#include "vrc_mailboxnetworking.h"



//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::MailboxClient )

namespace vrc
{

MailboxClient::MailboxClient() :
 _p_networking( NULL ),
 _p_cbResponse( NULL )
{

//! TODO: remove this, it is only for developing the client/networking code
//        later, the networking object is automatically replicated on client!
_p_networking = new MailboxNetworking;
_p_networking->setMailboxResponseCallback( this );

}

MailboxClient::~MailboxClient()
{
}

void MailboxClient::initialize()
{
    log_info << "MailboxClient: initializing" << std::endl;

    // currently, nothing to do here
}

void MailboxClient::shutdown()
{
    log_info << "MailboxClient: shutting down" << std::endl;

    // destroy the singleton
    destroy();
}

void MailboxClient::setNetworking( MailboxNetworking* p_networking )
{
    assert( ( _p_networking == NULL ) && "networking object already set!" );

    _p_networking = p_networking;
    _p_networking->setMailboxResponseCallback( this );
}

void MailboxClient::mailboxNetworkingResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response )
{
    if ( !_p_cbResponse )
    {
        log_error << "MailboxClient: unexpected response from server" << std::endl;
        return;
    }

    log_debug << "MailboxClient: server responds '" << response << "'" << std::endl;

    _p_cbResponse->mailboxResponse( mailcontent, status, response );
    _p_cbResponse = NULL; // reset the callback
}

void MailboxClient::getMailFolders( MailboxResponseCallback* p_cbResponse )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->getMailFolders();
}

void MailboxClient::getMailHeaders( MailboxResponseCallback* p_cbResponse, unsigned int attribute, const std::string& folder )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->getMailHeaders( attribute, folder );
}

void MailboxClient::getMail( MailboxResponseCallback* p_cbResponse, unsigned int mailID )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->getMail( mailID );
}

void MailboxClient::sendMail( MailboxResponseCallback* p_cbResponse, const MailboxContent& mailcontent )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->sendMail( mailcontent );
}

void MailboxClient::deleteMail( MailboxResponseCallback* p_cbResponse, unsigned int mailID )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->deleteMail( mailID );
}

void MailboxClient::moveMail( MailboxResponseCallback* p_cbResponse, unsigned int mailID, const std::string& destfolder )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->moveMail( mailID, destfolder );
}

void MailboxClient::createMailFolder( MailboxResponseCallback* p_cbResponse, const std::string& folder )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->createMailFolder( folder );
}

void MailboxClient::deleteMailFolder( MailboxResponseCallback* p_cbResponse, const std::string& folder )
{
    assert( p_cbResponse && "invalid callback object" );

    if ( !_p_networking )
        return;

    _p_cbResponse = p_cbResponse;
    _p_networking->deleteMailFolder( folder );
}

} // namespace vrc
