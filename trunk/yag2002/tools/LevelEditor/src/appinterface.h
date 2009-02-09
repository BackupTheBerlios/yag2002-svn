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

#ifndef _APPINTERFACE_H_
#define _APPINTERFACE_H_

#include <vrc_main.h>
#include "guibase.h"
#include "navigation.h"

class MainFrame;
class GameInterface;

//! Interface to application thread for receiving commands from game thread
class AppInterface: public wxTimer, public CallbackPicking
{
    public:

        //! Commands to game thread
        enum
        {
            CMD_UNKNOWN         = 0x0000,
            CMD_ERROR           = 0x1001,
            CMD_GAME_STARTED    = 0x1002,
            CMD_LEVEL_LOADED    = 0x1004,
            CMD_LEVEL_UNLOADED  = 0x1008,
            CMD_PICKING         = 0x1010
        };

        explicit                                        AppInterface( MainFrame* p_frame );

        virtual                                         ~AppInterface();

        //! Initialize the interface which starts the command dispatcher
        void                                            initialize( GameInterface* p_gameiface );

        //! Terminate the interface
        void                                            terminate();

        //! Send command to game thread
        void                                            sendCmd( unsigned int cmd, void* p_data );

    protected:

        //! Overridden method of timer which is called periodically in wx main context.
        virtual void                                    Notify();

        //! Called when the picker delivers results.
        virtual void                                    onEntityPicked( yaf3d::BaseEntity* p_entity );

        //! Dispatch command, returns the count of remaining commands in queue.
        unsigned int                                    dispatchCmd();

        MainFrame*                                      _p_mainFrame;

        GameInterface*                                  _p_gameIface;

        OpenThreads::Mutex                              _cmdMutex;

        std::queue< std::pair< unsigned int, void* > >  _cmds;
};


#endif // _APPINTERFACE_H_
