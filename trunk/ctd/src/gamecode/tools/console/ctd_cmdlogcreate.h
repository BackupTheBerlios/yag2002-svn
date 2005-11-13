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
 # console command for creating a yaf3d::log channel for console output
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDLOGCREATE_H_
#define _CTD_CMDLOGCREATE_H_

#include <ctd_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_LOGCREATE    "yaf3d::log.create"
#define CMD_USAGE_LOGCREATE   "direct the console output to given yaf3d::log file. specify option -a to append the yaf3d::log to an already existing file.\n"\
                              "use: yaf3d::log.create [ -a ] < yaf3d::log file name >"

class BaseConsoleCommand;

//! Class for command 'yaf3d::log.create'
class CmdLogCreate : public BaseConsoleCommand
{
    public:
                                                    CmdLogCreate();

        virtual                                     ~CmdLogCreate();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _CTD_CMDLOGCREATE_H_
