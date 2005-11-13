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
 # console command for listing files in a given folder (relative to
 #  media folder in file system)
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdls.h"
#include "vrc_console.h"

namespace vrc
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdLs );


CmdLs::CmdLs() :
 BaseConsoleCommand( CMD_NAME_LEVELLIST ),
 _p_console( NULL )
{
    setUsage( CMD_USAGE_LEVELLIST );
}

CmdLs::~CmdLs()
{
}

const std::string& CmdLs::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( !_p_console )
    {
        _p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
        assert( _p_console && "CmdExec::execute: console entity could not be found!" );
    }

    std::string dir;
    bool appenddetails = false;
    if ( arguments.size() > 0 )
    {
        if ( arguments.size() > 1 )
        {
            if ( arguments[ 0 ] == "-l" )
            {
                appenddetails = true;
                dir = arguments[ 1 ];
            }
            else
            {
                _cmdResult = getUsage();
                return _cmdResult;
            }
        }
        else
        {
            if ( arguments[ 0 ] == "-l" )
            {
                appenddetails = true;
                dir = _p_console->getCWD();
            }
            else
            {
                dir = arguments[ 0 ];
            }
        }
    }
    else
    {
        dir = _p_console->getCWD();
    }

    // show the content of the directory
    std::vector< std::string > filelisting;
    // get a directory listing
    yaf3d::getDirectoryListing( filelisting, yaf3d::Application::get()->getMediaPath() + dir, "", appenddetails );
    std::vector< std::string >::iterator p_beg = filelisting.begin(), p_end = filelisting.end();
    for ( ; p_beg != p_end; p_beg++ )
        _cmdResult += *p_beg + "\n";

    return _cmdResult;
}

} // namespace vrc
