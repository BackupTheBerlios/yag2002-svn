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
 # console command for showing / hiding fps display
 #
 #   date of creation:  06/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdshowfps.h"
#include "../vrc_fpsdisplay.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdShowFps );


CmdShowFps::CmdShowFps() :
 BaseConsoleCommand( CMD_NAME_SHOWFPS )
{
    setUsage( CMD_USAGE_SHOWFPS );
}

CmdShowFps::~CmdShowFps()
{
}

const std::string& CmdShowFps::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( !arguments.size() )
    {
        _cmdResult = "* error executing command '" + std::string( CMD_NAME_SHOWFPS ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    bool enable = false;
    if ( arguments[ 0 ] == "true" )
        enable = true;
    else if ( arguments[ 0 ] == "false" )
        enable = false;
    else 
    {
        _cmdResult = "* error executing command '" + std::string( CMD_NAME_SHOWFPS ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    // first check if the fps entity already exists
    EnFPSDisplay* p_fps = static_cast< EnFPSDisplay* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_FPSDISPLAY ) );
    if ( p_fps )
    {
        p_fps->enable( enable );
    }
    else
    {
        if ( enable )
        {
            p_fps = static_cast< EnFPSDisplay* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_FPSDISPLAY, "_fps_" ) );
            p_fps->initialize();
            p_fps->postInitialize();
            p_fps->enable( enable );    
        }
    }

    return _cmdResult;
}

} // namespace vrc
