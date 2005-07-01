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
 # console command 'help'
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDHELP_H_
#define _CTD_CMDHELP_H_

#include <ctd_main.h>

namespace CTD
{
// console command name
#define CMD_NAME_HELP    "help"
#define CMD_USAGE_HELP   "print help of given command. if a command name is missing then all available commands are printed.\n"\
                         "use option -d to get a detailes command list.\n"\
                         "use: help [-d][ command name ]"

class BaseConsoleCommand;

//! Class for command 'help'
class CmdHelp : public BaseConsoleCommand
{
    public:
                                                    CmdHelp();

        virtual                                     ~CmdHelp();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace CTD

#endif // _CTD_CMDHELP_H_
