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
 # base class for storage types
 #
 #   date of creation:  09/30/2007
 #
 #   author:            mpretz 
 #
 ################################################################*/

#ifndef _VRC_BASESTORAGE_H_
#define _VRC_BASESTORAGE_H_

#include <vrc_main.h>
#include "vrc_connectiondata.h"
#include "vrc_account.h"


namespace vrc
{

class StorageServer;
class UserInventory;

class BaseStorage
{
    public:

                                                BaseStorage();

        virtual                                 ~BaseStorage();

        //! Get the storage type ( e.g. PostgreSQL ).
        const std::string&                      getType() const;

        //! Try to login, if successful then return true; in this case 'acc' contains the valid user account data.
        virtual bool                            loginUser( const std::string login, const std::string passwd, UserAccount& acc ) = 0;

        //! Logout user with given login name
        virtual bool                            logoutUser( const std::string login ) = 0;

        //! Update the user account. 'acc' must have a valid user ID and the user must have been successfully logged in before.
        virtual bool                            updateUserAccount( const UserAccount& acc ) = 0;

        //! Get public user account information. 'acc' must contain a nickname.
        virtual bool                            getPublicUserAccountInfo( UserAccount& acc ) = 0;

        //! Get user data for given user ID. The user must be successfully logged in.
        virtual bool                            getUserData( unsigned int userID, UserData& data ) = 0;

        //! Get user inventory for given data ID ( use getUserData for getting the dataID ). The user must be successfully logged in.
        virtual bool                            getUserInventory( unsigned int dataID, UserInventory* p_inv ) = 0;

        //! Register a new user. Return false if the registration went wrong.
        virtual bool                            registerUser( const std::string& name, const std::string& login, const std::string& passwd, const std::string& email ) = 0;

    protected:

        //! Avoid copy constructor and assignment operator
                                                BaseStorage( const BaseStorage& );

        BaseStorage&                            operator =( BaseStorage& );

        //! Initialize the storage by connecting to storage server
        virtual bool                            initialize( const ConnectionData& connData ) = 0;

        //! Release the storage, after this call the storage can be initialized with new connection data.
        virtual void                            release() = 0;

        std::string                             _type;

    friend class StorageServer;
};

} // namespace vrc

#endif  // _VRC_BASESTORAGE_H_
