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

#ifndef _CTD_CMDCONNECT_H_
#define _CTD_CMDCONNECT_H_

#include <ctd_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_CONNECT    "connect"
#define CMD_USAGE_CONNECT   "connect to a server. if the ip address is not specified then the local net is searched for a server.\n"\
                            "if the channel is omitted then the channel specified in game configuration is taken.\n"\
                            "use: connect [ server ip address [: channel ] ]"

class BaseConsoleCommand;

//! Class for command 'connect'
class CmdConnect : public BaseConsoleCommand
{
    public:
                                                    CmdConnect();

        virtual                                     ~CmdConnect();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _CTD_CMDCONNECT_H_
