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
 # console command for getting an entity list
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDENTITYLIST_H_
#define _CTD_CMDENTITYLIST_H_

#include <ctd_main.h>

namespace CTD
{
// console command name
#define CMD_NAME_ENTITYLIST   "entity.list"
#define CMD_USAGE_ENTITYLIST  "get all currently existing entities and their flags (P persistent, T transformable, U is updated, N gets notifications )\n"\
                              "use: entity.list"

class BaseConsoleCommand;

//! Class for command 'entity.list'
class CmdEntityList : public BaseConsoleCommand
{
    public:
                                                    CmdEntityList();

        virtual                                     ~CmdEntityList();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace CTD

#endif // _CTD_CMDENTITYLIST_H_
