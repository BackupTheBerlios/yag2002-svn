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
 # neoengine, base class for all entities in plugins
 #
 #   date of creation:  12/01/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/01/2003 boto       creation of BaseEntity
 #
 #  03/30/2004 boto       added PreRender function to base entity
 #
 #  04/09/2004 boto       added network id and networking type
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/


#ifndef _CTD_BASEENTITY_H_
#define _CTD_BASEENTITY_H_

#include "ctd_plugin_defs.h"
#include "ctd_descriptor.h"
#include <neoengine/scenenode.h>
#include <neoengine/sceneentity.h>
#include <neoengine/submesh.h>
#include <neoengine/aabb.h>
#include <neoengine/collision.h>

#include <string>


namespace CTD 
{

class ParameterDescriptor;

// networking types

//  - none means no networking needed.

//  - server objects reside only on server and distribute their state to all connected clients
//     example: a day/night simulator would reside on the server.

//  - client objects are controlled by one of the connected clients and distribute their state 
//     to all other clients ( to their ghosts ) via server. 
//     example: a player is a client object.

// - remote client are ghosts on local machine
//     example: other players i see on my machine are represented by remote client objects

typedef enum  { stateNONE, stateSERVEROBJECT, stateCLIENTOBJECT, stateREMOTE_CLIENTOBJECT } tCTDNetworkingType;


// father class of all plugin entities, this class is derived from scene entity and add messaging and param desctiption functions
class BaseEntity : public NeoEngine::SceneNode
{

    public:

                                    BaseEntity() : SceneNode(), m_usNetworkID( 0 ), m_eNetworkingType( stateNONE ) {} 

        virtual                     ~BaseEntity() {}

        /**
        * Set instance name. Instance name is used to find and attach entities to eachother.
        * If an entity is not attachable it does not need to set instance name.
        * \param  strInstName       Instance name
        */
        void                        SetInstanceName( const std::string & strInstName ) { m_strInstanceName = strInstName; }

        /**
        * Get instance name. Instance name is used to find and attach entities to eachother.
        * \return                  Instance name
        */
        std::string                 GetInstanceName() { return m_strInstanceName; } 

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        virtual void                Initialize() {};

        /**
        * This funtion is called before room nodes' rendering happens. Use this to implement effects such as env mapping, morror, etc.
        * You have to call framework function QueuePreRenderEntity in every update phase of your entity to activate the PreRender call.
        */
        virtual void                PreRender() {};

        /**
        * This funtion is called during 2d rendering phase.
        * You have to call framework function Register2DRendering to get this function called by framework.
        */
        virtual void                Render2D() {};

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        virtual void                PostInitialize() {};

        /**
        * Update entity
        */
        virtual void                UpdateEntity( float fDeltaTime ) {};

        /**
        * Messaging function for incomming network pakets. This function is called by framework.
        * \param   iMsgId           Message ID
        * \param   pMsgStruct       Message specific data structure
        */
        virtual void                NetworkMessage( int iMsgId, void *pMsgStruct ) {};

        /**
        * Messaging function for intercommunication between entities.
        * \param   iMsgId           Message ID
        * \param   pMsgStruct       Message specific data structure
        * \return                   This return value is message specific.
        */
        virtual int                 Message( int iMsgId, void *pMsgStruct ) { return 0; }

        /**
        * Function for parameter description. This is used to set and retrieve entity parameters.
        * \param   iParamIndex      Parameter index
        * \param   pkDesc           Parameter descriptor to fill by this function.
        * \return                   If pkDesc is NULL then the count of parameters must be retrieved.
        *                           Otherwise -1 when a requested parameter does not match to one of entity parameters.
        */
        virtual int                 ParameterDescription( int iPramIndex, ParameterDescriptor *pkDesc ) { return -1; }

        /**
        * Set and Get functions for network ID
        */
        void                        SetNetworkID( unsigned short usNetworkID ) { m_usNetworkID = usNetworkID; }

        unsigned short              GetNetworkID() { return m_usNetworkID; }

        /**
        * This function is called uppon entity creation. 
        *  A return value of stateNONE disables networking for entity.
        *  A return value of stateSERVEROBJECT enables networking for entity as server object.
        *  A return value of stateCLIENTOBJECT enables networking for entity as client object ( e.g. player is a client object ).
        *  A return value of stateREMOTE_CLIENTOBJECT is set by framework and indicates the entity as remote client object ( e.g. a remote player is a remote client object ).
        *   as a client object ( client object ) which must be requested and acknowledged by the server during creation. It furthermore is represented
        *   by a ghost on other clients. A player is a client object.
        */
        virtual tCTDNetworkingType  GetNetworkingType() { return m_eNetworkingType; }
        
        /**
        * Set function for networking type
        */
        void                        SetNetworkingType( tCTDNetworkingType eType ) { m_eNetworkingType = eType; }

    private:

        /** 
        * Avoid usage of copy constructor
        */
                                    BaseEntity( BaseEntity& );

        /** 
        * Avoid usage of assignment operator
        */
        BaseEntity&                 operator = ( BaseEntity& );

        std::string                 m_strInstanceName;

        unsigned short              m_usNetworkID;

        tCTDNetworkingType          m_eNetworkingType;


};


}; // namespace CTD

#endif //_CTD_BASEENTITY_H_
