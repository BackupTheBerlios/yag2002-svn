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
 # neoengine, base header for menu plugin
 #
 #   date of creation:  05/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/13/2004 boto       creation of base.h
 #
 ################################################################*/



#ifndef _CTD_PLUGIN_MENU_H_
#define _CTD_PLUGIN_MENU_H_


#include <neoengine/base.h>
#include <neoengine/nemath.h>
#include <neoengine/mesh.h>
#include <neoengine/logstream.h>
#include <neoengine/renderentity.h>
#include <neoengine/sceneentity.h>
#include <neoengine/scenenode.h>
#include <neoengine/core.h>
#include <neoengine/aabb.h>
#include <neoengine/sphere.h>
#include <neoengine/light.h>

#include <ctd_framework.h>
#include <ctd_descriptor.h>
#include <ctd_baseentity.h>
#include <ctd_plugin_defs.h>
#include <ctd_levelmanager.h>

namespace CTD_IPluginMenu
{

//! returns the menu level set
extern CTD::LevelSet*     GetMenuLevelSet();
extern void               SetMenuLevelSet( CTD::LevelSet* pkLevelSet );
extern CTD::LevelSet*     GetGameLevelSet();
extern void               SetGameLevelSet( CTD::LevelSet* pkLevelSet );

// common constants for 3d menu plugin
//----------------------------------------------------------//
// some default values
#define CTD_MENUITEM_UNKNOWN_PARAM_NAME			"UnknownParam"
#define CTD_DISTANCE_TO_CAMERA_WHEN_FOCUSED		2.0f
#define CTD_MENUCTRL_ITEMMARKER_Y_OFFSET		1.5f
//----------------------------------------------------------//

// structures and ids for internal entity communication via messaging
//----------------------------------------------------------//
class CTDItemGroup;

// structure for registering a menu menu at start-up
#define CTD_MENUCTRL_REGISTER_GROUP			0x0000FF00
typedef struct _CTDMenuCtrlRegisterGroup {

	CTDItemGroup						*m_pkItemGroupPtr;	// pointer to menu group

} tCTDMenuCtrlRegisterGroup;
//----------------------------------------------------------//

// 3d font manager related defines and structs
//---------------------------------------------------------//
// this struct is used by menu items to get the 3d font set from font manager
#define CTD_3DFONTMGR_GETFONT				0x0000FE00
class CTDLetter;
typedef struct _CTD3DFonts {

	std::vector< CTDLetter* >			*m_pvpkLetters; // list of all 3d letters
	float								m_fSpaceSize;	// size ( width ) of white character

} tCTD3DFont;
//----------------------------------------------------------//

// menu item related messaging stuff
//----------------------------------------------------------//
//  set / get the value of input text
#define CTD_INPUTITEM_SET_STRING			0x0000FD00
#define CTD_INPUTITEM_GET_STRING			0x0000FD01

//  message for getting the destination group id
#define CTD_CHANGEGROUP_GET_ID				0x0000FD02
//   sending this id returns 1 signalizing that the camera reached the destination position
//    after trigerring it for animation; this is used in menu control to lock the input during 
//    camera path animation
#define CTD_CHANGEGROUP_CAM_IN_POS			0x0000FD03
//----------------------------------------------------------//

}

#endif //_CTD_PLUGIN_MENU_H_
