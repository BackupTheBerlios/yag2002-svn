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

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdphysics.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdPhysics );


CmdPhysics::CmdPhysics() :
 BaseConsoleCommand( CMD_NAME_PHYSICS ),
 _enableDebugRendering( false )
{
    setUsage( CMD_USAGE_PHYSICS );
}

CmdPhysics::~CmdPhysics()
{
}

const std::string& CmdPhysics::execute( const std::vector< std::string >& arguments )
{
    _cmdResult =  "";
    if ( ( arguments.size() < 1 ) || ( arguments[ 0 ] != "-vs" ) )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
    _enableDebugRendering = !_enableDebugRendering;
    if ( _enableDebugRendering )
        Physics::get()->enableDebugRender();
    else
        Physics::get()->disableDebugRender();

    return _cmdResult;
}

} // namespace CTD
