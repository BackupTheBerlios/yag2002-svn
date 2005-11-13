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
 # console command for listing files in a given folder (relative to
 #  media folder in file system)
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CMDLS_H_
#define _VRC_CMDLS_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_LEVELLIST    "ls"
#define CMD_USAGE_LEVELLIST   "list all files in given or current directory. use option -l to get file / directory details.\n"\
                              "use: ls [ -l ] [ directory ]"

class EnConsole;
class BaseConsoleCommand;

//! Class for command 'ls'
class CmdLs : public BaseConsoleCommand
{
    public:
                                                    CmdLs();

        virtual                                     ~CmdLs();

        const std::string&                          execute( const std::vector< std::string >& arguments );

    protected:

        EnConsole*                                  _p_console;
};



} // namespace vrc

#endif // _VRC_CMDLS_H_
