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
 ################################################################*/

#ifndef _VRC_MAILBOXPOSTGRES_H_
#define _VRC_MAILBOXPOSTGRES_H_

#include <vrc_main.h>
#include "vrc_basemailbox.h"

//! This causes to use the shared lib of pqxx
#define PQXX_SHARED
#include <pqxx/connection.hxx>

namespace vrc
{

class MailboxPostgres : public BaseMailbox
{
    public:

        //! Create the mailbox server for postgres database with given connection object. It must be already setup.
        explicit                                MailboxPostgres( pqxx::connection* p_database );

        virtual                                 ~MailboxPostgres();

        //! Get mail headers with given attribute (see Content::Attributes), they are stored in 'headers' without body.
        virtual bool                            getMailHeaders( unsigned int userID, unsigned int attribute, const std::string& folder, std::vector< Content >& headers );

        //! Get mail with given ID.
        virtual bool                            getMail( unsigned int userID, unsigned int mailID, Content& mailcontent );

        //! Send mail
        virtual bool                            sendMail( unsigned int userID, const Content& mailcontent );

        //! Delete mail with given ID.
        virtual bool                            deleteMail( unsigned int userID, unsigned int mailID );

        //! Move mail with given ID to 'destfolder'. The destination folder must exist.
        virtual bool                            moveMail( unsigned int userID, unsigned int mailID, const std::string& destfolder );

        //! Create a new mail folder.
        virtual bool                            createMailFolder( unsigned int userID, const std::string& folder );

        //! Delete the given mail folder.
        virtual bool                            deleteMailFolder( unsigned int userID, const std::string& folder );

    protected:

       //! Postgres database connection
       pqxx::connection*                        _p_databaseConnection;
};

} // namespace vrc

#endif  // _VRC_MAILBOXPOSTGRES_H_
