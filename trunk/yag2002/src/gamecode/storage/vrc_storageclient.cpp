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
 # storage interface used by a client for authentification and
 #  user data exchange.
 #
 #   date of creation:  09/27/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storageclient.h"
#include "vrc_storagenetworking.h"
#include "vrc_userinventory.h"

//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorageClient )


namespace vrc
{

StorageClient::StorageClient() :
 _userID( 0 ),
 _p_networking( NULL ),
 _p_userInventory( NULL )
{
}

StorageClient::~StorageClient()
{
}

void StorageClient::shutdown()
{
    log_info << "StorageClient: shutting down" << std::endl;

    yaf3d::NetworkDevice::get()->setAuthCallback( NULL );
    // destroy the singleton
    destroy();
}

void StorageClient::initialize() throw ( StorageClientException )
{
    log_info << "StorageClient: initializing" << std::endl;
    // register callback in network device for getting authentification result.
    yaf3d::NetworkDevice::get()->setAuthCallback( this );
}

void StorageClient::setNetworking( StorageNetworking* p_networking )
{
    _p_networking = p_networking;
}

void StorageClient::authentificationResult( unsigned int userID )
{ // this is only called when the client has successfully been authentified on server
    _userID = userID;
    // create the user inventory object
    assert ( ( _p_userInventory == NULL ) && "inventory object already exists!" );
    _p_userInventory = new UserInventory( userID );
}

bool StorageClient::requestAccountInfo( unsigned int userID, class AccountInfoResult* p_cb )
{
    if ( !_p_networking )
        return false;

    //! TODO: ...
    _p_networking->requestAccountInfo( userID, p_cb );

    return true;
}

unsigned int StorageClient::getUserID() const
{
    return _userID;
}

UserInventory* StorageClient::getUserInventory()
{
    return _p_userInventory;
}

} // namespace vrc
