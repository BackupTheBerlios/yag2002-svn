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
 # neoengine, misc functions for setting up chat plugin
 #
 # it provides input listener and chat setup funtion
 #
 #
 #   date of creation:  24/08/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#ifndef _CTD_CHAT_CHAT_H_
#define _CTD_CHAT_CHAT_H_


#include <base.h>

namespace CTD_IPluginChat {

// class input listener
class CTDInputListener : public NeoEngine::InputEntity
{
	public:

									CTDInputListener( NeoEngine::InputGroup *pkGroup );

		virtual void				Input( const NeoEngine::InputEvent *pkEvent );

	private:

		// key names and codes
		std::string					m_strKeyRotUp;
		int							m_iKeyCodeRotUp;

		std::string					m_strKeyRotDown;
		int							m_iKeyCodeRotDown;

		std::string					m_strKeyRotLeft;
		int							m_iKeyCodeRotLeft;

		std::string					m_strKeyRotRight;
		int							m_iKeyCodeRotRight;

		std::string					m_strKeyMoveForward;
		int							m_iKeyCodeMoveForward;

		std::string					m_strKeyMoveBackward;
		int							m_iKeyCodeMoveBackward;

		std::string					m_strKeyMoveSideLeft;
		int							m_iKeyCodeMoveSideLeft;

		std::string					m_strKeyMoveSideRight;
		int							m_iKeyCodeMoveSideRight;

		std::string					m_strKeyChangeMode;
		int							m_iKeyCodeChangeMode;

};

// camera stuff
//------------------------------------------//
// camera related variables
extern float		g_fFOVAngle;
extern float		g_fNearplane;
extern float		g_fFarplane;

// create and setup a camera 
void CreateCamera();
//------------------------------------------//


// input processing stuff
//------------------------------------------//
#define MOVE_FORWARD				0
#define KEY_MOVE_FORWARD			"chat_move_forward"

#define MOVE_BACKWARD				1
#define KEY_MOVE_BACKWARD			"chat_move_backward"

#define MOVE_SIDE_RIGHT				2
#define KEY_MOVE_SIDE_RIGHT			"chat_move_side_right"

#define MOVE_SIDE_LEFT				3
#define KEY_MOVE_SIDE_LEFT			"chat_move_side_left"

#define ROTATE_UP          			4
#define KEY_ROTATE_UP	   			"chat_rotate_up"

#define ROTATE_DOWN        			5
#define KEY_ROTATE_DOWN	   			"chat_rotate_down"

#define ROTATE_LEFT        			6
#define KEY_ROTATE_LEFT	   			"chat_rotate_left"

#define ROTATE_RIGHT       			7
#define KEY_ROTATE_RIGHT   			"chat_rotate_right"

#define CHANGE_MODE			  			8
#define KEY_CHANGE_MODE	   			"chat_change_mode"

// Global data for keypresses
extern int							g_aiKeys[ 10 ];
extern NeoEngine::InputGroup		*g_pkInputGroup;
extern CTDInputListener				*g_pkInputListener;

// setup input listener
void CreateInputListener();
//------------------------------------------//

}

#endif //_CTD_CHAT_SETUP_H_
