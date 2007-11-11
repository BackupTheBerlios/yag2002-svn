/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # console command for enabling / disabling the shadow map display
 #
 #   date of creation:  07/14/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_basecmd.h"
#include "vrc_cmdshadowdisplaymap.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdDisplayShadowMap )


CmdDisplayShadowMap::CmdDisplayShadowMap() :
 BaseConsoleCommand( CMD_NAME_SHADOWSHOWMAP )
{
    setUsage( CMD_USAGE_SHADOWSHOWMAP );
}

CmdDisplayShadowMap::~CmdDisplayShadowMap()
{
}

const std::string& CmdDisplayShadowMap::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( !arguments.size() )
    {
        _cmdResult += getUsage();
        return _cmdResult;
    }

    bool shadow;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, shadow );
    if ( !shadow )
    {
        _cmdResult += "dynamic shadows are not enabled! this command has not effect.";
        return _cmdResult;
    }

    if ( arguments[ 0 ] == "true" )
        yaf3d::ShadowManager::get()->displayShadowMap( true );
    else
        yaf3d::ShadowManager::get()->displayShadowMap( false );

    return _cmdResult;
}

} // namespace vrc
