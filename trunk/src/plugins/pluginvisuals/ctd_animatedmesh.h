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
 # neoengine, animated mesh class
 #
 # this class implements an keyframe animated mesh
 #
 #
 #   date of creation:  02/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  02/23/2004 boto       creation of CTDAnimMesh
 #
 #  03/30/2004 boto       set a link to staticmesh to get mesh for rendering
 #
 ################################################################*/

#ifndef _CTD_ANIMMESH_H_
#define _CTD_ANIMMESH_H_


#include <base.h>

#include <neoengine/skeletonanimator.h>
#include <neoengine/animatednode.h>

namespace CTD_IPluginVisuals {


// name of this entity
#define CTD_ENTITY_NAME_AnimMesh "AnimatedMesh"

class CTDAnimMesh : public CTD::BaseEntity
{

    public:

                                                    CTDAnimMesh();
                                                    ~CTDAnimMesh();

        //******************************* override some functions **********************************//

        /**
        * Initializing function
        */
        void                                        Initialize();
        
        /**
        * Post-Initializing function
        */
        void                                        PostInitialize();

        /**
        * Update object
        * \param fDeltaTime                         Time passed since last update
        */
        void                                         UpdateEntity( float fDeltaTime );

        /**
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        bool                                        Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

        //*******************************************************************************************//

        int                                         Message( int iMsgId, void *pkMsgStruct );
        int                                         ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

        // entity parameters
        NeoEngine::Vector3d                         m_vOffsetPosition;
        NeoEngine::Vector3d                         m_vOffsetRotation;

        std::string                                 m_strMeshEntiyName;
        CTD::BaseEntity                             *m_pkMeshEntity;

        std::string                                 m_strAnimationFile;

    // internal variables
    private:

        NeoEngine::AnimatedNode                     *m_pkAnimatedNode;

        /**
        * Reads and animation keyframe file and returns a pointer to animation object
        * \return  pointer to animation object, NULL means loading animation data was not successfull.
        */
        NeoEngine::AnimatedNode*                    ReadKeyframes( std::string &strFileName );


};


// descriptor for animated mesh entity
class CTDAnimMeshDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDAnimMeshDesc() { };
                                                    ~CTDAnimMeshDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_AnimMesh ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDAnimMesh; };

};

// global instance of moving platform entity is used in dll interface
extern CTDAnimMeshDesc g_pkAnimatedMeshEntity_desc;

} // namespace CTD_IPluginVisuals

#endif //_CTD_ANIMMESH_H_
