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
 # neoengine, menu group
 #
 # this class implements the a menu group containing items
 #
 #
 #   date of creation:  05/15/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/15/2004 boto       creation of CTDMenuGroup
 #
 ################################################################*/

#ifndef _CTD_MENUGROUP_H_
#define _CTD_MENUGROUP_H_


#include "base.h"
#include <ctd_printf.h>

#include <string>

namespace CTD_IPluginMenu
{

// name of this entity
#define CTD_ENTITY_NAME_MenuGroup  "MenuGroup"

class CTDMenuGroup : public CTD::BaseEntity
{

	public:

													CTDMenuGroup();
													~CTDMenuGroup();

		//******************************* override some functions **********************************//

		/**
		* Post-initializing function
		*/
		void										PostInitialize();

		//*******************************************************************************************//

		int											ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//


		std::string									m_strGroupName;
		std::string									m_strItemNames;
		unsigned int								m_uiGroupID;

};


// descriptor for menu camera entity
class CTDMenuGroupDesc : public CTD::EntityDescriptor
{

	public:
													CTDMenuGroupDesc() { };
													~CTDMenuGroupDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuGroup ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuGroup; }


};

// global instance of observer entity is used in dll interface
extern CTDMenuGroupDesc g_pkCTDMenuGroupEntity_desc;

}

#endif //_CTD_MENUGROUP_H_
