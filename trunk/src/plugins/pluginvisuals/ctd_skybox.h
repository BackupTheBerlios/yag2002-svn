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
 # neoengine, skybox class
 #
 # this class implements an entity a skybox
 #
 #   date of creation:  02/09/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  02/09/2004 boto       creation of CTDSkyBox
 #
 ################################################################*/


#ifndef _CTD_SKYBOX_H_
#define _CTD_SKYBOX_H_


#include <base.h>

#include <neoengine/skybox.h>

#include <string>


namespace CTD_IPluginVisuals {


// name of this entity
#define CTD_ENTITY_NAME_SkyBox "SkyBox"


class CTDSkyBox : public CTD::BaseEntity
{

    public:

                                                    CTDSkyBox();
        virtual                                     ~CTDSkyBox();

        //******************************* override some functions **********************************//

        /**
        * Initializing function
        */
        void                                        Initialize();

        /**
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        bool                                        Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );


        //*******************************************************************************************//

        int                                         ParameterDescription( int param_index, CTD::ParameterDescriptor *pd );

        //******************************************************************************************//

        // entity parameters: texture names of six sky planes
        std::string                                 m_strTextureUP,
                                                    m_strTextureDown,
                                                    m_strTextureLeft,
                                                    m_strTextureRight,
                                                    m_strTextureFront,
                                                    m_strTextureBack;

    protected:

        NeoEngine::Room                             *m_pkRoom;

        NeoEngine::SkyBox                           *m_pkSkyBox;


};


// descriptor for sky box entity
class CTDSkyBoxDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDSkyBoxDesc() { };
                                                    ~CTDSkyBoxDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_SkyBox ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDSkyBox; };

};

// global instance of sky box entity is used in dll interface
extern CTDSkyBoxDesc g_pkSkyBoxEntity_desc;

} // namespace CTD_IPluginVisuals

#endif //_CTD_SKYBOX_H_
