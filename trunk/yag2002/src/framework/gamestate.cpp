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
 # game state: holds game mode (server or client ) and all
 #   possible game states
 #
 #   date of creation:  04/09/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "gamestate.h"

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( GameState );

//! Implementation of GameState
GameState::GameState() :
_curState( GameState::UnknownState ),
_gameMode( GameState::UnknownMode )
{
}

GameState::~GameState()
{
}

void GameState::shutdown()
{
    destroy();
}

//! Set new state
void GameState::setState( unsigned int state )
{
    assert( 
            ( state == GameState::UnknownState ) || 
            ( state == GameState::Initializing ) || 
            ( state == GameState::Menu         ) ||
            ( state == GameState::Running      ) ||
            ( state == GameState::Pausing      ) ||
            ( state == GameState::Leaving      ) ||
            ( state == GameState::Quitting     ) &&
            "GameState: trying to set an invalid game state!" 
          );

    if ( _curState == state )
        return;

    _curState = state;

    // call all registered callbacks
    std::vector< CallbackStateChange* >::iterator p_beg = _cbsStateChange.begin(), p_end = _cbsStateChange.end();
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

    // call all registered callbacks
    std::vector< CallbackModeChange* >::iterator p_beg = _cbsModeChange.begin(), p_end = _cbsModeChange.end();
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

} // namespace yaf3d
