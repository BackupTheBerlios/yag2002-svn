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
 # console command for listing all available levels
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CMDLEVELLIST_H_
#define _VRC_CMDLEVELLIST_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_LEVELLIST    "level.list"
#define CMD_USAGE_LEVELLIST   "list all available levels.\n"\
                              "use: level.list"

class BaseConsoleCommand;

//! Class for command 'level.list'
class CmdLevelList : public BaseConsoleCommand
{
    public:
                                                    CmdLevelList();

        virtual                                     ~CmdLevelList();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDLEVELLIST_H_
