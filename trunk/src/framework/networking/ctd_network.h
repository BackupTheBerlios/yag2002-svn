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


#include <ctd_baseentity.h>
#include <ctd_pluginmanager.h>

#include "ctd_network_defs.h"
#include "ctd_network_server.h"
#include "ctd_network_client.h"

#include <Source/PacketEnumerations.h>
#include <Source/RakNetworkFactory.h>
#include <Source/RakClientInterface.h>
#include <Source/RakServerInterface.h>
#include <Source/RakClient.h>
#include <Source/RakServer.h>
#include <Source/NetworkTypes.h>
#include "raknet_NetworkObject.h"
#include "raknet_Multiplayer.h"

#include <string>
#include <vector>

namespace CTD
{

class FrameworkImpl;
class NetworkServer;
class NetworkClient;

// this is used for an update throttle for very fast machines
#define CTD_CLIENT_UPDATE_PERIOD_THROTTLE   0.015f


// networking core
class NetworkDevice
{

    public:

        /**
        * Get functions for server / client cores
        */
        NetworkServer*                  GetNetworkServer() { return m_pkServer; }
        NetworkClient*                  GetNetworkClient() { return m_pkClient; }

        /**
        * Use this function to send a packet from client to server.
        *\param usNetworkID             Network ID, this id is used to identify the counterpart object on server and deliver him the message.
        *                                In normal case this is the network id of sending object ( entity ).
        *\param pcData                  The data to send
        *\param lLength                 Data length
        *\param ePriority               Send priority option ( see RakNet/PacketPriority.h )
        *\param eReliability            Send reliabiliy option ( see RakNet/PacketPriority.h )
        *\param cOrderingStream         Ordering number
        *\return                        true if sending was successfull ( but this does not mean that receiving was successfull, too )
        */
        bool                            SendClient( 
                                                    unsigned short      usNetworkID, 
                                                    char                *pcData, 
                                                    const long          lLength, 
                                                    PacketPriority      ePriority       = HIGH_PRIORITY, 
                                                    PacketReliability   eReliability    = RELIABLE_SEQUENCED, 
                                                    char                cOrderingStream = 0
                                                    );


        /**
        * Use this function to send a packet from server to one or more clients.
        *\param usNetworkID             Network ID, this id is used to identify the counterpart object on server and deliver him the message.
        *                                In normal case this is the network id of sending object ( entity ).
        *\param pcData                  The data to send
        *\param lLength                 Data length
        *\param ePriority               Send priority option ( see RakNet/PacketPriority.h )
        *\param eReliability            Send reliabiliy option ( see RakNet/PacketPriority.h )
        *\param cOrderingStream         Ordering number
        *\param kNodeId                 The node id of receipient
        *\param broadcast               Set to true for sending to broadcase
        *\param bSecured                Set to true to activate security layer
        */
        bool                            SendServer( 
                                                    unsigned short      usNetworkID, 
                                                    char                *pcData, 
                                                    const long          lLength, 
                                                    PacketPriority      ePriority, 
                                                    PacketReliability   eReliability, 
                                                    char                cOrderingStream, 
                                                    PlayerID            kNodeId, 
                                                    bool                broadcast   = false, 
                                                    bool                bSecured    = false 
                                                    );
        

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
        * Return the level name currently running on server. This function is used by a client after connecting the server.
        * \return                   Level name running on server.
        */
        std::string                     GetLevelName() { return GetStaticDataClient()->m_pcLevelName; }

        /**
        * This function is called to enable an entity for networking as server object.
        * \param   pkEntity        pointer to entity ( in normal case a this-pointer )
        * \return                  unique network id, use this to remove the entity later if needed
        */
        unsigned short                  AddServerObject( BaseEntity *pkEntity );

        /**
        * This function is called to insert the network objects into room manager
        */
        void                            ActivateServerObjects();
        
        /**
        * Call this to add a network client object, which is created after server/client connection has been established.
        * \param   pkEntity        pointer to entity ( in normal case a this-pointer )
        */
        void                            AddClientObject( BaseEntity *pkEntity );

        /**
        * This function is called to add and intitialize all network client objects so they get updated periodically.
        *  It causes a negotiation with server and creation of ghosts in all connected clients through the server.
        */
        void                            ActivateClientObjects();

        /**
        * This function must be called at end of network setup phase for clients.
        * It activates the client core to dispatch entity messages.
        */
        void                            ActivateClientNetworking() 
                                        { 
                
                                            assert( ( m_pkClient != NULL ) && " first initialize client core before activating" );
                                            m_pkClient->SetReady( true ); 

                                        }

        /**
        * Call this to remove a server object.
        * \param usNetworkID            Network id of object to be removed ( retrieved from function AddNetworkObject )
        */
        void                            RemoveServerObject( unsigned short usNetworkID );

        /**
        * Call this to request for creation of a remote client object on connected server. In other clients this client object will be represented by a ghost ( remote client ).
        * \param   pkActorkEntity  Client object entity, this entity must not be of type server object.
        * \param pkActorkEntity
        * \return                  true if request could be sent out to server.
        */
        bool                            CreateClientObject( BaseEntity *pkActorkEntity );

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
        * Causes requesting server for sending a list of existing clients.
        * \return                       true if successful.
        */
        bool                            GetRemoteClients();

        /**
        * Returns true for successfull connection
        * \return                       true if connection successfully established
        */
        bool                            EstablishConnection();

        /**
        * Returns the name of this network node.
        * \return                       Network node name ( usually the player name )
        */
        std::string                     GetNetworkNodeName() { return m_strNetworkNodeName; }

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

        //! Client list
        std::vector< ClientNode >       m_vstrClientList;

        //! Flag indicating that the connection is established
        bool                            m_bConnectionEstablished;

        //! Static data of server is ready
        bool                            m_bServerStaticDataReady;

        //! Static data of client is ready
        bool                            m_bClientStaticDataReady;


    friend  class FrameworkImplClient;
    friend  class FrameworkImplServer;
    friend  class FrameworkImpl;
    friend  class LevelLoader;

};

#include "ctd_network_inl.h"

} // namespace CTD

#endif //_CTD_NETWORK_H_
