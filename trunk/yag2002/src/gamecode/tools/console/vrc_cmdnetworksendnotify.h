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

#ifndef _VRC_CMDNETWORKSENDNOTIFY_H_
#define _VRC_CMDNETWORKSENDNOTIFY_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_NETWORKSENDNOTIFY    "network.send.notification"
#define CMD_USAGE_NETWORKSENDNOTIFY   "send a notification to all connected clients\n"\
                                      " if a destruction timeout is given then the notification messagebox\n"\
                                      " automatically disappears after that timeout, -1 means no auto-destruction,\n"\
                                      " 0 of missing destuction timeout in argument causes taking the default timeout.\n"\
                                      "\n"\
                                      "use: network.send.notification < title > < text > [ destruction timeout in sec: > 0, -1, or 0 ]"

class EnConsole;
class BaseConsoleCommand;

//! Class for command 'network.send.notification'
class CmdNetworkSendNotify : public BaseConsoleCommand
{
    public:
                                                    CmdNetworkSendNotify();

        virtual                                     ~CmdNetworkSendNotify();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDNETWORKSENDNOTIFY_H_
