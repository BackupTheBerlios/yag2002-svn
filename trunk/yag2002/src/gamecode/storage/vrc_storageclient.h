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

#ifndef _VRC_STORAGECLIENT_H_
#define _VRC_STORAGECLIENT_H_

#include <vrc_main.h>
#include "vrc_storagenetworking.h"

namespace vrc
{

//! Class declarations
class UserInventory;
class StorageNetworking;
namespace gameutils
{
    class VRCStateHandler;
}


//! Class for storage client related exceptions
class StorageClientException : public std::runtime_error
{
    public:
                                                    StorageClientException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~StorageClientException() throw() {}

                                                    StorageClientException( const StorageClientException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    StorageClientException();

        StorageClientException&                     operator = ( const StorageClientException& );
};


//! Storage client
class StorageClient : public yaf3d::Singleton< vrc::StorageClient >, public yaf3d::CallbackAuthentification, public yaf3d::SessionNotifyCallback
{
    public:

        //! TODO: we need a more general callback mechanism used by all kinds of requests!
        class AccountInfoResult : public StorageNetworking::CallbackAccountInfoResult
        {
            public:

                virtual void                        accountInfoResult( tAccountInfoData& info ) = 0;
        };

        //! Request the server for account info
        bool                                        requestAccountInfo( unsigned int userID, class AccountInfoResult* p_cb );

        //! Get the unique user account ID
        unsigned int                                getUserID() const;

        //! Get the inventory of local player
        UserInventory*                              getUserInventory();

    protected:

                                                    StorageClient();

        virtual                                     ~StorageClient();

        //! Initialize the storage server
        void                                        initialize() throw ( StorageClientException );

        //! Shutdown the storage client
        void                                        shutdown();

        //! Set the networking object used by storage networking on creation.
        void                                        setNetworking( StorageNetworking* p_networking );

        //! Override of networking's CallbackAuthentification method for getting the user ID when successfully authentified. userID is the unique user ID.
        void                                        authentificationResult( unsigned int userID );

        //! Unique user ID set when successfully authentified
        unsigned int                                _userID;

        //! Networking for storage
        StorageNetworking*                          _p_networking;

        //! Inventory of local player
        UserInventory*                              _p_userInventory;

    friend class StorageNetworking;
    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< vrc::StorageClient >;
};

} // namespace vrc

#endif // _VRC_STORAGECLIENT_H_
