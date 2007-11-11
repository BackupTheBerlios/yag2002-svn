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
 # console command for unloading current level
 #
 #   date of creation:  06/15/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdunloadlevel.h"
#include <gui/vrc_menu.h>

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdUnloadLevel )


CmdUnloadLevel::CmdUnloadLevel() :
 BaseConsoleCommand( CMD_NAME_UNLOADLEVEL )
{
    setUsage( CMD_USAGE_UNLOADLEVEL );
}

CmdUnloadLevel::~CmdUnloadLevel()
{
}

const std::string& CmdUnloadLevel::execute( const std::vector< std::string >& /*arguments*/ )
{
    _cmdResult = "unloading level ...\n";

    // if the menu entity exists so let it do the job, otherwiese we unload the level outselves
    EnMenu* p_menu = static_cast< EnMenu* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_MENU ) );
    if ( p_menu )
    {
        yaf3d::EntityNotification ennotify( YAF3D_NOTIFY_UNLOAD_LEVEL );
        yaf3d::EntityManager::get()->sendNotification( ennotify, p_menu );
    }
    else
    {
        yaf3d::LevelManager::get()->unloadLevel( true, true );
    }

    _cmdResult += "unloading completed";
    return _cmdResult;
}

} // namespace vrc
