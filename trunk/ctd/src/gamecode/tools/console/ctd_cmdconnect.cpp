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
 #   date of creation:  06/15/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdconnect.h"
#include <ctd_gameutils.h>

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
    
    unsigned int channel;
    std::string serverip;
    std::string clientname;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, clientname );
    NodeInfo nodeinfo( "", clientname );
    Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );
    // parse the argument for ip and channel information
    if ( arguments.size() > 0 )
    {
        std::string addr = arguments[ 0 ];
        std::vector< std::string > ip;
        explode( addr, ":", &ip );
        if ( ip.size() > 1 )
        {
            std::stringstream chan;
            chan << ip[ 1 ];
            chan >> channel;
            addr = ip[ 0 ];
            ip.clear();
        }
        explode( addr, ".", &ip );
        if ( ip.size() < 4 )
        {
            _cmdResult = "the ip address must have the format 'X.X.X.X'";
            return _cmdResult;
        }
        serverip = addr;
    }

    // try to join
    if ( !NetworkDevice::get()->setupClient( serverip, channel, nodeinfo ) )
    {
        _cmdResult = "* cannot connect to server: '" + serverip + "'";
        return _cmdResult;
    }

    // set the game mode to Client before loading the level
    GameState::get()->setMode( GameState::Client );
    std::string playerCfgFile;
    // load local client's config ( .lvl file ) 
    if ( !CTD::gameutils::PlayerUtils::get()->getPlayerConfig( GameState::Client, false, playerCfgFile ) )
    {
        _cmdResult = "* error: cannot determine player configuration file";
        return _cmdResult;
    }

    // now prepare loading level
    _cmdResult += "unload level ...\n";
    LevelManager::get()->unloadLevel( true, true );
    _cmdResult += "loading level '" + NetworkDevice::get()->getNodeInfo()->getLevelName() + "' ...\n";
    LevelManager::get()->loadLevel( CTD_LEVEL_CLIENT_DIR + NetworkDevice::get()->getNodeInfo()->getLevelName() );
    // now load the player
    std::vector< BaseEntity* > entities;
    _cmdResult += "loading player '" + playerCfgFile + "' ...\n";
    LevelManager::get()->loadEntities( playerCfgFile );
    // complete level loading
    _cmdResult += "finalizing level loading ...\n";
    LevelManager::get()->finalizeLoading();

    _cmdResult += "starting client ...\n";
    if ( !NetworkDevice::get()->startClient() )
    {
        _cmdResult = "* cannot start client";
        return _cmdResult;
    }

    _cmdResult += "connection successful\n";
    return _cmdResult;
}

} // namespace CTD
