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
 # console command for getting an entity attribute list
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDENTITYATTRLIST_H_
#define _CTD_CMDENTITYATTRLIST_H_

#include <ctd_main.h>

namespace CTD
{
// console command name
#define CMD_NAME_ENTITYATTRLIST   "entity.attribute.list"
#define CMD_USAGE_ENTITYATTRLIST  "get all entity attributes for given entity instance\n"\
                                  "use: entity.attribute.list < instance name >"

class BaseConsoleCommand;

//! Class for command 'entity.attribute.list'
class CmdEntityAttributeList : public BaseConsoleCommand
{
    public:
                                                    CmdEntityAttributeList();

        virtual                                     ~CmdEntityAttributeList();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace CTD

#endif // _CTD_CMDENTITYATTRLIST_H_
