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
 # neoengine, entity for point light
 #
 #
 #   date of creation:  01/03/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/03/2004 boto       creation of CTDPointLight
 #
 #  03/22/2004 boto       added sprite entity parameter for light
 #
 ################################################################*/

#ifndef _CTD_POINTLIGHT_H_
#define _CTD_POINTLIGHT_H_


#include <base.h>
#include <string>


namespace CTD_IPluginVisuals {

// name of this entity
#define CTD_ENTITY_NAME_PointLight "PointLight"


class CTDPointLight : public CTD::BaseEntity
{

	public:

													CTDPointLight();
													~CTDPointLight();

		//******************************* overriding functions **********************************//

		/**
		* Initializing function, this is called after all engine modules are initialized and a map is loaded.
		*/
		void										Initialize();

		/**
		* Post-Initializing function
		*/
		void										PostInitialize();	

		// render the light sources as bbox for debugging

		/**
		* Render object
		* \param pkFrustum                          Current view frustum (if any)
		* \param bForce                             Render even if rendered previously this frame or deactivated (default false)
		* \return                                   true if we were rendered, false if not (already rendered, not forced)
		*/
		bool										Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

		/**
		* Function for parameter description. This is used to set and retrieve entity parameters.
		* \param   iParamIndex                      Parameter index
		* \param   pkDesc                           Parameter descriptor to fill my this function.
		* \return                                   If pkDesc is NULL then the count of parameters must be retrieved.
		*                                            Otherwise -1 when a requested parameter does not match to one of entity parameters.
		*/
		int											ParameterDescription( int iPramIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//

		// entity parameters
		NeoEngine::Light							*m_pkLight;

		NeoEngine::Vector3d							m_vPosition;
		float										m_fRadius;

		// ligth sprite
		std::string									m_strSprite;
		// entity for creating sprites
		CTD::BaseEntity							    *m_pkSpriteFactory;
		// created sprite entity using sprite factory; take care: you must detach this created entity in room manager during destruction
		CTD::BaseEntity							    *m_pkSprite;

};


// descriptor for observer entity
class CTDPointLightDesc : public CTD::EntityDescriptor
{

	public:
													CTDPointLightDesc() { };
													~CTDPointLightDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_PointLight ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDPointLight; };

};

// global instance of observer entity is used in dll interface
extern CTDPointLightDesc g_pkPointLightEntity_desc;

} // namespace CTD_IPluginVisuals

#endif //_CTD_POINTLIGHT_H_
