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
 # console command 'help'
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdhelp.h"
#include "ctd_cmdregistry.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdHelp );


CmdHelp::CmdHelp() :
 BaseConsoleCommand( CMD_NAME_HELP )
{
    setUsage( CMD_USAGE_HELP );
}

CmdHelp::~CmdHelp()
{
}

const std::string& CmdHelp::execute( const std::vector< std::string >& argmuments )
{
    _cmdResult = "possible commands: \n";
    std::vector< std::string > cmds;
    ConsoleCommandRegistry::get()->getAllCmds( cmds );
    std::vector< std::string >::iterator p_beg = cmds.begin(), p_end = cmds.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        _cmdResult += ( *p_beg ) + "  ";
    }
    _cmdResult += "\n";
    return _cmdResult;
}

} // namespace CTD
