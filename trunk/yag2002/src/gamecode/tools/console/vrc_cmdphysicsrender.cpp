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
 # console command for debug rendering the physics collision geoms
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdphysicsrender.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdPhysicsRender )


CmdPhysicsRender::CmdPhysicsRender() :
 BaseConsoleCommand( CMD_NAME_PHYSICSRENDER ),
 _enableDebugRendering( false )
{
    setUsage( CMD_USAGE_PHYSICSRENDER );
}

CmdPhysicsRender::~CmdPhysicsRender()
{
}

const std::string& CmdPhysicsRender::execute( const std::vector< std::string >& arguments )
{
    _cmdResult =  "";

    _enableDebugRendering = !_enableDebugRendering;
    if ( _enableDebugRendering )
        yaf3d::Physics::get()->enableDebugRender();
    else
        yaf3d::Physics::get()->disableDebugRender();

    return _cmdResult;
}

} // namespace vrc
