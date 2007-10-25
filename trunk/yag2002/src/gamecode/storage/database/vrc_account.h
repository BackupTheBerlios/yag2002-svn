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

        const std::string&                      getUsername() const;

        const std::string&                      getFirstname() const;

        const std::string&                      getLastname() const;

        const std::string&                      getNickname() const;

        const std::string&                      getEmail() const;

        const std::string&                      getLastLogin() const;

        const std::string&                      getOnlineTime() const;

        unsigned int                            getUserId() const;

        unsigned int                            getPriviledges() const;

        std::string                             _userName;

        std::string                             _nickName;

        std::string                             _email;

        std::string                             _lastLogin;

        std::string                             _onlineTime;

        unsigned int                            _userID;

        unsigned int                            _priviledges;
};

} // namespace vrc

#endif  // _VRC_ACCOUNT_H_
