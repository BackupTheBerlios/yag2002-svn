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
 # main game logic scripting interface
 #
 #   date of creation:  03/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _VRC_LOGICSCRIPT_H_
#define _VRC_LOGICSCRIPT_H_

#include <vrc_main.h>
#include "../scripting/vrc_script.h"
#include "../scripting/vrc_scripteditor.h"

namespace vrc
{

//! Game logic scripting interface
class GameLogicScript : public BaseScript< GameLogicScript >, public ScriptEditor
{
    public:

                                                    GameLogicScript();

        virtual                                     ~GameLogicScript();

        //! Load the script file
        bool                                        setupScript( const std::string& file );


        //! Call the script function 'requestAction'. The actual argument and return value counts depend on action type and objectID.
        bool                                        requestAction( unsigned int actiontype, unsigned int objectID, const std::vector< float >& params, std::vector< float >& returnvalues );

    protected:

        // Exposed methods to scripting
        // ############################

        //! Method for outputting to log system
        void                                        llog( const Params& arguments, Params& /*returnvalues*/ );

        // ############################

        //! Script editor's method called when a new command has been issued
        virtual void                                seProcessCmd( const std::string& cmd );

        //! Script file name
        std::string                                 _scriptFile;

        //! Log object for scripting output
        yaf3d::Log*                                 _p_log;
};

}

#endif // _VRC_LOGICSCRIPT_H_
