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
 # functionalities for account requests from storage
 #
 #   date of creation:  10/03/2007
 #
 #   author:            mpretz 
 #
 ################################################################*/

#ifndef _VRC_ACCOUNT_H_
#define _VRC_ACCOUNT_H_

#include <vrc_main.h>

namespace vrc
{

class BaseStorage;

//! Account related information
class AccountData
{
    public:

                                                AccountData();

                                                ~AccountData();

        const std::string&                      getUsername() const;

        const std::string&                      getFirstname() const;

        const std::string&                      getLastname() const;

        const std::string&                      getNickname() const;

        const std::string&                      getEmail() const;

        unsigned int                            getLastLogin() const;

        unsigned int                            getUserId() const;

        void                                    setUsername( const std::string& username );

        void                                    setFirstname( const std::string& firstname );

        void                                    setLastname( const std::string& lastname );

        void                                    setNickname( const std::string& nickname );

        void                                    setEmail( const std::string& email );

        void                                    setLastLogin( const unsigned int lastlogin );

        void                                    setUserId( const unsigned int userID );

    private:

        std::string                             _userName;

        std::string                             _firstName;

        std::string                             _lastName;

        std::string                             _nickName;

        std::string                             _email;

        unsigned int                            _lastLogin;

        unsigned int                            _userID;
};

//! Account query functionalities are implemented in this class
class Account
{
    public:

                                                Account( BaseStorage* p_storage );

                                                ~Account();

        //! Login a user; after a successful call account holds the user ID.
        bool                                    loginUser( AccountData& account, const std::string& password );

        //! Logout a user
        bool                                    logoutUser( const AccountData& account );

        //! Write back the account data
        bool                                    updateAccount( const AccountData& account, const std::string& password = "" );

    protected:

        BaseStorage*                            _p_storage;
};

} // namespace vrc

#endif  // _VRC_ACCOUNT_H_
