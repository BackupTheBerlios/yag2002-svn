/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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

#include <ctd_base.h>
#include "ctd_gamestate.h"
#include "ctd_log.h"

namespace CTD
{

CTD_SINGLETON_IMPL( GameState );

//! Implementation of GameState
GameState::GameState() :
_curState( GameState::Initializing ),
_gameMode( GameState::Client )
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
            ( state == GameState::Initializing ) || 
            ( state == GameState::Menu         ) ||
            ( state == GameState::Running      ) ||
            ( state == GameState::Pausing      ) ||
            ( state == GameState::Quitting     ) &&
            "GameState: trying to set an invalid game state!" 
          );

    _curState = state;
}

unsigned int GameState::getState()
{
    return _curState;
}

unsigned int GameState::getMode()
{
    return _gameMode;
}

void GameState::setMode( unsigned int mode )
{
    assert( 
            ( mode == GameState::None   ) || 
            ( mode == GameState::Client ) || 
            ( mode == GameState::Server ) && 
            "GameState: invalid game mode!" 
          );

    _gameMode = mode;
}

}
