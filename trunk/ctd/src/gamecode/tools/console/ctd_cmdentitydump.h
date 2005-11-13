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
 # console command for dumping entity information such as type
 #
 #  and parameters
 #
 #   date of creation:  08/15/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDENTITYDUMP_H_
#define _CTD_CMDENTITYDUMP_H_

#include <ctd_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_ENTITYDUMP    "entity.dump"
#define CMD_USAGE_ENTITYDUMP   "dump entity information to a given file. use option -all to dump all available entities.\n"\
                               "attention: for stability reasons the application will shutdown after exporting the entity dump using option -all.\n"\
                               "use: entity.dump < -all | entity instance name > < dump file name >"

class EnConsole;
class BaseConsoleCommand;

//! Class for command 'entity.dump'
class CmdEntityDump : public BaseConsoleCommand
{
    public:
                                                    CmdEntityDump();

        virtual                                     ~CmdEntityDump();

        const std::string&                          execute( const std::vector< std::string >& arguments );

    protected:

        std::string                                 dumpEntity( yaf3d::BaseEntity* p_entity, unsigned int creationPolicy );
};

} // namespace vrc

#endif // _CTD_CMDENTITYDUMP_H_
