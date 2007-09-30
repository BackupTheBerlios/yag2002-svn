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
 # storage interface used by server for authentification and
 #  user data exchange.
 #
 #   date of creation:  09/29/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_STORAGESERVER_H_
#define _VRC_STORAGESERVER_H_

#include <vrc_main.h>
#include "vrc_storagenetworking.h"

namespace vrc
{

//! Class declarations
class StorageNetworking;
namespace gameutils
{
    class VRCStateHandler;
}


//! Class for storage server related exceptions
class StorageServerException : public std::runtime_error
{
    public:
                                                    StorageServerException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~StorageServerException() throw() {}

                                                    StorageServerException( const StorageServerException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    StorageServerException();

        StorageServerException&                     operator = ( const StorageServerException& );
};


//! Storage server
class StorageServer : public yaf3d::Singleton< vrc::StorageServer >,  public yaf3d::CallbackAuthentification
{
    public:

        //! Process an authentification
        bool                                        authentify( const std::string& login, const std::string& passwd );

    protected:

                                                    StorageServer();

        virtual                                     ~StorageServer();

        //! Initialize the storage server
        void                                        initialize() throw ( StorageServerException );

        //! Shutdown the storage server
        void                                        shutdown();

        //! Override of CallbackAuthentification method for authentification when a client connects
        bool                                        authentify( const std::string& login, const std::string& passwd, unsigned int& userID );

        //! Networking for storage
        StorageNetworking*                          _p_networking;

    friend class StorageNetworking;
    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< vrc::StorageServer >;
};

} // namespace vrc

#endif // _VRC_STORAGESERVER_H_
