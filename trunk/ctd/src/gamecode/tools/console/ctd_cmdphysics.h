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
 # console command for debugging the physics system
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDPHYSICS_H_
#define _CTD_CMDPHYSICS_H_

#include <ctd_main.h>

namespace CTD
{
// console command name
#define CMD_NAME_PHYSICS    "physics"
#define CMD_USAGE_PHYSICS   "use option -vs to toggle visualization of the static collision geometry via wireframes.\n"\
                            "use: physics -vs"

class BaseConsoleCommand;

//! Class for command 'physics'
class CmdPhysics : public BaseConsoleCommand
{
    public:
                                                    CmdPhysics();

        virtual                                     ~CmdPhysics();

        const std::string&                          execute( const std::vector< std::string >& arguments );

    protected:

        bool                                        _enableDebugRendering;
};



} // namespace CTD

#endif // _CTD_CMDPHYSICS_H_
