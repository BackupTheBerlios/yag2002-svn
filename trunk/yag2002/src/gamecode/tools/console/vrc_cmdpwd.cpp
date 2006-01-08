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
 # console command for showing current working directory
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdpwd.h"
#include "vrc_console.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdPwd );


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
        _p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
        assert( _p_console && "CmdExec::execute: console entity could not be found!" );
    }

    _cmdResult = _p_console->getCWD();
    return _cmdResult;
}

} // namespace vrc
