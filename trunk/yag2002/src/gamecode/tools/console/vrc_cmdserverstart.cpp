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
 # console command for starting a server
 #
 #   date of creation:  06/15/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdserverstart.h"

namespace vrc
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdServerStart );


CmdServerStart::CmdServerStart() :
 BaseConsoleCommand( CMD_NAME_SERVERSTART )
{
    setUsage( CMD_USAGE_SERVERSTART );
}

CmdServerStart::~CmdServerStart()
{
}

const std::string& CmdServerStart::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    if ( arguments.size() < 1 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    // get the full binary path
    std::string cmd = yaf3d::Application::get()->getFullBinPath();
    std::string levelfile = arguments[ 0 ];
    std::string arg1( "-server" );
    std::string arg2( "-level" );
    std::string arg3( levelfile );

    // use utility function to start the server
    std::string args = arg1 + "  " + arg2 + "  " + arg3;
    SPAWN_PROC_ID serverProcHandle = yaf3d::spawnApplication( cmd, args );

    if ( serverProcHandle )
        _cmdResult = "trying to start server with level file '" + levelfile + "'.";
    else
        _cmdResult = "* error starting server with level file '" + levelfile + "'.";

    return _cmdResult;
}

} // namespace vrc
