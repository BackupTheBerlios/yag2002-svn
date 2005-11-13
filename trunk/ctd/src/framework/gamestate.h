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

#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include <base.h>
#include <singleton.h>
#include <application.h>

namespace yaf3d
{
//! Game state
/**
* Singleton for holding various game states in a central place.
* Both, the framework and entities can set states and retrieve current state.
*/
class GameState : public Singleton< GameState >
{
    public:

        //! Game mode
        enum GameMode
        {
            None        = 0x10,
            Standalone,
            Server,
            Client
        };

        //! Game state
        enum State
        {
            Initializing = 0x20,    // set at startup
            Menu,                   // user is in menu
            Running,                // running game
            Pausing,                // game pausing
            Leaving,                // leaving a level
            Quitting                // quitting application
        };

        //! Set game mode to Server, Client, or Standalone
        void                                    setMode( unsigned int mode );

        //! Get game mode
        unsigned int                            getMode();

        //! Set new state
        void                                    setState( unsigned int state );

        //! Get state state
        unsigned int                            getState();
 
    protected:

                                                GameState();

        virtual                                 ~GameState();

        //! Shutdown
        void                                    shutdown();

        //! Current game state
        unsigned int                            _curState;

        //! Game mode (server or client)
        unsigned int                            _gameMode;

    friend class Singleton< GameState >;
    friend class Application;
};

} // namespace yaf3d

#endif //_GAMESTATE_H_
