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
 # console command for starting a server
 #
 #   date of creation:  15/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDSTARTSERVER_H_
#define _CTD_CMDSTARTSERVER_H_

#include <ctd_main.h>

namespace CTD
{
// console command name
#define CMD_NAME_STARTSERVER    "start.server"
#define CMD_USAGE_STARTSERVER   "start a server with given level file. the level file path is determined by application itself.\n"\
                                "use: start.server < level file >"

class BaseConsoleCommand;

//! Class for command 'start.server'
class CmdStartServer : public BaseConsoleCommand
{
    public:
                                                    CmdStartServer();

        virtual                                     ~CmdStartServer();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace CTD

#endif // _CTD_CMDSTARTSERVER_H_
