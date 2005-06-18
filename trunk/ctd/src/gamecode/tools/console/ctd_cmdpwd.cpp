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
 # console command for showing current working directory
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdpwd.h"
#include "ctd_console.h"

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdPwd );


CmdPwd::CmdPwd() :
 BaseConsoleCommand( CMD_NAME_PWD ),
 _p_console( NULL )
{
    setUsage( CMD_USAGE_PWD );
}

CmdPwd::~CmdPwd()
{
}

const std::string& CmdPwd::execute( const std::vector< std::string >& arguments )
{
    if ( !_p_console )
    {
        _p_console = static_cast< EnConsole* >( EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
        assert( _p_console && "CmdExec::execute: console entity could not be found!" );
    }

    _cmdResult = _p_console->getCWD();
    return _cmdResult;
}

} // namespace CTD
