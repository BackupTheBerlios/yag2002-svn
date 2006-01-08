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
 # console command for showing the version information
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_basecmd.h"
#include "vrc_cmdversion.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdVersion );


CmdVersion::CmdVersion() :
 BaseConsoleCommand( CMD_NAME_VERSION )
{
    setUsage( CMD_USAGE_VERSION );
}

CmdVersion::~CmdVersion()
{
}

const std::string& CmdVersion::execute( const std::vector< std::string >& arguments )
{
    _cmdResult =  "Virtual Reality Chat (VRC)\n";     
    _cmdResult += "VRC version:   '" + std::string( VRC_VERSION ) + "'\n";
    _cmdResult += "YAF3D version: '" + std::string( YAF3D_VERSION ) + "'\n";
    _cmdResult += "site: http://yag2002.sourceforge.net\n";
    _cmdResult += "contact: botorabi@gmx.net\n";
    return _cmdResult;
}

} // namespace vrc
