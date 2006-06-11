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
 # console command 'rendeing.mode'
 #
 #   date of creation:  02/15/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdrenderingmode.h"
#include "vrc_cmdregistry.h"
#include <osg/PolygonMode>

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdRenderingMode )


CmdRenderingMode::CmdRenderingMode() :
 BaseConsoleCommand( CMD_NAME_RENDERINGMODE )
{
    setUsage( CMD_USAGE_RENDERINGMODE );
}

CmdRenderingMode::~CmdRenderingMode()
{
}

const std::string& CmdRenderingMode::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    if ( arguments.size() != 1 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
    
    std::string arg = arguments[ 0 ];
    osg::PolygonMode::Mode mode = osg::PolygonMode::POINT;
    if ( ( arg == "f" ) || ( arg == "fill" ) )
    {
        mode = osg::PolygonMode::FILL;
    }
    else if ( ( arg == "l" ) || ( arg == "line" ) )
    {
        mode = osg::PolygonMode::LINE;
    }
    else if ( ( arg == "p" ) || ( arg == "point" ) )
    {
        mode = osg::PolygonMode::POINT;
    }
    else
    {
        _cmdResult = "invalid mode: " + arg + "\n";
        _cmdResult += getUsage();
        return _cmdResult;
    }

    osg::StateSet* p_stateset = yaf3d::Application::get()->getSceneView()->getGlobalStateSet();
    osg::PolygonMode* p_polymode = dynamic_cast<osg::PolygonMode*>( p_stateset->getAttribute( osg::StateAttribute::POLYGONMODE ) );
    if ( !p_polymode ) 
    {
        p_polymode = new osg::PolygonMode;
        p_stateset->setAttribute( p_polymode );
    }
    p_polymode->setMode( osg::PolygonMode::FRONT_AND_BACK, mode );

    return _cmdResult;
}

} // namespace vrc
