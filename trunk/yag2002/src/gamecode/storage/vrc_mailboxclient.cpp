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
 _p_networking( NULL )
{
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

    //! TODO: remove this test code
    {
        BaseMailbox::Content content;
        content._from = "boto";
        content._to   = "someone, nokky, micha";
        content._cc   = "cc_reciepient";
        content._subject = "test mail";
        content._body    = "this is a test mail with special characters ÄÖÜäöüßßß\nand this is the second line of e-mail :-)";
        _p_networking->sendMail( 42, content );
    }

}

bool MailboxClient::getMailHeaders( unsigned int userID, unsigned int attribute, const std::string& folder, std::vector< Content >& headers )
{
    //! TODO: use _p_networking for sending out the request to server
    return false;
}

bool MailboxClient::getMail( unsigned int userID, unsigned int mailID, Content& mailcontent )
{
    //! TODO
    return false;
}

bool MailboxClient::sendMail( unsigned int userID, const Content& mailcontent )
{
    //! TODO
    return false;
}

bool MailboxClient::deleteMail( unsigned int userID, unsigned int mailID )
{
    //! TODO
    return false;
}

bool MailboxClient::moveMail( unsigned int userID, unsigned int mailID, const std::string& destfolder )
{
    //! TODO
    return false;
}

bool MailboxClient::createMailFolder( unsigned int userID, const std::string& folder )
{
    //! TODO
    return false;
}

bool MailboxClient::deleteMailFolder( unsigned int userID, const std::string& folder )
{
    //! TODO
    return false;
}

} // namespace vrc
