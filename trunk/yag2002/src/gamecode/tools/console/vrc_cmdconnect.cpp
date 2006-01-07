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

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdconnect.h"
#include <vrc_gameutils.h>

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdConnect );


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
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, clientname );
    yaf3d::NodeInfo nodeinfo( "", clientname );
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );
    // parse the argument for ip and channel information
    if ( arguments.size() > 0 )
    {
        std::string addr = arguments[ 0 ];
        std::vector< std::string > ip;
        yaf3d::explode( addr, ":", &ip );
        if ( ip.size() > 1 )
        {
            std::stringstream chan;
            chan << ip[ 1 ];
            chan >> channel;
            addr = ip[ 0 ];
            ip.clear();
        }
        yaf3d::explode( addr, ".", &ip );
        if ( ip.size() < 4 )
        {
            _cmdResult = "the ip address must have the format 'X.X.X.X'";
            return _cmdResult;
        }
        serverip = addr;
    }

    // try to setup client
    try
    {
        yaf3d::NetworkDevice::get()->setupClient( serverip, channel, nodeinfo );
    }
    catch ( yaf3d::NetworkExcpection& e )
    {
        _cmdResult = "* cannot connect to server: '" + serverip + "', reason: " + e.what();
        return _cmdResult;
    }

    // set the game mode to Client before loading the level
    yaf3d::GameState::get()->setMode( yaf3d::GameState::Client );
    std::string playerCfgFile;
    // load local client's config ( .lvl file ) 
    if ( !vrc::gameutils::PlayerUtils::get()->getPlayerConfig( yaf3d::GameState::Client, false, playerCfgFile ) )
    {
        _cmdResult = "* error: cannot determine player configuration file";
        return _cmdResult;
    }

    // now prepare loading level
    _cmdResult += "unload level ...\n";
    yaf3d::LevelManager::get()->unloadLevel( true, true );
    _cmdResult += "loading level '" + yaf3d::NetworkDevice::get()->getNodeInfo()->getLevelName() + "' ...\n";
    yaf3d::LevelManager::get()->loadLevel( YAF3D_LEVEL_CLIENT_DIR + yaf3d::NetworkDevice::get()->getNodeInfo()->getLevelName() );
    // now load the player
    std::vector< yaf3d::BaseEntity* > entities;
    _cmdResult += "loading player '" + playerCfgFile + "' ...\n";
    yaf3d::LevelManager::get()->loadEntities( playerCfgFile );
    // complete level loading
    _cmdResult += "finalizing level loading ...\n";
    yaf3d::LevelManager::get()->finalizeLoading();

    _cmdResult += "starting client ...\n";

    // try to start the networking
    try
    {
        yaf3d::NetworkDevice::get()->startClient();
    }
    catch ( yaf3d::NetworkExcpection& e )
    {
        _cmdResult = std::string( "* cannot start client, reason: " ) + e.what();
        return _cmdResult;
    }

    _cmdResult += "connection successful\n";
    return _cmdResult;
}

} // namespace vrc
