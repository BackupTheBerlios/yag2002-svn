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
 # neoengine, chat member class
 #
 # this class implements a chat member
 #
 #
 #   date of creation:  08/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  08/13/2004 boto       creation of CTDChatMember
 #
 ################################################################*/

#ifndef _CTD_MEMBER_H_
#define _CTD_MEMBER_H_


#include "base.h"
#include "../share/ctd_descriptor.h"
#include "../share/ctd_baseentity.h"
#include "../share/ctd_printf.h"
#include "../share/ctd_chat_defs.h"
#include "ctd_interpolator.h"
#include "ctd_networkingutils.h"
#include "ctd_physics.h"


namespace CTD_IPluginChat {


// this is the printf object
extern CTDPrintf    g_CTDPrintf;
extern CTDPrintf    g_CTDPrintfNwStats;


// networking related constants
//-------------------------------------------------------//
// update cycle for position and rotation( sec )
#define CTD_ChatMember_POS_UPDATE_PERIODE               0.200f
#define CTD_ChatMember_ROT_UPDATE_PERIODE               0.2000f
//-------------------------------------------------------//

// chat member entity
template< class InterpolatorPosT, class InterpolatorRotT >
class CTDChatMember : public CTD::BaseEntity
{

    public:

                                                    CTDChatMember();

                                                    ~CTDChatMember();

        //******************************* override some functions **********************************//

        /**
        * Get network type of this entity ( see BaseEntity for more details ).
        * Return always the type using BaseEntity::GetNetworkingType() and set
        * your desired type in ctor ( as a client object can get remote client object through the framework ).
        */
        CTD::tCTDNetworkingType                     GetNetworkingType() 
                                                    { 
                                                        return BaseEntity::GetNetworkingType();
                                                    }

        /**
        * Initializing function
        */
        void                                        Initialize();

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
         void                                       PostInitialize();

        /**
        * Update object
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateEntity( float fDeltaTime );

        /**
        * Update method for client objects called by UpdateEntity
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateClientObject( float fDeltaTime );

        /**
        * Update method for remote client objects called by UpdateEntity
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateRemoteClientObject( float fDeltaTime );

        /**
        * Update method for standalone objects called by UpdateEntity
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateStandaloneObject( float fDeltaTime );

        /**
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        bool                                        Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

        /**
        * Messaging function for incomming network pakets. This function is called by framework.
        * \param   iMsgId                           Message ID
        * \param   pMsgStruct                       Message specific data structure
        */
        void                                        NetworkMessage( int iMsgId, void *pMsgStruct );

        /**
        * Messaging function.
        * \param  iMsgId                            Message ID
        * \param  pMsgStruct                        Message specific data structure
        * \return                                   This return value is message specific.
        */
        int                                         Message( int iMsgId, void *pMsgStruct );

        //*******************************************************************************************//

        int                                         ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

        // entity parameters
        float                                       m_fPositionalSpeed;
        float                                       m_fAngularSpeed;
        NeoEngine::Vector3d                         m_kInitPosition;
        NeoEngine::Vector3d                         m_kInitRotation;
        NeoEngine::Vector3d                         m_kDimensions;
        NeoEngine::MeshEntity                       *m_pkMesh;

    protected:

        // this funtion compiles initialization data to send to new created remote clients on other machines
        void                                        SendInitDataToRemoteClients( tCTD_NM_InitRemoteChatMember *pkInitData );

        // network processing message functions
        void                                        ProcessEntityMessage( void *pkMsgStruct );

        void                                        ProcessSystemMessage( void *pkMsgStruct );

        // process inputs
        void                                        ProcessInput( float fDeltaTime );

        // physics for moving the body
        WalkPhysics                                 m_kPhysics;

        // position interpolator object
        InterpolatorPosT                            m_kPositionInterpolator;

        // rotation interpolator object
        InterpolatorRotT                            m_kRotationInterpolator;

        std::string                                 m_strPlayerName;

        // instance of gui entity
        CTD::BaseEntity                             *m_pkEntityGui;

        // a local pointer to camera object
        NeoEngine::Camera                           *m_pkCamera;

        CTD::AvarageUpdatePeriod< 10 >              m_kAvaragePositionUpdatePeriod;
        NeoEngine::Vector3d                         m_kActualPosition;
        NeoEngine::Vector3d                         m_kInterpolatedPosition;
        NeoEngine::Vector3d                         m_kLastPosition;

        CTD::AvarageUpdatePeriod< 10 >              m_kAvarageRotationUpdatePeriod;
        NeoEngine::Vector3d                         m_kActualRotation;
        NeoEngine::Quaternion                       m_kActualRotationQ;
        NeoEngine::Vector3d                         m_kInterpolatedRotation;
        NeoEngine::Vector3d                         m_kLastRotation;

        float                                       m_fPassedPosSendTime;
        float                                       m_fPassedRotSendTime;

        // input processing specific
        float                                       m_fPositionalAcceleration;
        float                                       m_fAngularAcceleration;

        bool                                        m_bMoved;
        enum { stateIdleMove, stateMoving }         m_eMove;
        bool                                        m_bRotated;
        enum { stateIdleRotate, stateRotating }     m_eRotate;

        //! Translation states
        enum _translationState { 
            stateTranslationIdle = 0x0, 
            stateTranslationStartMoving, 
            stateTranslationMoving, 
            stateTranslationStopMoving }            m_eTranslationState;

        //! Rotation states
        enum _rotationState { 
            stateRotationIdle = 0x0, 
            stateRotationStartMoving, 
            stateRotationMoving, 
            stateRotationStopMoving }               m_eRotationState;

        NeoEngine::Vector3d                         m_kMove;
        NeoEngine::Vector3d                         m_kRotate;

        CTD::BaseEntity                             *m_pkChatMember;

        // networking specific
        CTD::GameMode                               m_eGameMode;
        CTD::NetworkDevice                          *m_pkNetworkDevice;
        CTD::NetworkServer                          *m_pkNetworkServer;
        CTD::NetworkClient                          *m_pkNetworkClient;
        NeoEngine::Room                             *m_pkRoom;
};

typedef CTDChatMember< LinearInterpolator, LinearInterpolator > CTDChatMemberLinearPosLinearRot;

// descriptor for chat member entity
class CTDChatMemberDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDChatMemberDesc() { };
                                                    ~CTDChatMemberDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_ChatMember ) }

        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDChatMemberLinearPosLinearRot; }


};

// global instance of chat member entity is used in dll interface
extern CTDChatMemberDesc g_pkChatMemberEntity_desc;

}

#endif //_CTD_MEMBER_H_
