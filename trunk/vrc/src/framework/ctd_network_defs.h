/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  04/08/2004 boto       creation of common nw defines and structures
 #
 ################################################################*/

#ifndef _CTD_NETWORK_DEFS_H_
#define _CTD_NETWORK_DEFS_H_

namespace CTD
{

// structure for requesting server info during pre-connection ( client side )
#define CTD_NW_PRECON_DATA_CLIENT      0x01
struct PreconnectDataClient
{
    unsigned char  _typeId;  // set this to CTD_NW_PRECON_DATA_CLIENT
};


// structure for sending server info during pre-connection ( server side )
#define CTD_NW_PRECON_DATA_SERVER      0x02
struct PreconnectDataServer
{
    unsigned char  _typeId;  // set this to CTD_NW_PRECON_DATA_SERVER    
    char           _p_levelName[ 64 ];
    char           _p_serverName[ 64 ];
};

} // namespace CTD

#endif //_CTD_NETWORK_DEFS_H_
