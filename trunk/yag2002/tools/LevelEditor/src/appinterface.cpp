/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # this is the interface to the application thread (editor)
 #
 #   date of creation:  01/22/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "appinterface.h"
#include "gameinterface.h"
#include "editor.h"
#include "mainframe.h"
#include "navigation.h"


//! Update period for dispatching incomming messages ( in milliseconds )
#define UPDATE_PERIOD       100


//! Game loop thread
AppInterface::AppInterface( MainFrame* p_frame ) :
 _p_mainFrame( p_frame ),
 _p_gameIface( NULL )
{
}

AppInterface::~AppInterface()
{
}

void AppInterface::initialize( GameInterface* p_gameiface )
{
    assert( p_gameiface && "invalid game interface!" );

    _p_gameIface = p_gameiface;

    // set the picking callback so we get notified when something picked
    GameNavigator::get()->setPickingCallback( this );

    Start( UPDATE_PERIOD );
}

void AppInterface::terminate()
{
    Stop();
}

void AppInterface::sendCmd( unsigned int cmd, void* p_data )
{
    _cmdMutex.lock();

    _cmds.push( std::make_pair( cmd, p_data ) );

    _cmdMutex.unlock();
}

void AppInterface::Notify()
{
    dispatchCmd();

    unsigned int fps = GameNavigator::get()->getFPS();
    const osg::Vec3f& pos = GameNavigator::get()->getCameraPosition();
    osg::Vec2f rot;
    GameNavigator::get()->getCameraPitchYaw( rot._v[ 0 ], rot._v[ 1 ] );
    _p_mainFrame->updateStats( fps, pos, rot );
}

void AppInterface::onEntityPicked( yaf3d::BaseEntity* p_entity )
{
    // dispatch the result of picking in app loop context
    sendCmd( CMD_PICKING, p_entity );
}

unsigned int AppInterface::dispatchCmd()
{
    if ( _cmdMutex.trylock() )
        return 0;

    if ( !_cmds.size() )
    {
        _cmdMutex.unlock();
        return 0;
    }

    std::pair< unsigned int, void* > cmd = _cmds.front();
    _cmds.pop();

    switch ( cmd.first )
    {
        case CMD_GAME_STARTED:
        {
            _p_mainFrame->notify( MainFrame::NOTIFY_GAME_STARTED );
        }
        break;

        case CMD_LEVEL_LOADED:
        {
            _p_mainFrame->notify( MainFrame::NOTIFY_LEVEL_LOADED );
        }
        break;

        case CMD_LEVEL_UNLOADED:
        {
            _p_mainFrame->notify( MainFrame::NOTIFY_LEVEL_UNLOADED );
        }
        break;

        case CMD_PICKING:
        {
            yaf3d::BaseEntity* p_entity = reinterpret_cast< yaf3d::BaseEntity* >( cmd.second );
            _p_mainFrame->selectEntity( p_entity );
        }
        break;

        case CMD_ERROR:
        {
            wxMessageBox( "Problem running the game. Look into log file for more details.", "Error" );
        }
        break;

        default:
            ;
    }

    unsigned int remainingcmds = _cmds.size();

    _cmdMutex.unlock();

    return remainingcmds;
}
