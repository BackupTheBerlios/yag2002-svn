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
 # console command for loading entity description files
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDLOADENTITIES_H_
#define _CTD_CMDLOADENTITIES_H_

#include <ctd_main.h>

namespace CTD
{
// console command name
#define CMD_NAME_LOADENTITIES    "load.entities"
#define CMD_USAGE_LOADENTITIES   "load an entity description file. use -init for initializing loaded entities. "\
                                 "an optional post fix can be appended to all instance names.\n"\
                                 "use: load.entities [-init] < entity file > [ post fix ]"

class BaseConsoleCommand;

//! Class for command 'load.entities'
class CmdLoadEntities : public BaseConsoleCommand
{
    public:
                                                    CmdLoadEntities();

        virtual                                     ~CmdLoadEntities();

        const std::string&                          execute( const std::vector< std::string >& argmuments );
};



} // namespace CTD

#endif // _CTD_CMDLOADENTITIES_H_
