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
 # console command for executing a batch file containing console 
 #  commands
 #
 #   date of creation:  06/15/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdexec.h"
#include "ctd_console.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdExec );


CmdExec::CmdExec() :
 BaseConsoleCommand( CMD_NAME_EXEC ),
 _p_console( NULL )
{
    setUsage( CMD_USAGE_EXEC );
}

CmdExec::~CmdExec()
{
}

const std::string& CmdExec::execute( const std::vector< std::string >& arguments )
{  
    _cmdResult = "";

    // we have to avoid a reentrance which can occur when the exec command is used in a batch file!
    static s_lock = false;
    if ( s_lock )
    {
        _cmdResult = "command '" + std::string( CMD_NAME_EXEC ) + "' cannot be used in batch files! skipping";
        return _cmdResult;
    }
    // lock method
    s_lock = true;

    if ( arguments.size() < 1 )
    {
        _cmdResult = getUsage();
        s_lock = false;
        return _cmdResult;
    }

    std::string filename = Application::get()->getMediaPath() + arguments[ 0 ];
    std::fstream file;
    file.open( filename.c_str(), ios_base::binary | ios_base::in );
    if ( !file )
    {
        _cmdResult = "* cannot open batch file '" + filename + "'";
        s_lock = false;
        return _cmdResult;
    }

    // read the command lines
    std::vector< std::string > commandlines;
    std::string line;
    while ( !file.eof() )
    {
        getline( file, line, '\n' );
        if ( ( line.length() > 0 ) && ( line != "\n" ) && ( line != "" ) && ( line[ 0 ] != '#' ) )
        {
            line.erase( line.length() - 1 ); // remove line break from string
            commandlines.push_back( line );
        }
    }
    file.close();

    if ( !commandlines.size() )
    {
        _cmdResult = "* no command lines for execution!";
        s_lock = false;
        return _cmdResult;
    }

    // get the console entity where we will post command lines to
    if ( !_p_console )
    {
        _p_console = static_cast< EnConsole* >( EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
        assert( _p_console && "CmdExec::execute: console entity could not be found!" );
    }
    // execute the batch file lines
    std::vector< std::string >::iterator p_beg = commandlines.begin(), p_end = commandlines.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        _cmdResult += "executing command line: " + *p_beg + "\n";
        _p_console->enqueueCmd( *p_beg );
    }

    // release lock
    s_lock = false;

    return _cmdResult;
}

} // namespace CTD
