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
 # console command for showing the current configuration
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdgetconfig.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdGetConfig );


CmdGetConfig::CmdGetConfig() :
 BaseConsoleCommand( CMD_NAME_GETCONFIG )
{
    setUsage( CMD_USAGE_GETCONFIG );
}

CmdGetConfig::~CmdGetConfig()
{
}

const std::string& CmdGetConfig::execute( const std::vector< std::string >& arguments )
{

    Settings* p_settings = const_cast< Settings* >( Configuration::get()->getAllSettings() );
    std::vector< Settings::SettingBase* >& settingStorages = const_cast< std::vector< Settings::SettingBase* >& >( p_settings->getAllSettingStorages() );

    _cmdResult =  "setting file: " + string( CTD_GAMESETTING_FILENAME ) + "\n";
    _cmdResult += "-----------------\n";

    std::vector< Settings::SettingBase* >::iterator p_beg = settingStorages.begin(), p_end = settingStorages.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        string token = ( *p_beg )->getTokenName();
        _cmdResult += token;
        _cmdResult += " [ ";
        
        // get the setting value
        stringstream tokenvalue;
        const type_info& settings_typeinfo = ( *p_beg )->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) 
        {
            bool value;
            p_settings->getValue( token, value );
            tokenvalue << ( value ? "true" : "false" );
        } 
        else if ( settings_typeinfo == typeid( int ) ) 
        {
            int value;
            p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( unsigned int ) ) 
        {
            unsigned int value;
            p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( string ) ) 
        {
            string value;
            p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( float ) ) 
        {
            float value;
            p_settings->getValue( token, value );
            tokenvalue << value;
        }
        else if ( settings_typeinfo == typeid( osg::Vec3f ) ) 
        {
            osg::Vec3f value;
            p_settings->getValue( token, value );
            tokenvalue << value.x() << " " << value.y() << " " << value.z();
        }

        _cmdResult += tokenvalue.str();
        _cmdResult += " ]";
        _cmdResult += "\n";
    }

    return _cmdResult;
}

} // namespace CTD
