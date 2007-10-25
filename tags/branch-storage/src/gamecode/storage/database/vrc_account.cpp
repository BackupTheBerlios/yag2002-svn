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

#include <vrc_main.h>
#include "vrc_account.h"

namespace vrc
{

//! Implementation of class UserAccount
UserAccount::UserAccount() :
 _userID( static_cast< unsigned int >( -1 ) ),
 _priviledges( 0 )
{
}

UserAccount::~UserAccount()
{
}

const std::string& UserAccount::getUsername() const
{
    return _userName;
}

const std::string& UserAccount::getNickname() const
{
    return _nickName;
}

const std::string& UserAccount::getEmail() const
{
    return _email;
}

const std::string& UserAccount::getLastLogin() const
{
    return _lastLogin;
}

const std::string& UserAccount::getOnlineTime() const
{
    return _onlineTime;
}

unsigned int UserAccount::getUserId() const
{
    return _userID;
}

unsigned int UserAccount::getPriviledges() const
{
    return _priviledges;
}


} // namespace vrc
