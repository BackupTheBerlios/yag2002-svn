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
 # this is the interface to the game thread
 #
 #   date of creation:  01/22/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _GAMEINTERFACE_H_
#define _GAMEINTERFACE_H_

#include <vrc_main.h>

class EditorApp;
class AppInterface;
class GameNavigator;

//! Interface to game thread for updating / rendering the game loop, and receiving commands from application interface
class GameInterface: public OpenThreads::Thread
{
    public:

        //! Commands to game thread
        enum
        {
            CMD_UNKNOWN         = 0x00,
            CMD_LOAD_LEVEL      = 0x01,
            CMD_UNLOAD_LEVEL    = 0x02,
            CMD_INIT_ENTITY     = 0x04
        };

        //! Initialize the interface, pass the first application's main function argument ( argv[ 0 ] )
        explicit                                        GameInterface( const std::string& apppath );

        virtual                                         ~GameInterface();

        //! Initialize the interface which starts the game thread and command dispatcher
        void                                            initialize( AppInterface* p_appiface );

        //! Terminate the interface
        void                                            terminate();

        //! Send command to game thread
        void                                            sendCmd( unsigned int cmd, void* p_data );

    protected:

        //! OpenThreads' overridden method for starting the thread
        virtual void                                    run();

        //! Dispatch command, returns the count of remaining commands in queue.
        unsigned int                                    dispatchCmd();

        yaf3d::Application*                             _p_game;

        AppInterface*                                   _p_appIface;

        GameNavigator*                                  _p_navigation;

        bool                                            _terminate;

        std::string                                     _appPath;

        OpenThreads::Mutex                              _cmdMutex;

        std::queue< std::pair< unsigned int, void* > >  _cmds;
};


//! Helper class for a scoped game update mutex
class ScopedGameUpdateLock
{
    public:

        ScopedGameUpdateLock()
        {
            yaf3d::Application::get()->getUpdateMutex().lock();
        }

        ~ScopedGameUpdateLock()
        {
            yaf3d::Application::get()->getUpdateMutex().unlock();
        }
};

#endif // _GAMEINTERFACE_H_
