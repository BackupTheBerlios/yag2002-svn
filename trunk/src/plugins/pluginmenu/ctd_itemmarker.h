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
 # neoengine, menu item marker
 #
 # this class implements the a marker appearing when an item is marked
 #
 #
 #   date of creation:  05/27/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/27/2004 boto       creation of CTDItemMarker
 #
 ################################################################*/

#ifndef _CTD_ITEMMARKER_H_
#define _CTD_ITEMMARKER_H_


#include "base.h"
#include <ctd_printf.h>

namespace CTD_IPluginMenu
{

// this is the printf object
extern CTDPrintf	g_CTDPrintf;


// name of this entity
#define CTD_ENTITY_NAME_ItemMarker  "ItemMarker"

class CTDItemMarker : public CTD::BaseEntity
{

	public:

													CTDItemMarker();
													~CTDItemMarker();

		//******************************* override some functions **********************************//

		/**
		* Initializing function
		*/
		void										Initialize();

		/**
		* Render object
		* \param pkFrustum                          Current view frustum (if any)
		* \param bForce                             Render even if rendered previously this frame or deactivated (default false)
		* \return                                   true if we were rendered, false if not (already rendered, not forced)
		*/
		bool										Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

		/**
		* Update object
		* \param fDeltaTime                         Time passed since last update
		*/
		void									    UpdateEntity( float fDeltaTime );

		//*******************************************************************************************//

		int											ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//

		NeoEngine::MeshEntity						*m_pkMesh;
		float										m_fWaveVar;

};


// descriptor for item marker entity
class CTDItemMarkerDesc : public CTD::EntityDescriptor
{

	public:
													CTDItemMarkerDesc() { };
													~CTDItemMarkerDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_ItemMarker ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDItemMarker; }


};

// global instance of item marker entity is used in dll interface
extern CTDItemMarkerDesc g_pkCTDItemMarkerEntity_desc;

}

#endif //_CTD_ITEMMARKER_H_
