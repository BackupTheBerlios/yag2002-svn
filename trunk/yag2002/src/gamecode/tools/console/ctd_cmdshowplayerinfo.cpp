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
 # console command for showing / hiding player info display
 #
 #   date of creation:  06/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdshowplayerinfo.h"
#include "../ctd_playerinfodisplay.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdShowPlayerInfo );


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

    if ( !arguments.size() )
    {
        _cmdResult = "* error executing command '" + string( CMD_NAME_SHOWPLAYERINFO ) + "'\n ";
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
        _cmdResult = "* error executing command '" + string( CMD_NAME_SHOWPLAYERINFO ) + "'\n ";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    if ( !_p_playerInfo )
    {
        // first check if the entity entity already exists
        _p_playerInfo = static_cast< EnPlayerInfoDisplay* >( EntityManager::get()->findEntity( ENTITY_NAME_PLAYERINFODISPLAY ) );
        if ( !_p_playerInfo ) // now we create one
        {
            _p_playerInfo = static_cast< EnPlayerInfoDisplay* >( EntityManager::get()->createEntity( ENTITY_NAME_PLAYERINFODISPLAY, "_playerinfo_" ) );
            assert( _p_playerInfo && "error creating EnPlayerInfo entity!" );
            _p_playerInfo->initialize();
            _p_playerInfo->postInitialize();
        }

    }

    _p_playerInfo->enable( enable );
    
    return _cmdResult;
}

} // namespace CTD