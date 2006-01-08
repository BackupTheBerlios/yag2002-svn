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
 # console command 'help'
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdhelp.h"
#include "vrc_cmdregistry.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdHelp );


CmdHelp::CmdHelp() :
 BaseConsoleCommand( CMD_NAME_HELP )
{
    setUsage( CMD_USAGE_HELP );
}

CmdHelp::~CmdHelp()
{
}

const std::string& CmdHelp::execute( const std::vector< std::string >& arguments )
{
    bool detail = false;
    if ( arguments.size() > 0 )
    {
        if ( arguments[ 0 ] == "-d" )
        {
            detail = true;
        }
        else
        {
            BaseConsoleCommand* p_cmd = ConsoleCommandRegistry::get()->getCmd( arguments[ 0 ] );
            if ( !p_cmd )
            {
                _cmdResult = "* command or option '" + arguments[ 0 ] + "' does not exist.\n";
                _cmdResult += "usage: " + getUsage();
                return _cmdResult;
            }
            _cmdResult = p_cmd->getUsage();
            return _cmdResult;
        }
    }

    _cmdResult = "note: use option -d to get a detailed list.\n";
    _cmdResult += "available commands: \n";
    std::vector< BaseConsoleCommand* > cmds;
    ConsoleCommandRegistry::get()->getAllCmds( cmds );
    std::vector< BaseConsoleCommand* >::iterator p_beg = cmds.begin(), p_end = cmds.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( !detail )
        {
            _cmdResult += ( *p_beg )->getCmdName() + "  ";
        }
        else
        {
            _cmdResult += "[ " + ( *p_beg )->getCmdName() + " ]\n";
            _cmdResult += ( *p_beg )->getUsage() + "\n";
            _cmdResult += "\n";
        }
    }
    _cmdResult += "\n";

    return _cmdResult;
}

} // namespace vrc
