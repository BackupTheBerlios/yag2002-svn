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
 # neoengine,exit game item for 3d menu
 #  this item allows the termination of game
 #
 #
 #   date of creation:  08/04/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  08/04/2004 boto       creation of CTDMenuExitGameItem
 #
 ################################################################*/

#ifndef _CTD_MENUEXITGAMEITEM_H_
#define _CTD_MENUEXITGAMEITEM_H_


#include "../base.h"
#include "ctd_menuitem.h"
#include "../ctd_menucontrol.h"
#include <string>

namespace CTD_IPluginMenu
{

// name of this entity
#define CTD_ENTITY_NAME_MenuExitGameItem  "MenuExitGameItem"

class CTDMenuExitGameItem: public CTDMenuItem
{

	public:

													CTDMenuExitGameItem();
													~CTDMenuExitGameItem();

		//******************************* override some functions **********************************//

		/**
		* Initializing function
		*/
		void										Initialize();

		/**
		* Post-initializing function, this is called after all plugins' entities are initilized.
		* One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
		*/
		 void										PostInitialize();

		/**
		* Update object
		* \param fDeltaTime                         Time passed since last update
		*/
		void									    UpdateEntity( float fDeltaTime );

		/**
		* Messaging function.
		* \param  iMsgId                            Message ID
		* \param  pMsgStruct                        Message specific data structure
		* \return                                   This return value is message specific.
		*/
		int											Message( int iMsgId, void *pkMsgStruct );

		/**
		* Call back funtion for beginning with focus state. This function is called when 
		*  the user activates an item.
		*/
		void										OnBeginFocus();

		/**
		* Call back funtion for ending with focus state. This function is called when 
		*  the user changes the focus to another item.
		*/
		void										OnEndFocus();

		/**
		* Call back funtion for getting activated. This function is called when 
		*  the user activates an item, e.g. by pressing enter for a focused item.
		*/
		void										OnActivate();

		//*******************************************************************************************//

		int											ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//

	private:

		NeoEngine::Vector3d							m_kCurrPosition;

		float										m_fPosVar;

};


// descriptor for menu's exit game item entity
class CTDMenuExitGameItemDesc : public CTD::EntityDescriptor
{

	public:
													CTDMenuExitGameItemDesc() { };
													~CTDMenuExitGameItemDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuExitGameItem ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuExitGameItem; }


};

// global instance of exit game item entity is used in dll interface
extern CTDMenuExitGameItemDesc g_pkCTDMenuExitGameItemEntity_desc;

}

#endif //_CTD_MENUEXITGAMEITEM_H_
