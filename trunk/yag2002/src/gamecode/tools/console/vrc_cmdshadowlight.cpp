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
 # console command for setting and displaying shadow light source
 #
 #   date of creation:  01/06/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_basecmd.h"
#include "vrc_cmdshadowlight.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdShadowLight )


CmdShadowLight::CmdShadowLight() :
 BaseConsoleCommand( CMD_NAME_SHADOWSHOWLIGHT )
{
    setUsage( CMD_USAGE_SHADOWSHOWLIGHT );
}

CmdShadowLight::~CmdShadowLight()
{
}

const std::string& CmdShadowLight::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( !arguments.size() )
    {
        _cmdResult += getUsage();
        return _cmdResult;
    }

    bool shadow = false;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, shadow );
    if ( !shadow )
    {
        _cmdResult += "dynamic shadows are not enabled! this command has no effect.";
        return _cmdResult;
    }

    if ( arguments[ 0 ] == "get" )
    {
        std::stringstream str;
        osg::Vec3f lightpos = yaf3d::ShadowManager::get()->getLightPosition();
        str << lightpos._v[ 0 ] << " " <<  lightpos._v[ 1 ] << " " << lightpos._v[ 2 ];
        _cmdResult = "light position: " + str.str();
        return _cmdResult;
    }
    else if ( arguments[ 0 ] == "pos" )
    {
        if ( arguments.size() < 2 )
        {
            _cmdResult += getUsage();
            return _cmdResult;
        }

        std::stringstream str;
        osg::Vec3f lightpos;
        str << arguments[ 1 ];
        str >> lightpos._v[ 0 ] >> lightpos._v[ 1 ] >> lightpos._v[ 2 ];
        yaf3d::ShadowManager::get()->setLightPosition( lightpos );
    }
    else
    {
        _cmdResult = getUsage();
    }

    return _cmdResult;
}

} // namespace vrc
