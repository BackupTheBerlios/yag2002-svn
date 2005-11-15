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
 # console command for changing current working directory
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdcd.h"
#include "vrc_console.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdCd );


CmdCd::CmdCd() :
 BaseConsoleCommand( CMD_NAME_CD ),
 _p_console( NULL )
{
    setUsage( CMD_USAGE_CD );
}

CmdCd::~CmdCd()
{
}

const std::string& CmdCd::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    if ( !_p_console )
    {
        _p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
        assert( _p_console && "CmdExec::execute: console entity could not be found!" );
    }

    if ( arguments.size() < 1 )
    {
        _p_console->setCWD( "" ); 
    }
    else
    {
        std::string dir = arguments[ 0 ];
        if ( !dir.length() )
            dir = yaf3d::Application::get()->getMediaPath();
        else if ( dir == ".." )
        {
            dir = _p_console->getCWD();
            dir.erase( dir.rfind( "/" ) );
        }
        else if ( dir == "." )
            dir = _p_console->getCWD();
        else if ( dir[ 0 ] != '/' )
        {
            if ( _p_console->getCWD() != "/" )
                dir = _p_console->getCWD() + "/" + dir;
            else
                dir = "/" + dir;
        }

        if ( !_p_console->setCWD( dir ) )
        {
            _cmdResult = "* invalid path";
            return _cmdResult;
        }
    }
    return _cmdResult;
}

} // namespace vrc
