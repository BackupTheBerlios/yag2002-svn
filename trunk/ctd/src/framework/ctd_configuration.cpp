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
_colorBits( 24 ),
_fullScreen( true ),
_mouseSensitivity( 1.0f ),
_guiScheme( CTD_GUI_DEFUALT_SCHEME ),
_playerName( "NoName" ),
_playerConfigDir( "player" ),
_playerConfig( "player.cfg" ),
_mouseInverted( false ),
_moveForward( "W" ),
_moveBackward( "S" ),
_moveLeft( "A" ),
_moveRight( "D" ),
_jump( "Space" ),
_cameramode( "F1" ),
_chatmode( "RMB" )
{
    // register all settings
    _p_settings->registerSetting( CTD_GS_KEYBOARD,            _keyboardType     );
    _p_settings->registerSetting( CTD_GS_SCREENWIDTH,         _screenWidth      );
    _p_settings->registerSetting( CTD_GS_SCREENHEIGHT,        _screenHeight     );
    _p_settings->registerSetting( CTD_GS_COLORBITS,           _colorBits        );
    _p_settings->registerSetting( CTD_GS_FULLSCREEN,          _fullScreen       );
    _p_settings->registerSetting( CTD_GS_GUISCHEME,           _guiScheme        );

    _p_settings->registerSetting( CTD_GS_PLAYER_NAME,         _playerName       );
    _p_settings->registerSetting( CTD_GS_PLAYER_CONFIG_DIR,   _playerConfigDir  );
    _p_settings->registerSetting( CTD_GS_PLAYER_CONFIG,       _playerConfig     );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_FORWARD,    _moveForward      );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_BACKWARD,   _moveBackward     );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_LEFT,       _moveLeft         );
    _p_settings->registerSetting( CTD_GS_KEY_MOVE_RIGHT,      _moveRight        );
    _p_settings->registerSetting( CTD_GS_KEY_JUMP,            _jump             );
    _p_settings->registerSetting( CTD_GS_KEY_CAMERAMODE,      _cameramode       );
    _p_settings->registerSetting( CTD_GS_KEY_CHATMODE,        _chatmode         );

    _p_settings->registerSetting( CTD_GS_MOUSESENS,           _mouseSensitivity );
    _p_settings->registerSetting( CTD_GS_INVERTMOUSE,         _mouseInverted    );
    _p_settings->registerSetting( CTD_GS_SERVER_NAME,         _serverName       );
    _p_settings->registerSetting( CTD_GS_SERVER_IP,           _serverIP         );
    _p_settings->registerSetting( CTD_GS_SERVER_PORT,         _serverPort       );

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

void Configuration::getConfigurationAsString( std::vector< std::pair< std::string, std::string > >& settings )
{
    std::vector< Settings::SettingBase* >& settingStorages = const_cast< std::vector< Settings::SettingBase* >& >( _p_settings->getAllSettingStorages() );
    std::vector< Settings::SettingBase* >::iterator p_beg = settingStorages.begin(), p_end = settingStorages.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        // get the setting value
        std::string token = ( *p_beg )->getTokenName();
        std::stringstream tokenvalue;
        const type_info& settings_typeinfo = ( *p_beg )->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) 
        {
            bool value;
            _p_settings->getValue( token, value );
            tokenvalue << ( value ? "true" : "false" );
        } 
        else if ( settings_typeinfo == typeid( int ) ) 
        {
            int value;
            _p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( unsigned int ) ) 
        {
            unsigned int value;
            _p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( std::string ) ) 
        {
            std::string value;
            _p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( float ) ) 
        {
            float value;
            _p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( osg::Vec3f ) ) 
        {
            osg::Vec3f value;
            _p_settings->getValue( token, value );
            tokenvalue << value.x() << " " << value.y() << " " << value.z();
        }
        settings.push_back( make_pair( token, tokenvalue.str() ) );
    }
}

bool Configuration::setSettingValue( const std::string& name, const std::string& valuestring )
{
    std::vector< Settings::SettingBase* >& settingStorages = const_cast< std::vector< Settings::SettingBase* >& >( _p_settings->getAllSettingStorages() );
    std::vector< Settings::SettingBase* >::iterator p_beg = settingStorages.begin(), p_end = settingStorages.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        std::string token = ( *p_beg )->getTokenName();
        if ( token != name ) // search for given token
            continue;

        // set the setting value
        std::stringstream tokenvalue;
        const type_info& settings_typeinfo = ( *p_beg )->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) 
        {
            bool value = ( valuestring == "true" ) ? true : false;
            _p_settings->setValue( token, value );
            break;
        } 
        else if ( settings_typeinfo == typeid( int ) ) 
        {
            tokenvalue << valuestring;
            int value = -1;
            tokenvalue >> value;
            _p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( unsigned int ) ) 
        {
            tokenvalue << valuestring;
            unsigned int value = -1;
            tokenvalue >> value;
            _p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( std::string ) ) 
        {
            std::string value = valuestring;
            _p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( float ) ) 
        {
            tokenvalue << valuestring;
            float value = -1;
            tokenvalue >> value;
            _p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( osg::Vec3f ) ) 
        {
            tokenvalue << valuestring;
            osg::Vec3f value;
            tokenvalue >> value._v[ 0 ] >> value._v[ 1 ] >> value._v[ 2 ];
            _p_settings->setValue( token, value );
            break;
        }
    }

    if ( p_beg == p_end )
        return false;

    return true;
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
