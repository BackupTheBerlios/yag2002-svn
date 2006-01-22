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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include <base.h>
#include <singleton.h>

namespace yaf3d
{

class Application;

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
            UnknownMode  = 0x10,
            Standalone,
            Server,
            Client
        };

        //! Game state
        enum State
        {
            UnknownState = 0x20,
            Initializing,           // set at startup
            Menu,                   // user is in menu
            Running,                // running game
            Pausing,                // game pausing
            Leaving,                // leaving a level
            Quitting,               // quitting application
            Shutdown                // shutdown the application
        };

        //! Application window state
        enum ApplicationWindowState
        {
            UnknownAppWindowState = 0x30,
            Minimized,              // application window has been iconified
            Restored,               // application window has been restored
            GainedFocus,            // application window got input focus
            LostFocus               // application window lost input focus
        };

        //! Class for getting callback whenever mode change occured
        class CallbackModeChange
        {
            public:
                                                    CallbackModeChange() {}

                virtual                             ~CallbackModeChange() {}

                virtual void                        onModeChange( unsigned int mode ) = 0;
        };

        //! Class for getting callback whenever state change occured
        class CallbackStateChange
        {
            public:
                                                    CallbackStateChange() {}

                virtual                             ~CallbackStateChange() {}

                virtual void                        onStateChange( unsigned int state ) = 0;
        };

        //! Class for getting callback whenever an application window state change occured
        class CallbackAppWindowStateChange
        {
            public:
                                                    CallbackAppWindowStateChange() {}

                virtual                             ~CallbackAppWindowStateChange() {}

                virtual void                        onAppWindowStateChange( unsigned int state ) = 0;
        };

        //! Set game mode to Server, Client, or Standalone
        void                                    setMode( unsigned int mode );

        //! Get game mode
        unsigned int                            getMode();

        //! Register / deregister a callback for mode change ( pass reg = true for registering, otherwiese deregistering )
        //! Note, this action is not queued, so don't call this method inside a mode change callback.
        void                                    registerCallbackModeChange( CallbackModeChange* p_cb, bool reg = true );

        //! Set new state
        void                                    setState( unsigned int state );

        //! Get state state
        unsigned int                            getState();
 
        //! Register / deregister a callback for state change ( pass reg = true for registering, otherwiese deregistering )
        //! Note, this action is not queued, so don't call this method inside a state change callback.
        void                                    registerCallbackStateChange( CallbackStateChange* p_cb, bool reg = true );

        //! Get application window state ( Minimized, Restored )
        unsigned int                            getAppWindowState();
 
        //! Register / deregister a callback for app window state change ( pass reg = true for registering, otherwiese deregistering )
        void                                    registerCallbackAppWindowStateChange( CallbackAppWindowStateChange* p_cb, bool reg = true );

    protected:

                                                GameState();

        virtual                                 ~GameState();

        //! Shutdown
        void                                    shutdown();

        //! Input handler for catching application window state changes
        class InputHandler : public osgGA::GUIEventHandler
        {
            public:

                explicit                            InputHandler( GameState* p_gamestate );

                virtual                             ~InputHandler();


                //! Handle input events
                bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

            protected:

                GameState*                          _p_gameState;
        };

        //! Set application window state, this state can be set only by GameState itself.
        void                                    setAppWindowState( unsigned int state );

        //! Create the input handler for catching app window state changes.
        //! Note: This method must be called before making use of window state change callbacks.
        //!       In normal case, Application will call this during system initialization.
        void                                    initAppWindowStateHandler();

        //! Current game state
        unsigned int                            _curState;

        //! Game mode (server or client)
        unsigned int                            _gameMode;

        //! Application window state
        unsigned int                            _appWindowState;

        //! Registered callbacks for mode change
        std::vector< CallbackModeChange* >      _cbsModeChange;

        //! Registered callbacks for state change
        std::vector< CallbackStateChange* >     _cbsStateChange;

        //! Registered callbacks for app window state change
        std::vector< CallbackAppWindowStateChange* > _cbsAppWindowStateChange;

        //! Input handler for catching and distributing applicaiton window state changes
        osg::ref_ptr< GameState::InputHandler >  _inputHandler;

    friend class GameState::InputHandler;
    friend class Singleton< GameState >;
    friend class Application;
};

} // namespace yaf3d

#endif //_GAMESTATE_H_
