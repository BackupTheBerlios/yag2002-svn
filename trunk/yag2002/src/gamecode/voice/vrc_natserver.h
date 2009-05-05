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
 # entity NAT server used for establishing peer2peer connections
 #  between voice clients
 #
 #   date of creation:  02/14/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_NATSERVER_H_
#define _VRC_NATSERVER_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_natcommon.h"

namespace vrc
{

#define ENTITY_NAME_NATSERVER    "NATServer"


//! NATServer Entity created only on server
class EnNATServer : public yaf3d::BaseEntity, public yaf3d::SessionNotifyCallback
{
    public:
                                                    EnNATServer();

        virtual                                     ~EnNATServer();

        //! Initialize
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

    protected:

        //! This entity needs no transformation.
        bool                                        isTransformable() const { return false; }

        //! Override of method for getting notification when a client leaves the network
        void                                        onSessionLeft( int sessionID );

        //! Class holding NAT client information, used for internal house-keeping
        class NATInfo
        {
            public:
                                                            NATInfo( RNReplicaNet::XPAddress address, RNReplicaNet::XPAddress localaddr, int fwdport ) : 
                                                             _mappedAddress( address ),
                                                             _localAddress( localaddr ),
                                                             _forwardedPort( fwdport )
                                                            {
                                                            }

                virtual                                     ~NATInfo()
                                                             {
                                                             }

                RNReplicaNet::XPAddress                     _mappedAddress;

                RNReplicaNet::XPAddress                     _localAddress;

                int                                         _forwardedPort;
        };

        //! Lookup for < sid, NATClient >, used on server
        std::map< int, NATInfo* >                   _sidNATInfoMap;

        //! Communication socket
        RNReplicaNet::XPSocketUrgent*               _p_socket;
};

//! Entity type definition used for type registry
class NATServerEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    NATServerEntityFactory() : 
                                                     yaf3d::BaseEntityFactory
                                                     ( 
                                                       ENTITY_NAME_NATSERVER, yaf3d::BaseEntityFactory::Server
                                                      )
                                                    {}

        virtual                                     ~NATServerEntityFactory() {}

        Macro_CreateEntity( EnNATServer );
};

}

#endif // _VRC_NATSERVER_H_
