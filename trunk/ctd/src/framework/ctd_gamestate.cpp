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

#include <ctd_base.h>
#include "ctd_gamestate.h"

namespace CTD
{

CTD_SINGLETON_IMPL( GameState );

//! Implementation of GameState
GameState::GameState() :
_curState( GameState::Initializing ),
_gameMode( GameState::None )
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
            ( state == GameState::Leaving      ) ||
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
            ( mode == GameState::None       ) || 
            ( mode == GameState::Standalone ) || 
            ( mode == GameState::Client     ) || 
            ( mode == GameState::Server     ) && 
            "GameState: invalid game mode!" 
          );

    _gameMode = mode;
}

} // namespace CTD
