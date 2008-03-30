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
 # console command for showing / hiding player info display
 #
 #   date of creation:  06/28/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdshowplayerinfo.h"
#include "../vrc_playerinfodisplay.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdShowPlayerInfo )


CmdShowPlayerInfo::CmdShowPlayerInfo() :
 BaseConsoleCommand( CMD_NAME_SHOWPLAYERINFO ),
 _p_playerInfo( NULL )
{
    setUsage( CMD_USAGE_SHOWPLAYERINFO );
}

CmdShowPlayerInfo::~CmdShowPlayerInfo()
{
}

const std::string& CmdShowPlayerInfo::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        _cmdResult = "this command is not available in server mode";
        return _cmdResult;
    }

    if ( !arguments.size() )
    {
        _cmdResult = "* error executing command '" + std::string( CMD_NAME_SHOWPLAYERINFO ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    bool enable = false;
    if ( arguments[ 0 ] == "true" )
        enable = true;
    else if ( arguments[ 0 ] == "false" )
        enable = false;
    else
    {
        _cmdResult = "* error executing command '" + std::string( CMD_NAME_SHOWPLAYERINFO ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    if ( !_p_playerInfo )
    {
        // first check if the entity entity already exists
        _p_playerInfo = static_cast< EnPlayerInfoDisplay* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLAYERINFODISPLAY ) );
        if ( !_p_playerInfo ) // now we create one
        {
            _p_playerInfo = static_cast< EnPlayerInfoDisplay* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_PLAYERINFODISPLAY, "_playerinfo_" ) );
            assert( _p_playerInfo && "error creating EnPlayerInfo entity!" );
            _p_playerInfo->initialize();
            _p_playerInfo->postInitialize();
        }

    }

    _p_playerInfo->enable( enable );

    return _cmdResult;
}

} // namespace vrc
