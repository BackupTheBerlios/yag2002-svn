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
#include "ctd_membernw.h"
#include "ctd_physics.h"
#include "ctd_gui.h"
#include "ctd_cal3danim.h"

namespace CTD_IPluginChat {


// this is the printf object
extern CTDPrintf    g_CTDPrintf;
extern CTDPrintf    g_CTDPrintfNwStats;

// chat member entity
class CTDChatMember : public CTD::BaseEntity
{

    public:

                                                    CTDChatMember();

                                                    ~CTDChatMember();

        //******************************* override some functions **********************************//

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
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        bool                                        Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

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

        // these methods are used for setting up remote clients
        //------------------------------------------------------------------------------------------//
        void                                        SetPlayerNetworking( CTD_MemberNw* pkNw ) { m_pkPlayerNetworking = pkNw; }

        // get player name, is used for sending initialization data to remote clients
        std::string                                 GetPlayerName() { return m_strPlayerName; }

        // set player character's animation config file name
        void                                        SetAnimConfig( const std::string& strAnimConfig ) { m_strCal3dConfigFile = strAnimConfig; }

        // set name for remote clients
        void                                        SetPlayerName( const std::string& strName ) { m_strPlayerName = strName; }

        // post new chat text into gui
        void                                        PostChatText( const std::string& strText );
        //------------------------------------------------------------------------------------------//

        // entity parameters
        std::string                                 m_strCal3dConfigFile;

        float                                       m_fPositionalSpeed;
        float                                       m_fAngularSpeed;
        float                                       m_fMouseSensitivity;

        NeoEngine::Vector3d                         m_kInitPosition;
        NeoEngine::Vector3d                         m_kInitRotation;
        NeoEngine::Vector3d                         m_kDimensions;

    protected:

        /**
        * Update method for object on server called by UpdateEntity
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateServerObject( float fDeltaTime );

        /**
        * Update method for object on client called by UpdateEntity
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateClientObject( float fDeltaTime );

        /**
        * Update method for standalone objects called by UpdateEntity
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateStandaloneObject( float fDeltaTime );

        // process inputs
        void                                        ProcessInput( float fDeltaTime );

        // physics for moving the body
        WalkPhysics                                 m_kPhysics;

        std::string                                 m_strPlayerName;

        // instance of gui entity
        CTDGui                                      *m_pkEntityGui;

        // a local pointer to camera object
        NeoEngine::Camera                           *m_pkCamera;
        
        // networking interface
        CTD_MemberNw                               *m_pkPlayerNetworking;

        // animation module
        CTDCal3DAnim                                *m_pkAnimMgr;

        // input processing specific
        float                                       m_fPositionalAcceleration;
        float                                       m_fAngularAcceleration;

        // animation commands
        unsigned int                                m_uiCmdFlag, m_uiLastCmdFlag;

        bool                                        m_bMoved;
        bool                                        m_bRotated;
        bool                                        m_bMouseRotated;
        float                                       m_fMouseDebounceTimer;

        NeoEngine::Vector3d                         m_kMove;
        NeoEngine::Vector3d                         m_kRotate;

        NeoEngine::Vector3d                         m_kVelocity;

        NeoEngine::Vector3d                         m_kCurrentPosition;

        NeoEngine::Vector3d                         m_kCurrentRotation;
        NeoEngine::Quaternion                       m_kCurrentRotationQ;

        // networking specific
        CTD::GameMode                               m_eGameMode;
        CTD::NetworkDevice                          *m_pkNetworkDevice;

        NeoEngine::Room                             *m_pkRoom;


};


// descriptor for chat member entity
class CTDChatMemberDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDChatMemberDesc() { };
                                                    ~CTDChatMemberDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_ChatMember ) }

        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDChatMember; }


};

// global instance of chat member entity
extern CTDChatMemberDesc g_pkChatMemberEntity_desc;

}

#endif //_CTD_MEMBER_H_
