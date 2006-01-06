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
 # console command for creating a log channel for console output
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdlogcreate.h"
#include "vrc_console.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdLogCreate );


CmdLogCreate::CmdLogCreate() :
 BaseConsoleCommand( CMD_NAME_LOGCREATE )
{
    setUsage( CMD_USAGE_LOGCREATE );
}

CmdLogCreate::~CmdLogCreate()
{
}

const std::string& CmdLogCreate::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    bool append = false;
    std::string logfile;
    if ( arguments.size() < 1 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
    else
    {
        if ( arguments.size() > 1 )
        {
            if ( arguments[ 0 ] == "-a" )
            {
                logfile = arguments[ 1 ];
                append = true;
            }
            else
            {
                _cmdResult = "* unknown option " + arguments[ 0 ] + "\n";
                _cmdResult += getUsage();
                return _cmdResult;
            }
        }
        else
            logfile = arguments[ 0 ];
    }

    EnConsole* p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
    if ( !p_console->createLog( logfile, append ) )
    {
        _cmdResult = "* cannot create log file '" + logfile + "', check the file path!";
        return _cmdResult;
    }

    return _cmdResult;
}

} // namespace vrc
