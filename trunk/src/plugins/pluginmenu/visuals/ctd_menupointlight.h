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
 # neoengine, entity for point light for menu system
 #
 #
 #   date of creation:  10/19/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  10/19/2004 boto       creation of CTDMenuPointLight
 #
 ################################################################*/

#ifndef _CTD_MENUPOINTLIGHT_H_
#define _CTD_MENUPOINTLIGHT_H_

#include <base.h>

namespace CTD_IPluginMenu {

// name of this entity
#define CTD_ENTITY_NAME_MenuPointLight "MenuPointLight"


class CTDMenuPointLight : public CTD::BaseEntity
{

    public:

                                                    CTDMenuPointLight();
                                                    ~CTDMenuPointLight();

        //******************************* overriding functions **********************************//

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        Initialize();

        /**
        * Post-Initializing function
        */
        void                                        PostInitialize();   

        /**
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        bool                                        Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

        /**
        * Function for parameter description. This is used to set and retrieve entity parameters.
        * \param   iParamIndex                      Parameter index
        * \param   pkDesc                           Parameter descriptor to fill my this function.
        * \return                                   If pkDesc is NULL then the count of parameters must be retrieved.
        *                                            Otherwise -1 when a requested parameter does not match to one of entity parameters.
        */
        int                                         ParameterDescription( int iPramIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

    protected:

        NeoEngine::Room                             *m_pkRoom;

        // entity parameters
        NeoEngine::Light                            *m_pkLight;

        NeoEngine::Vector3d                         m_vPosition;

        // bbox radius
        float                                       m_fRadius;

};


// descriptor for observer entity
class CTDMenuPointLightDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDMenuPointLightDesc() { };
                                                    ~CTDMenuPointLightDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuPointLight ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuPointLight; };

};

// global instance of observer entity is used in dll interface
extern CTDMenuPointLightDesc g_pkMenuPointLightEntity_desc;

} // namespace CTD_IPluginMenu

#endif //_CTD_MENUPOINTLIGHT_H_
