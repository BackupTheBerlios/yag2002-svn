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
 # console command for creating a log channel for console output
 #
 #   date of creation:  06/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CMDLOGCREATE_H_
#define _VRC_CMDLOGCREATE_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_LOGCREATE    "log.create"
#define CMD_USAGE_LOGCREATE   "direct the console output to given log file. specify option -a to append the log to an already existing file.\n"\
                              "use: log.create [ -a ] < log file name >"

class BaseConsoleCommand;

//! Class for command 'log.create'
class CmdLogCreate : public BaseConsoleCommand
{
    public:
                                                    CmdLogCreate();

        virtual                                     ~CmdLogCreate();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDLOGCREATE_H_
