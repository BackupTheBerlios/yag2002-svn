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
 # console command for shutting down the application
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdshutdown.h"
#include "vrc_console.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdShutdown );


CmdShutdown::CmdShutdown() :
 BaseConsoleCommand( CMD_NAME_SHUTDOWN )
{
    setUsage( CMD_USAGE_SHUTDOWN );
}

CmdShutdown::~CmdShutdown()
{
}

const std::string& CmdShutdown::execute( const std::vector< std::string >& arguments )
{
    float fdelay = 0;
    if ( arguments.size() > 0 )
    {
        std::stringstream delay;
        delay << arguments[ 0 ];
        delay >> fdelay;
    }
    if ( fdelay > 0 )
        _cmdResult = "shutting down the application in " + arguments[ 0 ] + " seconds ...";
    else
        _cmdResult = "shutting down the application now ...";

    // begin the shutdown process
    EnConsole* p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
    p_console->triggerShutdown( fdelay );

    return _cmdResult;
}

} // namespace vrc
