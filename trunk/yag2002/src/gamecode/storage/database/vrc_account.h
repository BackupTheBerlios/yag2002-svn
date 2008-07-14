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
 # account related data
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

//! Account related information
class UserAccount
{
    public:

                                                UserAccount();

                                                ~UserAccount();

        //! Get the user account ID
        unsigned int                            getUserId() const;

        //! Get user name
        const std::string&                      getUsername() const;

        //! Get nickname (login name)
        const std::string&                      getNickname() const;

        //! Get e-mail address
        const std::string&                      getEmail() const;

        //! Get date of registration
        const std::string&                      getRegistrationDate() const;

        //! Get last login time stamp as string
        const std::string&                      getLastLogin() const;

        //! Get total on-line time as string
        const std::string&                      getOnlineTime() const;

        //! Get user priviledges
        unsigned int                            getPriviledges() const;

        //! Get user description
        const std::string&                      getUserDescription() const;

        std::string                             _userName;

        std::string                             _nickName;

        std::string                             _email;

        std::string                             _registrationDate;

        std::string                             _lastLogin;

        std::string                             _onlineTime;

        std::string                             _userDescription;

        unsigned int                            _userID;

        unsigned int                            _priviledges;
};


//! User data associated with an account. The IDs are references to other tables
class UserData
{
    public:

                                                UserData();

                                                ~UserData();

        //! Get the user account ID
        unsigned int                            getUserId() const;

        //! Get the data ID
        unsigned int                            getDataId() const;

        //! Get inventory ID
        unsigned int                            getInventoryId() const;

        //! Get mailbox ID
        unsigned int                            getMailboxId() const;

        //! Get skills ID
        unsigned int                            getSkillsId() const;

        //! Account ID
        unsigned int                            _userID;

        //! User data ID
        unsigned int                            _dataID;

        //! Inventory ID
        unsigned int                            _inventoryID;

        //! Mailbox ID
        unsigned int                            _mailboxID;

        //! Skills ID
        unsigned int                            _skillsID;
};

} // namespace vrc

#endif  // _VRC_ACCOUNT_H_
