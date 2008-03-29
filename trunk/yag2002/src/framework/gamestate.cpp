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
 # game state: holds game mode (server or client ) and all
 #   possible game states
 #
 #   date of creation:  04/09/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "log.h"
#include "gamestate.h"
#include "application.h"


namespace yaf3d
{

YAF3D_SINGLETON_IMPL( GameState )

GameState::InputHandler::InputHandler( GameState* p_gameState ) :
_p_gameState( p_gameState )
{
    // register us in viewer to get event callbacks
    Application::get()->getViewer()->addEventHandler( this );
}

GameState::InputHandler::~InputHandler()
{
    // remove this handler from viewer's handler list
    Application::get()->getViewer()->removeEventHandler( this );
}

bool GameState::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
{
    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );
    const SDL_Event sdlevent = p_eventAdapter->getSDLEvent();

    // get current app window state
    unsigned int state = _p_gameState->_appWindowState;
    // set proper state depending on current state and incoming sdl event
    if ( sdlevent.active.type == SDL_ACTIVEEVENT )
    {
        if ( sdlevent.active.state == ( SDL_APPINPUTFOCUS | SDL_APPACTIVE ) )
        {
            if ( sdlevent.active.gain == 0 )
                state = GameState::Minimized;
            else
            {
                if ( state == GameState::LostFocus )
                    state = GameState::GainedFocus;
                else
                    state = GameState::Restored;
            }
        }
        else if ( sdlevent.active.state & SDL_APPINPUTFOCUS )
        {
            if ( sdlevent.active.gain == 0 )
                state = GameState::LostFocus;
            else
                state = GameState::GainedFocus;
        }

        _p_gameState->setAppWindowState( state );
    }

    return false;
}

//! Implementation of GameState
GameState::GameState() :
_curState( GameState::UnknownState ),
_gameMode( GameState::UnknownMode ),
_appWindowState( GameState::Restored )
{
}

GameState::~GameState()
{
}

void GameState::shutdown()
{
    log_info << "GameState: shutting down" << std::endl;

    destroy();
}

//! Set new state
void GameState::setState( unsigned int state )
{
    assert(
            ( state == GameState::UnknownState        ) ||
            ( state == GameState::Initializing        ) ||
            ( state == GameState::GraphicsInitialized ) ||
            ( state == GameState::StartRunning        ) ||
            ( state == GameState::MainLoop            ) ||
            ( state == GameState::Pausing             ) ||
            ( state == GameState::StartingLevel       ) ||
            ( state == GameState::LeavingLevel        ) ||
            ( state == GameState::Quitting            ) ||
            ( state == GameState::Shutdown            ) &&
            "GameState: trying to set an invalid game state!"
          );

    if ( _curState == state )
        return;

    _curState = state;

    // call all registered callbacks; allow modifying the registry list during callback
    std::vector< CallbackStateChange* > templist = _cbsStateChange;
    std::vector< CallbackStateChange* >::iterator p_beg = templist.begin(), p_end = templist.end();
    for ( ; p_beg != p_end; ++p_beg )
        ( *p_beg )->onStateChange( _curState );
}

unsigned int GameState::getState()
{
    return _curState;
}

void GameState::registerCallbackStateChange( CallbackStateChange* p_cb, bool reg )
{
    // check whether the callback already exists
    std::vector< CallbackStateChange* >::iterator p_beg = _cbsStateChange.begin(), p_end = _cbsStateChange.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == p_cb )
            break;

    if ( reg )
    {
        assert( ( p_beg == p_end ) && "state change callback is already registered!" );
        _cbsStateChange.push_back( p_cb );
    }
    else
    {
        assert( ( p_beg != p_end ) && "state change callback does not exist for deregistering!" );
        _cbsStateChange.erase( p_beg );
    }
}

void GameState::setMode( unsigned int mode )
{
    assert(
            ( mode == GameState::UnknownMode ) ||
            ( mode == GameState::Standalone  ) ||
            ( mode == GameState::Client      ) ||
            ( mode == GameState::Server      ) &&
            "GameState: invalid game mode!"
          );

    if ( _gameMode == mode )
        return;

    _gameMode = mode;

    // call all registered callbacks; allow modifying the registry list during callback
    std::vector< CallbackModeChange* > templist = _cbsModeChange;
    std::vector< CallbackModeChange* >::iterator p_beg = templist.begin(), p_end = templist.end();
    for ( ; p_beg != p_end; ++p_beg )
        ( *p_beg )->onModeChange( _gameMode );
}

unsigned int GameState::getMode()
{
    return _gameMode;
}

void GameState::registerCallbackModeChange( CallbackModeChange* p_cb, bool reg )
{
    // check whether the callback already exists
    std::vector< CallbackModeChange* >::iterator p_beg = _cbsModeChange.begin(), p_end = _cbsModeChange.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == p_cb )
            break;

    if ( reg )
    {
        assert( ( p_beg == p_end ) && "mode change callback is already registered!" );
        _cbsModeChange.push_back( p_cb );
    }
    else
    {
        assert( ( p_beg != p_end ) && "mode change callback does not exist for deregistering!" );
        _cbsModeChange.erase( p_beg );
    }
}

void GameState::setAppWindowState( unsigned int state )
{
    assert(
            ( state == GameState::UnknownAppWindowState ) ||
            ( state == GameState::Minimized             ) ||
            ( state == GameState::Restored              ) ||
            ( state == GameState::GainedFocus           ) ||
            ( state == GameState::LostFocus             ) &&
            "GameState: invalid application window state!"
          );

    if ( _appWindowState == state )
        return;

    _appWindowState = state;

    // call all registered callbacks; allow modifying the registry list during callback
    std::vector< CallbackAppWindowStateChange* > templist = _cbsAppWindowStateChange;
    std::vector< CallbackAppWindowStateChange* >::iterator p_beg = templist.begin(), p_end = templist.end();
    for ( ; p_beg != p_end; ++p_beg )
        ( *p_beg )->onAppWindowStateChange( _appWindowState );
}

void GameState::initAppWindowStateHandler()
{
    assert( ( _inputHandler.get() == NULL ) && "GameState: app window handler already exists!" );
    _inputHandler = new GameState::InputHandler( this );
}

unsigned int GameState::getAppWindowState()
{
    return _gameMode;
}

void GameState::registerCallbackAppWindowStateChange( CallbackAppWindowStateChange* p_cb, bool reg )
{
    // check whether the callback already exists
    std::vector< CallbackAppWindowStateChange* >::iterator p_beg = _cbsAppWindowStateChange.begin(), p_end = _cbsAppWindowStateChange.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == p_cb )
            break;

    if ( reg )
    {
        assert( ( p_beg == p_end ) && "app window state change callback is already registered!" );
        _cbsAppWindowStateChange.push_back( p_cb );
    }
    else
    {
        assert( ( p_beg != p_end ) && "app window state change callback does not exist for deregistering!" );
        _cbsAppWindowStateChange.erase( p_beg );
    }
}

} // namespace yaf3d
