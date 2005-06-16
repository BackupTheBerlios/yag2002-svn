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
 # console command for connecting to a server
 #
 #   date of creation:  15/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdconnect.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdConnect );


CmdConnect::CmdConnect() :
 BaseConsoleCommand( CMD_NAME_CONNECT )
{
    setUsage( CMD_USAGE_CONNECT );
}

CmdConnect::~CmdConnect()
{
}

const std::string& CmdConnect::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    std::string serverip;
    if ( arguments.size() > 0 )
        serverip = arguments[ 0 ];

    string clientname;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, clientname );
    NodeInfo nodeinfo( "", clientname );
    unsigned int channel;
    Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );

    // try to join
    if ( !NetworkDevice::get()->setupClient( serverip, channel, nodeinfo ) )
    {
        _cmdResult = "* cannot connect to server: '" + serverip + "'";
        return _cmdResult;
    }

    // set the game mode to Client before loading the level
    GameState::get()->setMode( GameState::Client );
    // now prepare loading level
    LevelManager::get()->unloadLevel( true, true );
    LevelManager::get()->loadLevel( CTD_LEVEL_CLIENT_DIR + NetworkDevice::get()->getNodeInfo()->getLevelName() );
    // now load the player
//string playerCfgFile = getPlayerConfig( GameState::get()->getMode() );
//std::vector< BaseEntity* > entities;
//LevelManager::get()->loadEntities( playerCfgFile );
//// complete level loading
//LevelManager::get()->finalizeLoading();

    if ( !NetworkDevice::get()->startClient() )
    {
        _cmdResult = "* cannot start client";
        return _cmdResult;
    }

    return _cmdResult;
}

} // namespace CTD
