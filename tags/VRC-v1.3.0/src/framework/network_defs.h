/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # networking defines and structs
 #
 #   date of creation:  04/08/2004
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  04/08/2004 boto       creation of common nw defines and structures
 #
 ################################################################*/

#ifndef _NETWORK_DEFS_H_
#define _NETWORK_DEFS_H_

namespace yaf3d
{

//! Connection states
enum ConnectionState
{
    eConnecting = 0x20,
    eLogin,
    eLoginResult,
    eRegister,
    eRegistrationResult
};

//! Structure for requesting server info during pre-connection ( client side )
#define YAF3DNW_PRECON_DATA_CLIENT      0x01
struct PreconnectDataClient
{
    unsigned char  _typeId;           // set this to YAF3DNW_PRECON_DATA_CLIENT
    unsigned char  _state;            // connection state, one of ConnectionState enums
    char           _p_login[ 64 ];    // login name
    char           _p_passwd[ 64 ];   // password
    char           _p_name[ 64 ];     // real name ( used for registration )
    char           _p_email[ 64 ];    // e-mail address ( used for registration )
};


// structure for sending server info during pre-connection ( server side )
#define YAF3DNW_PRECON_DATA_SERVER      0x02
struct PreconnectDataServer
{
    unsigned char  _typeId;             // set this to YAF3DNW_PRECON_DATA_SERVER    
    unsigned char  _state;              // connection state, one of ConnectionState enums
    char           _p_levelName[ 64 ];
    char           _p_serverName[ 64 ];
    bool           _needAuthentification;
    bool           _accessGranted;      // used for authentification and registration result
    unsigned int   _userID;
    unsigned int   _protocolVersion;
};

} // namespace yaf3d

#endif //_NETWORK_DEFS_H_
