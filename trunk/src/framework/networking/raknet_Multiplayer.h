// ----------------------------------------------------------------------
// This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
// Licensed under the "RakNet" brand by Rakkarsoft LLC and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com
// Multiplayer.h
// Written by Kevin Jenkins (rakkar@rakkar.org) January 24, 2003
// ----------------------------------------------------------------------

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H

struct Packet;

template <class InterfaceType>
class Multiplayer
{
public:
    Multiplayer();
    virtual ~Multiplayer();

    // -----------------------------------------------------------------
    // CALL THIS EVERY FRAME
    // Reads any packets from the network, handles the native messages, and sends user defined messages to ProcessUnhandledPacket
    // -----------------------------------------------------------------
    virtual void ProcessPackets(InterfaceType *interfaceType);

    // -----------------------------------------------------------------
    //  *** YOU PROBABLY WANT TO OVERRIDE THESE FOR YOUR OWN APP ***
    // -----------------------------------------------------------------
    virtual void ReceiveRemoteDisconnectionNotification(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveRemoteConnectionLost(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveRemoteNewIncomingConnection(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveConnectionRequestAccepted(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveNewIncomingConnection(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveConnectionResumption(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveNoFreeIncomingConnections(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveDisconnectionNotification(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveConnectionLost(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceivedStaticData(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveInvalidPassword(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveModifiedPacket(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveRemotePortRefused(Packet *packet,InterfaceType *interfaceType);
    virtual void ReceiveVoicePacket(Packet *packet,InterfaceType *interfaceType);
    // -----------------------------------------------------------------

    // -----------------------------------------------------------------
    // Given a packet, returns the packet identifier
    // -----------------------------------------------------------------
    static unsigned char GetPacketIdentifier(Packet *packet);

protected:
    // -----------------------------------------------------------------
    // *** YOU MUST OVERRIDE THIS FOR YOUR OWN APP ***
    // -----------------------------------------------------------------
    // All user defined packets are sent to this function.
    virtual void ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &pkNodeID, InterfaceType *interfaceType);
    // -----------------------------------------------------------------

private:
};

#include <Source/PacketEnumerations.h>
#include <Source/NetworkTypes.h>
#include <assert.h>
#include <stdio.h>

#ifdef _DEBUG
#include <memory.h>
#endif

template <class InterfaceType>
Multiplayer<InterfaceType>::Multiplayer()
{
}

template <class InterfaceType>
Multiplayer<InterfaceType>::~Multiplayer()
{
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ProcessPackets(InterfaceType *interfaceType)
{
    Packet* p;
    unsigned char packetIdentifier;

    p = interfaceType->Receive();

    char *pcData; // this is used for calling unhandled packets

    while (p)
    {
        if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
            if (p->length > sizeof(unsigned char) + sizeof(unsigned long)) {

                packetIdentifier = (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
                pcData           = ( char* )p->data;

                pcData += ( sizeof( unsigned char ) + sizeof( unsigned long ) );

            } else {

                break;

            }

        } else {

            packetIdentifier = (unsigned char) p->data[0];
            pcData           = ( char* )p->data;

        }


        // Check if this is a native packet
        switch (packetIdentifier)
        {
        case ID_REMOTE_DISCONNECTION_NOTIFICATION:
            ReceiveRemoteDisconnectionNotification(p,interfaceType);
        break;
        case ID_REMOTE_CONNECTION_LOST:
            ReceiveRemoteConnectionLost(p,interfaceType);
        break;
        case ID_REMOTE_NEW_INCOMING_CONNECTION:
            ReceiveRemoteNewIncomingConnection(p,interfaceType);
        break;
        case ID_CONNECTION_REQUEST_ACCEPTED:
            ReceiveConnectionRequestAccepted(p,interfaceType);
            break;
        case ID_NEW_INCOMING_CONNECTION:
            ReceiveNewIncomingConnection(p,interfaceType);
            break;
        case ID_CONNECTION_RESUMPTION:
            ReceiveConnectionResumption(p,interfaceType);
            break;
        case ID_NO_FREE_INCOMING_CONNECTIONS:
            ReceiveNoFreeIncomingConnections(p,interfaceType);
            break;
        case ID_DISCONNECTION_NOTIFICATION:
            ReceiveDisconnectionNotification(p,interfaceType);
            break;
        case ID_CONNECTION_LOST:
            ReceiveConnectionLost(p,interfaceType);
            break;
        case ID_RECEIVED_STATIC_DATA:
            ReceivedStaticData(p,interfaceType);
            break;
        case ID_INVALID_PASSWORD:
            ReceiveInvalidPassword(p,interfaceType);
            break;
        case ID_MODIFIED_PACKET:
            ReceiveModifiedPacket(p,interfaceType);
            break;
        case ID_REMOTE_PORT_REFUSED:
            ReceiveRemotePortRefused(p,interfaceType);
            break;
        case ID_VOICE_PACKET:
            ReceiveVoicePacket(p,interfaceType);
            break;

        default:
            // If not a native packet send it to ProcessUnhandledPacket which should have been written by the user
            ProcessUnhandledPacket( pcData, packetIdentifier, p->playerId, interfaceType );
            break;
        }

        interfaceType->DeallocatePacket(p);

        p = interfaceType->Receive();
    }
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &pkNodeID, InterfaceType *interfaceType)
{
    // This class should have been overrided to handle user defined packets
#ifdef _DEBUG
    // Uncomment this to show output as integers
    /*
    int i;
    static unsigned packetNumber=0;
    // Raw output (nonstring)
    printf("Multiplayer::ProcessUnhandledPacket (%i) (%i): ", packetNumber++, p->length);
    for (i=0; i < p->length; i++)
    printf("%i ",p->data[i]);
    printf("\n");
    */

    // Uncomment this to show output as a string
    /*
    // Raw output (string)
    printf("%s\n", p->data);
    */
#endif
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteDisconnectionNotification(Packet *packet,InterfaceType *interfaceType)
{
    // Another system has disconnected
#ifdef _DEBUG
    printf("ID_REMOTE_DISCONNECTION_NOTIFICATION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_REMOTE_DISCONNECTION_NOTIFICATION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteConnectionLost(Packet *packet,InterfaceType *interfaceType)
{
    // Another system has been dropped by the server
#ifdef _DEBUG
    printf("ID_REMOTE_CONNECTION_LOST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_REMOTE_CONNECTION_LOST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemoteNewIncomingConnection(Packet *packet,InterfaceType *interfaceType)
{
    // Another system has connected
#ifdef _DEBUG
    printf("ID_REMOTE_NEW_INCOMING_CONNECTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_REMOTE_NEW_INCOMING_CONNECTION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionRequestAccepted(Packet *packet, InterfaceType *interfaceType)
{
    // Our request to connect to another system has been accepted
#ifdef _DEBUG
    printf("ID_CONNECTION_REQUEST_ACCEPTED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_CONNECTION_REQUEST_ACCEPTED,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveNewIncomingConnection(Packet *packet, InterfaceType *interfaceType)
{
    // Another system has requested to connect to us, which we have accepted
#ifdef _DEBUG
    printf("ID_NEW_INCOMING_CONNECTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_NEW_INCOMING_CONNECTION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionResumption(Packet *packet, InterfaceType *interfaceType)
{
    // Another system has requested to connect to us, which we have accepted
#ifdef _DEBUG
    printf("ID_CONNECTION_RESUMPTION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_CONNECTION_RESUMPTION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveNoFreeIncomingConnections(Packet *packet, InterfaceType *interfaceType)
{
    // The system we connected to has no free slots to connect to
    // Set free slots by calling SetMaximumIncomingConnections
#ifdef _DEBUG
    printf("ID_NO_FREE_INCOMING_CONNECTIONS from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_NO_FREE_INCOMING_CONNECTIONS,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveDisconnectionNotification(Packet *packet, InterfaceType *interfaceType)
{
    // A remote system terminated the connection.  packet->playerId specifies which remote system
#ifdef _DEBUG
    printf("ID_DISCONNECTION_NOTIFICATION from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_DISCONNECTION_NOTIFICATION,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveConnectionLost(Packet *packet, InterfaceType *interfaceType)
{
    // The network cannot send reliable packets so has terminated the connection. packet->playerId specifies which remote system
#ifdef _DEBUG
    printf("ID_CONNECTION_LOST from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_CONNECTION_LOST,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceivedStaticData(Packet *packet, InterfaceType *interfaceType)
{
    // Another system has just sent their static data to us (which we recorded automatically)
#ifdef _DEBUG
    printf("ID_RECEIVED_STATIC_DATA from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_RECEIVED_STATIC_DATA,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveInvalidPassword(Packet *packet, InterfaceType *interfaceType)
{
    // Our connection to another system was refused because the passwords do not match
#ifdef _DEBUG
    printf("ID_INVALID_PASSWORD from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_INVALID_PASSWORD,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveModifiedPacket(Packet *packet, InterfaceType *interfaceType)
{
    // The network layer has detected packet tampering
    // This does NOT automatically close the connection
#ifdef _DEBUG
    printf("ID_MODIFIED_PACKET from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_MODIFIED_PACKET,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveRemotePortRefused(Packet *packet, InterfaceType *interfaceType)
{
    // The remote system has responded with ICMP_PORT_UNREACHABLE
#ifdef _DEBUG
    printf("ID_REMOTE_PORT_REFUSED from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_REMOTE_PORT_REFUSED,interfaceType);
}

template <class InterfaceType>
void Multiplayer<InterfaceType>::ReceiveVoicePacket(Packet *packet, InterfaceType *interfaceType)
{
    // We got a voice packet
#ifdef _DEBUG
    printf("ID_VOICE_PACKET from PlayerID:%u:%u on %p.\n",packet->playerId.binaryAddress, packet->playerId.port, interfaceType);
#endif
//  ProcessUnhandledPacket(packet, ID_VOICE_PACKET,interfaceType);
}

#endif


