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
 # neoengine, input item for 3d menu
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
 #  05/13/2004 boto       creation of CTDMenuInputItem
 #
 ################################################################*/

#ifndef _CTD_MENUINPUTITEM_H_
#define _CTD_MENUINPUTITEM_H_


#include "../base.h"
#include <ctd_printf.h>
#include "ctd_menuitem.h"
#include <string>

namespace CTD_IPluginMenu
{

extern CTDPrintf	g_CTDPrintf;

// name of this entity
#define CTD_ENTITY_NAME_MenuInputItem  "MenuInputItem"

// this class holds the information about one single formatted 3d letter
//  it is used for rendering
class CTD3DInputText
{
	public:
													CTD3DInputText() {};
													~CTD3DInputText() {};

		NeoEngine::SceneNode*						m_pkMeshNode;
		NeoEngine::Vector3d							m_kPositionOffset;	// position offset of letter mesh relative to menu item position
		float										m_fLetterWidth;

};

class CTDMenuInputItem: public CTDMenuItem
{

	public:

													CTDMenuInputItem();
													~CTDMenuInputItem();

		//******************************* override some functions **********************************//

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
		* Render object
		* \param pkFrustum                          Current view frustum (if any)
		* \param bForce                             Render even if rendered previously this frame or deactivated (default false)
		* \return                                   true if we were rendered, false if not (already rendered, not forced)
		*/
		bool										Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

		/**
		* Messaging function.
		* \param  iMsgId                            Message ID
		* \param  pMsgStruct                        Message specific data structure
		* \return                                   This return value is message specific.
		*/
		int											Message( int iMsgId, void *pkMsgStruct );

		//*******************************************************************************************//

		int											ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//

		unsigned int								m_uiMaxChars;
		// distance to camera when item is in focus
		float										m_fDistanceOnFocused;

	private:

		enum  { eIdle, eStartFocus, eEditing, eOnFocus, eLostFocus } m_eState;

		NeoEngine::Vector3d							m_kCurrentPos;
		NeoEngine::Vector3d							m_kDestPos;
		float										m_fBlendFactor;

		NeoEngine::Matrix							m_kRotationMatrix;
		unsigned int								m_uiInputLetterIndex;
		bool										m_bInputDispatched;

		float										m_fTextSize;
		float										m_fSpaceGap;

		std::string									m_strInputText;
		std::vector< CTDLetter* >					*m_pvpkLetters;
		std::vector< CTD3DInputText* >				m_vpkFormatted3DText;

		// given a letter this functions returns the corresponding 3d mesh
		NeoEngine::SceneNode*						GetLetterMeshNode( const char &cLetter );

		// add and format a new letter to 3d text
		void										AddLetterTo3DText( NeoEngine::SceneNode* pkLetterNode );
		// delete last letter in text
		void										DelLetter();
		// format 3d text( calculate mesh positions )
		void										Format3DText();

};


// descriptor for menu's output item entity
class CTDMenuInputItemDesc : public CTD::EntityDescriptor
{

	public:
													CTDMenuInputItemDesc() { };
													~CTDMenuInputItemDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuInputItem ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuInputItem; }


};

// global instance of input item entity is used in dll interface
extern CTDMenuInputItemDesc g_pkCTDMenuInputItemEntity_desc;

}

#endif //_CTD_MENUINPUTITEM_H_
