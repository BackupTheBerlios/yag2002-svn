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
 # neoengine, menu camera
 #
 # this class implements the camera control for 3d menu
 #
 #
 #   date of creation:  05/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/13/2004 boto       creation of CTDMenuCamera
 #
 ################################################################*/

#ifndef _CTD_MENUCAM_H_
#define _CTD_MENUCAM_H_


#include "base.h"
#include <neoengine/light.h>
#include <neoengine/skeletonanimator.h>
#include <neoengine/animatednode.h>
#include <neochunkio/nodeanimation.h>
#include <neochunkio/scenenode.h>
#include <neochunkio/stdstring.h>
#include <neochunkio/float.h>
#include <neochunkio/integer.h>

#include <ctd_printf.h>

#include <string>

namespace CTD_IPluginMenu
{

// this is the printf object
extern CTDPrintf    g_CTDPrintf;


// name of this entity
#define CTD_ENTITY_NAME_MenuCamera  "MenuCamera"

class CTDMenuCamera : public CTD::BaseEntity
{

    public:

                                                    CTDMenuCamera();
                                                    ~CTDMenuCamera();

        //******************************* override some functions **********************************//

        /**
        * Initializing function
        */
        void                                        Initialize();

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

        // entity parameters
        NeoEngine::Vector3d                         m_kPosition;
        NeoEngine::Vector3d                         m_kRotation;
        NeoEngine::Vector3d                         m_kLightColor;
        std::string                                 m_strEntranceAnim;

        // camera settings
        float                                       m_fFOVAngle;
        float                                       m_fNearplane;
        float                                       m_fFarplane;

    protected:

        NeoEngine::Light                            *m_pkLight;
        NeoEngine::Camera                           *m_pkCamera;

        NeoEngine::AnimatedNode                     *m_pkPathAnim;
        
        enum { eIdle, eAnim }                       m_eState;
        bool                                        m_bHasAnimation;
        float                                       m_fAnimLength;
        int                                         m_iCheckLastKey;


};


// descriptor for menu camera entity
class CTDMenuCameraDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDMenuCameraDesc() { };
                                                    ~CTDMenuCameraDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuCamera ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuCamera; }


};

// global instance of menu camera entity is used in dll interface
extern CTDMenuCameraDesc g_pkCTDMenuCameraEntity_desc;

}

#endif //_CTD_MENUCAM_H_
