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
 # neoengine, output item for 3d menu
 #
 #
 #   date of creation:  05/14/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/13/2004 boto       creation of CTDMenuOutputItem
 #
 ################################################################*/

#ifndef _CTD_MENUOUTPUTITEM_H_
#define _CTD_MENUOUTPUTITEM_H_


#include "../base.h"
#include "ctd_menuitem.h"
#include "../ctd_menucontrol.h"
#include <string>

namespace CTD_IPluginMenu
{

// name of this entity
#define CTD_ENTITY_NAME_MenuOutputItem  "MenuOutputItem"

class CTDMenuOutputItem: public CTDMenuItem
{

	public:

													CTDMenuOutputItem();
													~CTDMenuOutputItem();

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
		*  the user activates an item by e.g. going with mouse over the item mesh.
		*/
		void										OnBeginFocus();

		/**
		* Call back funtion for ending with focus state. This function is called when 
		*  the user leaves item's active area.
		*/
		void										OnEndFocus();


		/**
		* Call back funtion for input event. This function is called when 
		*  the user presses a key.
		*/
		void										OnInput( CTDMenuControl::CTDCtrlKeys eCtrlKey, unsigned int iKeyData, char cKeyData );



		//*******************************************************************************************//

		int											ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//

		// distance to camera when item is in focus
		float										m_fDistanceOnFocused;

	private:

		NeoEngine::Vector3d							m_kCurrentPos;
		NeoEngine::Vector3d							m_kDestPos;
		float										m_fBlendFactor;

		enum  { eIdle, eStartFocus, eActive, eOnFocus, eLostFocus } m_eState;


};


// descriptor for menu's output item entity
class CTDMenuOutputItemDesc : public CTD::EntityDescriptor
{

	public:
													CTDMenuOutputItemDesc() { };
													~CTDMenuOutputItemDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuOutputItem ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuOutputItem; }


};

// global instance of output item entity is used in dll interface
extern CTDMenuOutputItemDesc g_pkCTDMenuOutputItemEntity_desc;

}

#endif //_CTD_MENUOUTPUTITEM_H_
