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
 # console command for showing / hiding fps display
 #
 #   date of creation:  13/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdshowfps.h"
#include "../ctd_fpsdisplay.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdShowFps );


CmdShowFps::CmdShowFps() :
 BaseConsoleCommand( CMD_NAME_SHOWFPS )
{
    setUsage( CMD_USAGE_SHOWFPS );
}

CmdShowFps::~CmdShowFps()
{
}

const std::string& CmdShowFps::execute( const std::vector< std::string >& argmuments )
{
    _cmdResult = "";

    if ( !argmuments.size() )
    {
        _cmdResult = "* error executing command '" + string( CMD_NAME_SHOWFPS ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    bool enable = false;
    if ( argmuments[ 0 ] == "true" )
        enable = true;
    else if ( argmuments[ 0 ] == "false" )
        enable = false;
    else 
    {
        _cmdResult = "* error executing command '" + string( CMD_NAME_SHOWFPS ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    // first check if the fps entity already exists
    EnFPSDisplay* p_fps = static_cast< EnFPSDisplay* >( EntityManager::get()->findEntity( ENTITY_NAME_FPSDISPLAY ) );
    if ( p_fps )
    {
        p_fps->enable( enable );
    }
    else
    {
        if ( enable )
        {
            p_fps = static_cast< EnFPSDisplay* >( EntityManager::get()->createEntity( ENTITY_NAME_FPSDISPLAY, "_fps_" ) );
            p_fps->initialize();
            p_fps->postInitialize();
            p_fps->enable( enable );    
        }
    }

    return _cmdResult;
}

} // namespace CTD
