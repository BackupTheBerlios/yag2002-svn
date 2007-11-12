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


#include <vrc_main.h>
#include "vrc_gamelogic.h"
#include "vrc_logicscript.h"


//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::GameLogic )

namespace vrc
{

GameLogic::GameLogic() :
 _p_script( NULL )
{
}

GameLogic::~GameLogic()
{
    if ( _p_script )
        delete _p_script;
}

void GameLogic::initialize( const std::string& scriptfile ) throw ( GameLogicException )
{
    // if already intitialized then re-initialize the script
    if ( _p_script )
        delete _p_script;

    _p_script = new GameLogicScript;
    if ( !_p_script->setupScript( scriptfile ) )
        throw( GameLogicException( "cannot load script file " + scriptfile ) );
}

void GameLogic::shutdown()
{
    // destroy the singleton
    destroy();
}

ScriptConsole* GameLogic::getScriptConsole()
{
    return _p_script;
}

bool GameLogic::requestAction( unsigned int actionType, unsigned int objectID, const std::vector< float >& params, std::vector< float >& returnvalues )
{
    assert( _p_script && "scripting interface does not exists!" );

    //! TODO: implement the networking part here, for now we test the logic in standalone mode!
    return _p_script->requestAction( actionType, objectID, params, returnvalues );
}

} // namespace vrc
