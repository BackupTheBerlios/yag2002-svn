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
 # console command for sending network notification, useful only 
 #  on server.
 #
 #   date of creation:  08/11/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdnetworksendnotify.h"
#include "../../maintenance/vrc_networknotification.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdNetworkSendNotify )


CmdNetworkSendNotify::CmdNetworkSendNotify() :
 BaseConsoleCommand( CMD_NAME_NETWORKSENDNOTIFY )
{
    setUsage( CMD_USAGE_NETWORKSENDNOTIFY );
}

CmdNetworkSendNotify::~CmdNetworkSendNotify()
{
}

const std::string& CmdNetworkSendNotify::execute( const std::vector< std::string >& arguments )
{  
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server )
    {
        _cmdResult = "this command can be executed only in server mode!\n";
        return _cmdResult;
    }

    _cmdResult = "";
    if ( arguments.size() < 2 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    EnNetworkNotification* p_entity = dynamic_cast< EnNetworkNotification* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_NETWORKNOTIFICATION ) );
    if ( !p_entity )
    {
        log_error << "cmd could not find entity of type 'EnNetworkNotification'" << std::endl;
        _cmdResult = "error executing cmd '"  CMD_NAME_NETWORKSENDNOTIFY  "'\n";
        return _cmdResult;
    }

    float destructiontimeout = 0.0f;
    if ( arguments.size() > 2 )
    {
        std::stringstream str;
        str << arguments[ 2 ];
        str >> destructiontimeout;
    }

    p_entity->sendNotification( arguments[ 0 ], arguments[ 1 ], destructiontimeout );

    _cmdResult = "sending out the notification ...";

    return _cmdResult;
}

} // namespace vrc
