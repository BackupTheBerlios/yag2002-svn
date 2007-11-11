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


#include <vrc_main.h>
#include "vrc_logicscript.h"


//! File name of logic log output
#define SCRIPTING_LOG_FILE_NAME     "logic.log"

//! Exposed logic interface name to scripting
#define SCRIPT_INTERFACE_NAME       "logic"

//! Script function names
#define FCN_REQUEST_ACTION          "requestAction"


namespace vrc
{

GameLogicScript::GameLogicScript() :
 _p_log( NULL )
{
    _p_log = new yaf3d::Log;
    _p_log->addSink( "file", yaf3d::Application::get()->getMediaPath() + std::string( SCRIPTING_LOG_FILE_NAME ), yaf3d::Log::L_VERBOSE );

    //! TODO: setup editor only when in dev mode
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server )
    {
        seInitialize();
    }
}

GameLogicScript::~GameLogicScript()
{
    if ( _p_log )
        delete _p_log;
}

bool GameLogicScript::setupScript( const std::string& file )
{
    _scriptFile = file;

    // load and setup the scripting binding
    try
    {
        // load script file and scope all exposed methods with 'logic'
        loadScript( SCRIPT_INTERFACE_NAME, file, BaseScript< GameLogicScript >::MATH | BaseScript< GameLogicScript >::STRING );

        // expose methods
        Params arguments;
        Params returnsvalues;

        // expose method log having the pseudo-signatur: void getPosition( char* string )
        {
            std::string strbuf;
            arguments.add< std::string >( strbuf );
            exposeMethod( "log", &GameLogicScript::llog, arguments, returnsvalues );
        }

        // execute the script after exposing methods; after this, all script functions are ready to be called now
        execute();
    }
    catch( const ScriptingException& e )
    {
        log_error << "error occured during script setup: " << e.what() << std::endl;
        *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "error occured during script setup: " << e.what() << std::endl;

        // output also to script editor
        seAddOutput( e.what() );

        return false;
    }

    return true;
}

void GameLogicScript::seProcessCmd( const std::string& cmd )
{
    std::vector< std::string > args;
    yaf3d::explode( cmd, " ", &args );

    if ( args[ 0 ] == "help" )
    {
        std::string cmds;
        cmds += "help\n";
        cmds += "load [ -r | file name ]";

        seAddOutput( "list of valid commands:\n" + cmds );
    }
    else if ( args[ 0 ] == "load" )
    {
        if ( args.size() > 1 )
        {
            // first close the script
            closeScript();

            if ( ( args[ 1 ] == "-r" ) && _scriptFile.length() )
            {
                if ( !setupScript( _scriptFile ) )
                    closeScript();
            }
            else
            {
                if ( !setupScript( args[ 1 ] ) )
                    closeScript();
            }
        }
        else
        {
            seAddOutput( "usage: load [ -r | file name ]\nuse -r for reload current script." );
        }
    }
    else
    {
        seAddOutput( "invalid command, use the command 'help' for getting valid commands." );
    }
}

bool GameLogicScript::requestAction( unsigned int actiontype, unsigned int objectID, const std::vector< float >& params, std::vector< float >& returnvalues )
{
    // check if the script has been loaded without errors
    if ( !_valid )
    {
        seAddOutput( "*** " FCN_REQUEST_ACTION " cannot be executed. script not loaded!" );
        return false;
    }

    // fill in the parameters
    Params arguments;

    // first argument is the action type
    arguments.add( int( actiontype ) );
    arguments.add( int( objectID ) );

    // fill the parameters
    std::vector< float >::const_iterator p_param = params.begin(), p_end = params.end();
    for ( ; p_param != p_end; ++p_param )
    {
        arguments.add< float >( *p_param );
    }

    Params ret;
    // fill the return values
    std::vector< float >::const_iterator p_ret = returnvalues.begin(), p_endret = returnvalues.end();
    for ( ; p_ret != p_endret; ++p_ret )
    {
        ret.add< float >( *p_ret );
    }

    try
    {
        // call script function requestAction, the actual argument and return value counts depend on action type
        callScriptFunction( FCN_REQUEST_ACTION, &arguments, &ret );
    }
    catch ( const ScriptingException& e )
    {
        *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "GameLogicScript: error on calling function " << FCN_REQUEST_ACTION << std::endl;
        *_p_log << "reason : " << e.what() << std::endl;

        // output also to script editor
        seAddOutput( std::string( "error on executing function " FCN_REQUEST_ACTION "\n" ) + e.what() );
        return false;
    }

    // fill in the output values
    std::size_t retcnt = ret.size();
    for ( unsigned int cnt = 0; cnt < retcnt; ++cnt )
    {
        returnvalues[ cnt ] = ret.getValue< float >( cnt );
    }

    return true;
}

void GameLogicScript::llog( const Params& arguments, Params& /*returnvalues*/ )
{
    std::string str = GET_SCRIPT_PARAMVALUE( arguments, 0, std::string );
    if ( arguments.size() )
    {
        //! TODO: get the lua function name
        //lua_Debug dbg;
        //dbg.name = NULL;
        //lua_getinfo( _p_state, ">S", &dbg );
        //std::string fcnname = dbg.name ? dbg.name : "?fcn";

        _p_log->enableSeverityLevelPrinting( false );
        *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << /*fcnname << ": " <<*/ str << std::endl;
        _p_log->enableSeverityLevelPrinting( false );

        // add the log string also to script editor's output
        seAddOutput( "# " + str );
    }
}

} // namespace vrc
