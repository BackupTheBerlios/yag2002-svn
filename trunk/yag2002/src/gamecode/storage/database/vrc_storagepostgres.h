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
 # implementation of PostgreSQL storage
 #
 #   date of creation:  10/18/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_MYSQLSTORAGE_H_
#define _VRC_MYSQLSTORAGE_H_

#include <vrc_main.h>
#include "vrc_basestorage.h"

//! This causes to use the shared lib of pqxx
#define PQXX_SHARED
#include <pqxx/connection.hxx>


namespace vrc
{

class UserInventory;
class StorageServer;


class StoragePostgreSQL : public BaseStorage
{
    public:

                                                StoragePostgreSQL();

        virtual                                 ~StoragePostgreSQL();

        //! Try to login, if successful then return true; in this case 'acc' contains the valid user account data.
        bool                                    loginUser( const std::string login, const std::string passwd, UserAccount& acc );

        //! Logout the user
        bool                                    logoutUser( const std::string login );

        //! Register a new user.
        bool                                    registerUser( const std::string& name, const std::string& login, const std::string& passwd, const std::string& email );

        //! Update the user account. 'acc' must have a valid user ID and the user must have been successfully logged in before.
        bool                                    updateUserAccount( const UserAccount& acc );

        //! Get public user account information. 'acc' must contain a nickname.
        bool                                    getPublicUserAccountInfo( UserAccount& acc );

        //! Get user data for given uid. The user must be successfully logged in.
        bool                                    getUserData( unsigned int userID, UserData& data );

        //! Get user inventory for given user ID. The user must be successfully logged in.
        bool                                    getUserInventory( unsigned int userID, UserInventory* p_inv );

    protected:

        //! Initialze the connection to database
       bool                                     initialize( const ConnectionData& connData );

        //! Release the connection to database
       void                                     release();

       //! Given a string prepare it for using in a SQL expression.
       std::string                              cleanString( const std::string& str );

       //! Postgres database connection
       pqxx::connection*                        _p_databaseConnection;

       //! The storage server needs the database connection
       friend class StorageServer;
};

} // namespace vrc

#endif  // _VRC_MYSQLSTORAGE_H_
