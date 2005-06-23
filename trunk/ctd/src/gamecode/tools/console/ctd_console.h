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
 # entity for an in-game console
 #
 #   date of creation:  06/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CONSOLE_H_
#define _CTD_CONSOLE_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_CONSOLE    "Console"

class ConsoleIH;

//! Console entity
class EnConsole :  public BaseEntity
{
    public:
                                                    EnConsole();

        virtual                                     ~EnConsole();

        //! Initialize entity
        void                                        initialize();

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Enqueue command for execution in next update ( is used when commands need other commands for execution, e.g. 'exec' )
        void                                        enqueueCmd( std::string cmd );

        //! Try to find a cmd match given the string
        void                                        autoCompleteCmd( const std::string& cmd );

        //! Triggers the application shutdown after 'delay' elapsed.
        void                                        triggerShutdown( float delay );

        //! Trigger idle state for 'steps' updates, while in idle state no queued commands are executed
        void                                        triggerIdle( unsigned int steps );

        //! Creates a log file and directs all console outputs also to that file. Returns fals if the log file could not be created.
        //! If append is false then an existing lod file will be recreated.
        bool                                        createLog( const std::string& filename, bool append = false );

        //! Closes the currently active log. Returns false if there is no active log previously created with createLog.
        bool                                        closeLog();

        //! Set current working directory. Returns false if the path could not be set.
        bool                                        setCWD( const std::string& cwd );

        //! Get current working directory
        const std::string&                          getCWD();

        //! Override this method and return true to get a persisten entity
        const bool                                  isPersistent() const { return true; }

        //! This entity needs no transformation
        const bool                                  isTransformable() const { return false; }

    protected:

        //! Enable / disable statistics rendering
        void                                        enable( bool en );

        //! Enqueue given command received from console
        void                                        issueCmd( std::string cmd );

        //! Apply command, it hashcmd is true then the command will be stored in command history
        void                                        applyCmd( const std::string& cmd, bool hashcmd = true );

        //! Dispatch command, return its result as string
        const std::string&                          dispatchCmdLine( const std::string& cmdline );

        //! Execute one single command including its arguments and return its result as string
        const std::string&                          executeCmd( const std::string& cmd );

        //! Extract the arguments from given cmdline and store them into 'args'.
        void                                        parseArguments( const std::string& cmdline, std::vector< std::string >& args );

        //! Restores an already used command from history. If prev is true then the history is stepped forward, otherwise it is stepped backward.
        void                                        cmdHistory( bool prev );

        //! Override notification callback
        void                                        handleNotification( const EntityNotification& notification );

        //! Callback for chat frame window close button
        bool                                        onCloseFrame( const CEGUI::EventArgs& arg );

        // Console window
        CEGUI::Window*                              _p_wnd;

        CEGUI::Editbox*                             _p_inputWindow;

        CEGUI::MultiLineEditbox*                    _p_outputWindow;

        bool                                        _enable;

        ConsoleIH*                                  _p_inputHandler;

        std::vector< std::string >                  _cmdHistory;

        unsigned int                                _cmdHistoryIndex;

        std::queue< std::pair< std::string, bool > > _cmdQueue;

        std::queue< std::pair< std::string, bool > > _issuedCmdQueue;

        bool                                        _shutdownInProgress;

        float                                       _shutdownCounter;

        bool                                        _idleInProgress;

        unsigned int                                _idleCounter;

        std::ofstream*                              _p_log;

        std::string                                 _cwd;

    friend class ConsoleIH;
};

//! Entity type definition used for type registry
class ConsoleEntityFactory : public BaseEntityFactory
{
    public:
                                                    ConsoleEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_CONSOLE, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~ConsoleEntityFactory() {}

        Macro_CreateEntity( EnConsole );
};

} // namespace CTD

#endif // _CTD_CONSOLE_H_
