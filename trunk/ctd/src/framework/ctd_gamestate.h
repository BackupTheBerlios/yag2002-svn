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

#ifndef _CTD_GAMESTATE_H_
#define _CTD_GAMESTATE_H_

#include <ctd_base.h>
#include <ctd_singleton.h>
#include <ctd_application.h>

namespace CTD
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
            None,
            Server,
            Client
        };

        //! Game state
        enum State
        {
            Initializing,   // set at startup
            Menu,           // user is in menu
            Running,        // running game
            Pausing,        // game pausing
            Quitting        // quitting application
        };

        //! Set new state
        void                                    setState( unsigned int state );

        //! Get state state
        unsigned int                            getState();
 
        //! Get game mode
        unsigned int                            getMode();

    protected:

                                                GameState();

        /**
        * Avoid the usage of copy constructor.
        */
                                                GameState( GameState& );

        /**
        * Avoid assigning this singleton
        */
        GameState&                              operator = ( const GameState& );

        virtual                                 ~GameState();

        //! Shutdown
        void                                    shutdown();

        //! Set game mode, used by class Application 
        void                                    setMode( unsigned int mode );

        //! Current game state
        unsigned int                            _curState;

        //! Game mode (server or client)
        unsigned int                            _gameMode;

    friend class Singleton< GameState >;
    friend class Application;
};


} // namespace CTD

#endif //_CTD_GAMESTATE_H_
