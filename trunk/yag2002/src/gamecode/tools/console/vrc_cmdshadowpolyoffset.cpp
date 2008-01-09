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
 # console command for setting and displaying shadow polygon offset
 #
 #   date of creation:  01/09/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_basecmd.h"
#include "vrc_cmdshadowpolyoffset.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdShadowPolygonOffset )


CmdShadowPolygonOffset::CmdShadowPolygonOffset() :
 BaseConsoleCommand( CMD_NAME_SHADOWPOLYOFFSET )
{
    setUsage( CMD_USAGE_SHADOWPOLYOFFSET );
}

CmdShadowPolygonOffset::~CmdShadowPolygonOffset()
{
}

const std::string& CmdShadowPolygonOffset::execute( const std::vector< std::string >& arguments )
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
        _cmdResult += "dynamic shadows are not enabled! this command has no effect.";
        return _cmdResult;
    }

    if ( arguments[ 0 ] == "info" )
    {
        std::stringstream str;
        float factor = 0.0f;
        float units  = 0.0f;
        yaf3d::ShadowManager::get()->getPolygonOffset( factor, units );
        str << "factor: " << factor << " " << "units: " << units;
        _cmdResult = str.str();
        return _cmdResult;
    }
    else if ( arguments[ 0 ] == "set" )
    {
        if ( arguments.size() < 2 )
        {
            _cmdResult += getUsage();
            return _cmdResult;
        }

        std::stringstream str;
        float factor = 0.0f;
        float units  = 0.0f;
        str << arguments[ 1 ];
        str >> factor >> units;
        yaf3d::ShadowManager::get()->setPolygonOffset( factor, units );
    }
    else
    {
        _cmdResult = getUsage();
    }

    return _cmdResult;
}

} // namespace vrc
