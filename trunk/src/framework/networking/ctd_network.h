/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # neoengine, networking core
 #
 # this core uses RakNet and provides networking services
 #
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
 #  04/08/2004 boto       creation of CTDNetworking
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_NETWORK_H_
#define _CTD_NETWORK_H_

#include "ctd_network_defs.h"
#include <include/PacketEnumerations.h>
#include <include/RakServer.h>
#include <include/Multiplayer.h>
#include <include/DistributedNetworkObjectHeader.h>
#include <include/DistributedNetworkObjectManager.h>

#include <string>
#include <vector>

namespace CTD
{

class FrameworkImpl;
class NetworkServer;
class NetworkClient;

// this is used for an update throttle for very fast machines
#define CTD_CLIENT_UPDATE_PERIOD_THROTTLE   0.015f
// connection timeout
#define CTD_CLIENT_CONNECTION_TIMEOUT       10.0f

// networking core
class NetworkDevice
{

    public:

        /**
        * Get functions for server / client cores
        */
        NetworkServer*                  GetNetworkServer() { return m_pkServer; }

        NetworkClient*                  GetNetworkClient() { return m_pkClient; }


    protected:

        /**
        * Construction is only allowed to FrameworkImpl.
        */
                                        NetworkDevice( FrameworkImpl *pkFrameworkImpl );
        /**
        * Avoid the usage of copy constructor.
        */
                                        NetworkDevice( NetworkDevice &kNetWorkDevice );

        /**
        * Destruction is only allowed to FrameworkImpl.
        */
                                        ~NetworkDevice();

        /**
        * Shutdown the network device.
        */
        void                            Shutdown();

        /**
        * Set static data for client. Use this function before calling the function Initialize.
        * \param   strNodeName      This is the name of your network node, in normal case this will be the player name.
        */
        void                            SetupClientStaticData( const std::string &strNodeName );

        /**
        * Set static data for server. Use this function before calling the function Initialize.
        * \param   strNodeName      This is the name of your network node, in normal case this will be the player name.
        * \param   strLevelName     The level name which the server has to run. This name is used in connecting clients
        *                             to start the currently running level on server.
        */
        void                            SetupServerStaticData( const std::string &strNodeName, const std::string &strLevelName );

        /**
        * Returns true for successfull connection
        * \return                       true if connection successfully established
        */
        bool                            EstablishConnection();

        /**
        * Return the level name currently running on server. This function is used by a client after connecting the server.
        * \return                   Level name running on server.
        */
        std::string                     GetLevelName() { return GetStaticDataClient()->m_pcLevelName; }
  
        /**
        * Returns a pointer to static data for server, it contains information about running session, etc.
        * Call this after established connection to get e.g. the level name running on connected server.
        */
        tStaticDataServer*              GetStaticDataServer() { return &m_kStaticDataServer; }

        /**
        * Returns a pointer to static data for client, it contains information about node name, etc.
        */
        tStaticDataClient*              GetStaticDataClient() { return &m_kStaticDataClient; }

        /**
        * Initialize server ( if bServer is true ) or client ( if bServer is false )
        * \param bServer                true for server, false for client
        */
        void                            Initialize( bool bServer );

        /**
        * Start server.
        * \return     true if successfull
        */
        bool                            StartServer( unsigned short usMaxNumberOfClients = 16, bool bHightPriority = false, unsigned short usServerPort = 2002 );
        
        /**
        * Start Client.
        * \return                       true if connection to server was successful
        */
        bool                            ConnectClient( char *pcHost, unsigned short usServerPort = 2002, unsigned short usClientPort = 2004, bool bHighPriority = false );

        /**
        * Paket processing function for server called in main loop in every step.
        * \param fDeltaTime             Passes time since last update
        */
        void                            UpdateServer( float fDeltaTime );

        /**
        * Paket processing function for client called in main loop in every step.
        * \param fDeltaTime             Passes time since last update
        */
        void                            UpdateClient( float fDeltaTime );

        //-------------------------------------------------------//

        //! Framework implementation instance
        FrameworkImpl                   *m_pkFrameworkImpl;

        //! Server's static date
        tStaticDataServer               m_kStaticDataServer;
        
        //! Client's static data
        tStaticDataClient               m_kStaticDataClient;
        
        //! Network node name, usually the player name
        std::string                     m_strNetworkNodeName;

        //! Server instance, created in server mode
        NetworkServer                   *m_pkServer;
        
        //! Client instance, created in client mode
        NetworkClient                   *m_pkClient;

        //! These are used to throttle very fast machines avoiding throughput problems
        float                           m_fClientMinUpdatePeriod;
        
        //! Time passed since last message dispatch update
        float                           m_fPassedUpdateTime;

        //! Static data of server is ready
        bool                            m_bServerStaticDataReady;

        //! Static data of client is ready
        bool                            m_bClientStaticDataReady;


    friend  class FrameworkImplClient;
    friend  class FrameworkImplServer;
    friend  class FrameworkImpl;

};

} // namespace CTD

#endif //_CTD_NETWORK_H_
