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
 # neoengine, 3d fonts
 #
 # this class implements a 3d font manager
 #
 #
 #   date of creation:  05/24/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/24/2004 boto       creation of CTD3DFontMgr
 #
 ################################################################*/

#ifndef _CTD_3DFONTMGR_H_
#define _CTD_3DFONTMGR_H_


#include "../base.h"
#include <ctd_printf.h>

#include <string>

namespace CTD_IPluginMenu
{

// this is the printf object
extern CTDPrintf	g_CTDPrintf;

// this class describes one letter
class CTDLetter
{

	public:
													CTDLetter();
													~CTDLetter();

		char										m_cLetter;			// the letter to be represented via a 3d mesh
		std::string									m_strMeshFile;		// mesh's file name
		NeoEngine::SceneNode						*m_pkMeshNode;		// mesh node

};


// name of this entity
#define CTD_ENTITY_NAME_3DFontManager  "3DFontManager"

class CTD3DFontMgr : public CTD::BaseEntity
{

	public:

													CTD3DFontMgr();
													~CTD3DFontMgr();

		//******************************* override some functions **********************************//

		/**
		* Initializing function
		*/
		void										Initialize();

		/**
		* Messaging function.
		* \param  iMsgId                            Message ID
		* \param  pMsgStruct                        Message specific data structure
		* \return                                   This return value is message specific.
		*/
		int											Message( int iMsgId, void *pMsgStruct );

		//*******************************************************************************************//

		int											ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

		//******************************************************************************************//

		// configuration file for font manager
		std::string									m_strFontsCfg;

	private:

		// is set if font manager successfully initialized
		bool										m_bReady;

		std::vector< CTDLetter* >					m_vpkLetters;
		// size of white space character
		float										m_fSpaceSize;

		// setup font manager
		bool										SetupFontManager( std::string &strCfgFileName );



};


// descriptor for menu camera entity
class CTD3DFontMgrDesc : public CTD::EntityDescriptor
{

	public:
													CTD3DFontMgrDesc() { };
													~CTD3DFontMgrDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_3DFontManager ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTD3DFontMgr; }


};

// global instance of 3d font manager entity is used in dll interface
extern CTD3DFontMgrDesc g_pkCTD3DFontMgrEntity_desc;

}

#endif //_CTD_3DFONTMGR_H_
