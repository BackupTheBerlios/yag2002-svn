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
 # console command for finalizing level loading
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CMDLOADFINALIZE_H_
#define _VRC_CMDLOADFINALIZE_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_LOADFINALIZE    "load.finalize"
#define CMD_USAGE_LOADFINALIZE   "finalize the process of level loading. you muse have called 'load.level' before!\n"\
                                 "use: load.finalize"

class BaseConsoleCommand;

//! Class for command 'load.finalize'
class CmdLoadFinalize : public BaseConsoleCommand
{
    public:
                                                    CmdLoadFinalize();

        virtual                                     ~CmdLoadFinalize();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDLOADFINALIZE_H_
