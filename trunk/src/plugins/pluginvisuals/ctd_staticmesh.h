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
 # neoengine, static mesh class
 #
 # this class implements an entity for placing meshes into a map
 #
 #
 #   date of creation:  12/14/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/14/2003 boto       creation of CTDStaticMesh
 #
 #
 ################################################################*/

#ifndef _CTD_STATICMESH_H_
#define _CTD_STATICMESH_H_


#include <base.h>

#include <string>

namespace CTD_IPluginVisuals {


// name of this entity
#define CTD_ENTITY_NAME "StaticMesh"


class CTDStaticMesh : public CTD::BaseEntity
{

	public:

													CTDStaticMesh();
		virtual										~CTDStaticMesh();

		//******************************* override some functions **********************************//

		/**
		* Initializing function
		*/
		void										Initialize();

		/**
		* Update object
		* \param fDeltaTime                         Time passed since last update
		*/
		void									    UpdateEntity( float fDeltaTime );

		/**
		* PreRender function called before rendering phase begins.
		* Here it is used to render the cubemap.
		*/
		void										PreRender();

		/**
		* Render object
		* \param pkFrustum                          Current view frustum (if any)
		* \param bForce                             Render even if rendered previously this frame or deactivated (default false)
		* \return                                   true if we were rendered, false if not (already rendered, not forced)
		*/
		bool										Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );


		//*******************************************************************************************//

		int											ParameterDescription( int param_index, CTD::ParameterDescriptor *pd );

		//******************************************************************************************//

		// entity parameters
		NeoEngine::Vector3d							m_kPosition;
		NeoEngine::Vector3d							m_kRotation;

		NeoEngine::Vector3d							m_kAmientColor;
		NeoEngine::Vector3d							m_kDiffuseColor;
		NeoEngine::Vector3d							m_kSpecularColor;
		NeoEngine::Vector3d							m_kEmission;
		float										m_fShininess;

		float										m_fScale;
		bool										m_bThrowShadows;
		bool										m_bEnvMapping;

		// mesh entity
		NeoEngine::MeshEntity						*m_pkMesh;

	private:


		NeoEngine::RenderDevice						*m_pkRenderDevice;
		NeoEngine::PixelBuffer						*m_pkCubemap;
		NeoEngine::Camera							*m_pkCubemapCamera;

};


// descriptor for static mesh entity
class CTDStaticMeshDesc : public CTD::EntityDescriptor
{

	public:
													CTDStaticMeshDesc() { };
													~CTDStaticMeshDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDStaticMesh; };

};

// global instance of static mesh entity is used in dll interface
extern CTDStaticMeshDesc g_pkStaticMeshEntity_desc;

} // namespace CTD_IPluginVisuals 

#endif //_CTD_STATICMESH_H_
