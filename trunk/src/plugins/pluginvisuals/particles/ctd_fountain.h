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
 # neoengine, particle class
 #
 # this class implements a particle system
 #
 #
 #   date of creation:  03/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/23/2004 boto       creation of CTDParticleFountain
 #
 ################################################################*/

#ifndef _CTD_PARTICLE_H_
#define _CTD_PARTICLE_H_

#include <base.h>
#include "ctd_baseparticle.h"
#include <neoengine/sprite.h>

#include <string>


namespace CTD_IPluginVisuals {

// name of this entity
#define CTD_ENTITY_NAME_ParticleFountain "ParticleFountain"

class CTDParticleFountain : public CTD::BaseEntity, public CTDBaseParticle
{

    public:

                                                    CTDParticleFountain();
        virtual                                     ~CTDParticleFountain();

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

        int                                         ParameterDescription( int param_index, CTD::ParameterDescriptor *pd );

        //******************************************************************************************//

        
        // entity parameters

        NeoEngine::Vector3d                         m_kPosition;
        NeoEngine::Vector3d                         m_kDimensions;

        int                                         m_iPopulation;
        int                                         m_iRate;
        float                                       m_fLife;
        float                                       m_fFadeOutFactor;

        NeoEngine::Vector3d                         m_kVelocity;
        float                                       m_fVelocitySigma;
        NeoEngine::Vector3d                         m_kGravity;
        float                                       m_fBounceFriction;
        float                                       m_fBounceResilience;
        NeoEngine::Vector3d                         m_kSourcePosition;
        float                                       m_kSourceRadius;

        // color ranges
        NeoEngine::Vector3d                         m_kColor;

        // size of one single particle
        float                                       m_fSizeU;
        float                                       m_fSizeV;
        // material file of one single particle
        std::string                                 m_strMatFile;

        // this flas determines wether the particle system should be activated after initialization
        //  if you intend to use this particle system in other entities set this parameter to false ( default true )
        bool                                        m_bAutoActivate;

    private:

        NeoEngine::Sprite                           *m_pkSprite;

};


// descriptor for particle entity
class CTDParticleFountainDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDParticleFountainDesc() { };
                                                    ~CTDParticleFountainDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_ParticleFountain ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDParticleFountain; };

};

// global instance of particle entity is used in dll interface
extern CTDParticleFountainDesc g_pkParticleFountainEntity_desc;

} // namespace CTD_IPluginVisuals

#endif //_CTD_PARTICLE_H_
