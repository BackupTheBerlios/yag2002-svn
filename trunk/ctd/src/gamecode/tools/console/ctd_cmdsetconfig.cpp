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
 # console command for setting a value in current configuration
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdsetconfig.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdSetConfig );


CmdSetConfig::CmdSetConfig() :
 BaseConsoleCommand( CMD_NAME_SETCONFIG )
{
    setUsage( CMD_USAGE_SETCONFIG );
}

CmdSetConfig::~CmdSetConfig()
{
}

const std::string& CmdSetConfig::execute( const std::vector< std::string >& argmuments )
{
    _cmdResult = "";
    if ( argmuments.size() < 2 )
    {
        _cmdResult = CMD_USAGE_SETCONFIG;
        _cmdResult += "\n";
        return _cmdResult;
    }
    // get the token name to be set with new value
    std::string reqtoken = argmuments[ 0 ];

    Settings* p_settings = const_cast< Settings* >( Configuration::get()->getAllSettings() );
    std::vector< Settings::SettingBase* >& settingStorages = const_cast< std::vector< Settings::SettingBase* >& >( p_settings->getAllSettingStorages() );

    std::vector< Settings::SettingBase* >::iterator p_beg = settingStorages.begin(), p_end = settingStorages.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        string token = ( *p_beg )->getTokenName();
        if ( token != reqtoken ) // search for given token
            continue;

        // set the setting value
        stringstream tokenvalue;
        const type_info& settings_typeinfo = ( *p_beg )->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) 
        {
            bool value = ( argmuments[ 1 ] == "true" ) ? true : false;
            p_settings->setValue( token, value );
            break;
        } 
        else if ( settings_typeinfo == typeid( int ) ) 
        {
            tokenvalue << argmuments[ 1 ];
            int value;
            tokenvalue >> value;
            p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( unsigned int ) ) 
        {
            tokenvalue << argmuments[ 1 ];
            unsigned int value;
            tokenvalue >> value;
            p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( string ) ) 
        {
            string value = argmuments[ 1 ];
            p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( float ) ) 
        {
            tokenvalue << argmuments[ 1 ];
            float value;
            tokenvalue >> value;
            p_settings->setValue( token, value );
            break;
        }
        else if ( settings_typeinfo == typeid( osg::Vec3f ) ) 
        {
            if ( argmuments.size() < 4 )
            {
                _cmdResult += "could not set requested setting name '" + reqtoken + "'. a vector with 3 elements is expected as value!\n";
                return _cmdResult;
            }

            tokenvalue << argmuments[ 1 ] << " " << argmuments[ 2 ] << " " << argmuments[ 2 ];
            osg::Vec3f value;
            tokenvalue >> value._v[ 0 ] >> value._v[ 1 ] >> value._v[ 2 ];
            p_settings->setValue( token, value );
            break;
        }

        _cmdResult += "\n";
    }

    if ( p_beg == p_end )
    {
        _cmdResult += "could not find requested setting name '" + reqtoken + "'. check for right spelling!\n";
    }

    return _cmdResult;
}

} // namespace CTD
