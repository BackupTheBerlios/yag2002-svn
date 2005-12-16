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
 # console command 'writeosgfile', use this to write out the current
 #  scene into a given osg file
 #
 #   date of creation:  08/11/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdwriteosgfile.h"
#include "vrc_cmdregistry.h"
#include "vrc_console.h"
#include <osgDB/WriteFile>

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdWriteOSGFile );


CmdWriteOSGFile::CmdWriteOSGFile() :
 BaseConsoleCommand( CMD_NAME_WRITEOSGFILE )
{
    setUsage( CMD_USAGE_WRITEOSGFILE );
}

CmdWriteOSGFile::~CmdWriteOSGFile()
{
}

const std::string& CmdWriteOSGFile::execute( const std::vector< std::string >& arguments )
{
    bool hasinputfile = false;
    if ( arguments.size() < 1 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
    else if ( ( arguments.size() > 2 ) && ( arguments[ 0 ] == "-i" ) )
    {
        hasinputfile = true;
    }
    else
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    EnConsole* p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
    assert( _p_console && "CmdWriteOSGFile::execute: console entity could not be found!" );
    std::string cwd = yaf3d::Application::get()->getMediaPath() + p_console->getCWD() + "/";
    if ( !hasinputfile )
    {
        osg::Group* grp = yaf3d::Application::get()->getSceneRootNode();
        if ( grp )
        {
            _cmdResult = "writing scene to osg file '" + arguments[ 0 ] + "\n";
            std::string filename = cwd + arguments[ 0 ];

            if ( !osgDB::writeNodeFile( *grp, filename ) )
            {
                _cmdResult += " warning: problem writing file '" + filename + "\n";
            }
        }
    }
    else
    {
        _cmdResult = "reading scene file " + arguments[ 1 ] + " ...\n";
        std::string infilename = cwd + arguments[ 1 ];
        osg::ref_ptr< osg::Node > node = osgDB::readNodeFile( infilename );
        if ( node.get() )
        {
            std::string outfilename = cwd + arguments[ 2 ];
            _cmdResult += "writing scene to file " + outfilename + "\n";

            if ( !osgDB::writeNodeFile( *( node.get() ), outfilename ) )
            {
                _cmdResult += " warning: problem writing file " + outfilename + "\n";
            }
        }
        else
        {
            _cmdResult += "cannot read scene file " + infilename + "\n";
        }
    }

    return _cmdResult;
}

} // namespace vrc
