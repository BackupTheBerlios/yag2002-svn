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
 # console command for serializing the physics static collision geoms
 #
 #   date of creation:  10/07/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdphysicsserialize.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdPhysicsSerialize );


CmdPhysicsSerialize::CmdPhysicsSerialize() :
 BaseConsoleCommand( CMD_NAME_PHYSICSSERIALIZE )
{
    setUsage( CMD_USAGE_PHYSICSSERIALIZE );
}

CmdPhysicsSerialize::~CmdPhysicsSerialize()
{
}

const std::string& CmdPhysicsSerialize::execute( const std::vector< std::string >& arguments )
{
    _cmdResult =  "";
    if ( arguments.size() < 2 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    if ( !yaf3d::Physics::get()->serialize( arguments[ 0 ], arguments[ 1 ] ) )
    {
        _cmdResult = "error occured during serialization";
        return _cmdResult;
    }

    _cmdResult = "serialization successful, output file created: '" + yaf3d::Application::get()->getMediaPath() + arguments[ 1 ] + "'";

    return _cmdResult;
}

} // namespace vrc
