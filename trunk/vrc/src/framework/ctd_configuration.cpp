/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # the overall game configuration is handled here
 #  - loading
 #  - storing
 #  - value retrieval method
 #  - value modificaton method
 #
 #   date of creation:  04/02/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_configuration.h"
#include "ctd_application.h"
#include "ctd_settings.h"

using namespace std;

// default gui scheme
#define CTD_GUI_DEFUALT_SCHEME "gui/schemes/TaharezLook.scheme"

namespace CTD
{

CTD_SINGLETON_IMPL( Configuration );

// implementation of GuiManager
Configuration::Configuration() :
_p_settings( SettingsManager::get()->createProfile( CTD_GAMESETTING_PROFILENAME, Application::get()->getMediaPath() + CTD_GAMESETTING_FILENAME ) ),
_keyboardType( CTD_GS_KEYBOARD_ENGLISH ),
_screenWidth( 600 ),
_screenHeight( 400 ),
_mouseSensitivity( 1.0f ),
_guiScheme( CTD_GUI_DEFUALT_SCHEME ),
_playerName( "NoName" ),
_mouseInverted( false ),
_moveForward( "W" ),
_moveBackward( "S" ),
_moveLeft( "A" ),
_moveRight( "D" ),
_jump( "Space" )
{
    // register all settings
    _p_settings->registerSetting( CTD_GS_KEYBOARD,            _keyboardType     );
    _p_settings->registerSetting( CTD_GS_SCREENWIDTH,         _screenWidth      );
    _p_settings->registerSetting( CTD_GS_SCREENHEIGHT,        _screenHeight     );
    _p_settings->registerSetting( CTD_GS_GUISCHEME,           _guiScheme        );
    _p_settings->registerSetting( CTD_GS_PLAYERNAME,          _playerName       );
    _p_settings->registerSetting( CTD_GS_MOUSESENS,           _mouseSensitivity );
    _p_settings->registerSetting( CTD_GS_INVERTMOUSE,         _mouseInverted    );
    _p_settings->registerSetting( CTD_GS_SERVER_NAME,         _serverName       );
    _p_settings->registerSetting( CTD_GS_SERVER_IP,           _serverIP         );
    _p_settings->registerSetting( CTD_GS_SERVER_PORT,         _serverPort       );

    _p_settings->registerSetting( CTD_GS_KEY_MOVE_FORWARD,    _moveForward      );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_BACKWARD,   _moveBackward     );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_LEFT,       _moveLeft         );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_RIGHT,      _moveRight        );
    _p_settings->registerSetting( CTD_GS_KEY_JUMP,            _jump             );

    // load profile
    bool exists = SettingsManager::get()->loadProfile( CTD_GAMESETTING_PROFILENAME );
    if ( !exists )
    {
        assert( SettingsManager::get()->storeProfile( CTD_GAMESETTING_PROFILENAME ) && "cannot store settings file" );
    }
}

Configuration::~Configuration()
{
}

void Configuration::store()
{
    SettingsManager::get()->storeProfile( CTD_GAMESETTING_PROFILENAME );
}

void Configuration::shutdown()
{
    // store the latest changes in profile
    SettingsManager::get()->storeProfile( CTD_GAMESETTING_PROFILENAME );

    // destroy singleton
    destroy();
}

}
