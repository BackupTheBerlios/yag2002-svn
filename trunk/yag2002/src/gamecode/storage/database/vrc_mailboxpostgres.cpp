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
 # class for mailbox functionality on server
 #
 #   date of creation:  07/29/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_mailboxpostgres.h"


namespace vrc
{

MailboxPostgres::MailboxPostgres( pqxx::connection* p_database ) :
 _p_databaseConnection( p_database )
{
    assert( _p_databaseConnection && "invalid database object!" );
}

MailboxPostgres::~MailboxPostgres()
{
}

bool MailboxPostgres::getMailHeaders(  unsigned int userID, unsigned int attribute, const std::string& folder, std::vector< MailboxContent >& headers )
{
    //! TODO
    return false;
}

bool MailboxPostgres::getMail(  unsigned int userID, unsigned int mailID, MailboxContent& mailcontent )
{
    //! TODO
    return false;
}

bool MailboxPostgres::sendMail(  unsigned int userID, const MailboxContent& mailcontent )
{
    //! TODO
    return false;
}

bool MailboxPostgres::deleteMail(  unsigned int userID, unsigned int mailID )
{
    //! TODO
    return false;
}

bool MailboxPostgres::moveMail(  unsigned int userID, unsigned int mailID, const std::string& destfolder )
{
    //! TODO
    return false;
}

bool MailboxPostgres::createMailFolder(  unsigned int userID, const std::string& folder )
{
    //! TODO
    return false;
}

bool MailboxPostgres::deleteMailFolder(  unsigned int userID, const std::string& folder )
{
    //! TODO
    return false;
}

} // namespace vrc
