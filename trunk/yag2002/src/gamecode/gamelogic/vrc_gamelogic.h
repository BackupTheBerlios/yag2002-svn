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
 # main game logic implementation
 #
 #   date of creation:  03/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _VRC_GAMELOGIC_H_
#define _VRC_GAMELOGIC_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>


namespace vrc
{

//! Class for game logic related exceptions
class GameLogicException : public std::runtime_error
{
    public:
                                                    GameLogicException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~GameLogicException() throw() {}

                                                    GameLogicException( const GameLogicException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    GameLogicException();

        GameLogicException&                         operator = ( const GameLogicException& );
};


class GameLogicScript;
class ScriptConsole;

//! Game logic class
class GameLogic : public yaf3d::Singleton< GameLogic >
{
    public:

        //! Action types
        enum ActionType
        {
            eActionPick = 0x1000,
            eActionDrop = 0x1001,
            eActionUse  = 0x1002
        };

        //! Request the game logic for processing an action on object with given object and instance ID.
        bool                                        requestAction( unsigned int actionType, unsigned int objectID, unsigned int objectInstanceID, const std::vector< float >& params, std::vector< float >& returnvalues );

        //! Get the script console, used for script debugging
        ScriptConsole*                              getScriptConsole();

    protected:

                                                    GameLogic();

        virtual                                     ~GameLogic();

        //! Initialize the storage server
        void                                        initialize( const std::string& scriptfile ) throw ( GameLogicException );

        //! Shutdown the storage server
        void                                        shutdown();

        //! Game script interface
        GameLogicScript*                            _p_script;

    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< GameLogic >;
};

}

#endif // _VRC_GAMELOGIC_H_
