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
 # console command for pushing idle steps into command processing queue
 #
 #   date of creation:  06/23/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CMDIDLE_H_
#define _VRC_CMDIDLE_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_IDLE    "idle"
#define CMD_USAGE_IDLE   "idle the command execution for given amount of steps\n"\
                          "use: idle < count of steps >"

class EnConsole;
class BaseConsoleCommand;

//! Class for command 'idle'
class CmdIdle : public BaseConsoleCommand
{
    public:
                                                    CmdIdle();

        virtual                                     ~CmdIdle();

        const std::string&                          execute( const std::vector< std::string >& arguments );

    protected:

        EnConsole*                                  _p_console;
};



} // namespace vrc

#endif // _VRC_CMDIDLE_H_
