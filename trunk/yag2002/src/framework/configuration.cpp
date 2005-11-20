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

#include <base.h>
#include "configuration.h"
#include "application.h"
#include "settings.h"

// default gui scheme
#define YAF3D_GUI_DEFUALT_SCHEME "gui/schemes/TaharezLook.scheme"

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( Configuration );

Configuration::Configuration() :
_p_settings( SettingsManager::get()->createProfile( YAF3D_GAMESETTING_PROFILENAME, Application::get()->getMediaPath() + YAF3D_GAMESETTING_FILENAME ) ),
_screenWidth( 600 ),
_screenHeight( 400 ),
_colorBits( 24 ),
_fullScreen( true ),
_keyboardType( YAF3D_GS_KEYBOARD_ENGLISH ),
_guiScheme( YAF3D_GUI_DEFUALT_SCHEME )
{
    // register standard settings
    _p_settings->registerSetting( YAF3D_GS_KEYBOARD,     _keyboardType );
    _p_settings->registerSetting( YAF3D_GS_SCREENWIDTH,  _screenWidth  );
    _p_settings->registerSetting( YAF3D_GS_SCREENHEIGHT, _screenHeight );
    _p_settings->registerSetting( YAF3D_GS_COLORBITS,    _colorBits    );
    _p_settings->registerSetting( YAF3D_GS_FULLSCREEN,   _fullScreen   );
    _p_settings->registerSetting( YAF3D_GS_GUISCHEME,    _guiScheme    );
    _p_settings->registerSetting( YAF3D_GS_SERVER_NAME,  _serverName   );
    _p_settings->registerSetting( YAF3D_GS_SERVER_IP,    _serverIP     );
    _p_settings->registerSetting( YAF3D_GS_SERVER_PORT,  _serverPort   );
}

Configuration::~Configuration()
{
}

bool Configuration::load()
{
    return SettingsManager::get()->loadProfile( YAF3D_GAMESETTING_PROFILENAME );
}

bool Configuration::store()
{
    return SettingsManager::get()->storeProfile( YAF3D_GAMESETTING_PROFILENAME );
}

void Configuration::getConfigurationAsString( std::vector< std::pair< std::string, std::string > >& settings )
{
    const std::vector< Settings::SettingBase* >& settingStorages = _p_settings->getAllSettingStorages();
    std::vector< Settings::SettingBase* >::const_iterator p_beg = settingStorages.begin(), p_end = settingStorages.end();
    for( ; p_beg != p_end; ++p_beg )
    {
        // get the setting value
        std::string token = ( *p_beg )->getTokenName();
        std::stringstream tokenvalue;
        const std::type_info& settings_typeinfo = ( *p_beg )->getTypeInfo();
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

bool Configuration::setSettingValueAsString( const std::string& name, const std::string& valuestring )
{
    const std::vector< Settings::SettingBase* >& settingStorages = _p_settings->getAllSettingStorages();
    std::vector< Settings::SettingBase* >::const_iterator p_beg = settingStorages.begin(), p_end = settingStorages.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        std::string token = ( *p_beg )->getTokenName();
        if ( token != name ) // search for given token
            continue;

        // set the setting value
        std::stringstream tokenvalue;
        const std::type_info& settings_typeinfo = ( *p_beg )->getTypeInfo();
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
            unsigned int value = ( unsigned int )-1;
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

void Configuration::shutdown()
{
    // store the latest changes in profile
    SettingsManager::get()->storeProfile( YAF3D_GAMESETTING_PROFILENAME );
    SettingsManager::get()->destroyProfile( YAF3D_GAMESETTING_PROFILENAME );
    // destroy singleton
    destroy();
}

} // namespace yaf3d
