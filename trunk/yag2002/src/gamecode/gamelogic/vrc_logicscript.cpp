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
#include "../objects/vrc_baseobject.h"

//! File name of logic log output
#define SCRIPTING_LOG_FILE_NAME             "logic.log"

//! Exposed logic interface name to scripting
#define SCRIPT_INTERFACE_NAME               "logic"

//! Script function initialize called on script setup
#define FCN_INITIALIZE                      "initialize"

//! Script function names
#define FCN_REQUEST_ACTION                  "requestAction"

//! Exposed method names
#define EXPOSED_METHOD_LOG                  "log"
#define EXPOSED_METHOD_GET_OBJECT_PROPS     "getObjectProperties"
#define EXPOSED_METHOD_GET_OBJECT_STATE     "getObjectState"

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
        scInitialize();
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

        // expose method log having the pseudo-signatur: void log( char* string )
        {
            std::string strbuf;
            arguments.add( strbuf );
            exposeMethod( EXPOSED_METHOD_LOG, &GameLogicScript::llog, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();
        // expose method for getting object properties having the pseudo-signatur: [ return value (int), prop string ] getObjectProperties( unsigned int objectID )
        {
            unsigned int objectID = 0;
            arguments.add( objectID );
            returnsvalues.add( int( 0 ) );
            returnsvalues.add( std::string( "" ) );
            exposeMethod( EXPOSED_METHOD_GET_OBJECT_PROPS, &GameLogicScript::lgetObjectProperties, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();
        // expose method for getting the object state having the pseudo-signatur: [ state (1 active, 0 not active), object ID ( can be used for validation ) ] getObjectState( unsigned int objectRefID )
        {
            unsigned int objectRefID = 0;
            arguments.add( objectRefID );
            returnsvalues.add( int( 0 ) );
            returnsvalues.add( int( 0 ) );
            exposeMethod( EXPOSED_METHOD_GET_OBJECT_STATE, &GameLogicScript::lgetObjectState, arguments, returnsvalues );
        }

        // execute the script after exposing methods; after this, all script functions are ready to be called now
        execute();

        // call the initialize function
        arguments.clear();
        callScriptFunction( FCN_INITIALIZE, &arguments );
    }
    catch( const ScriptingException& e )
    {
        log_error << "error occured during script setup: " << e.what() << std::endl;
        *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "error occured during script setup: " << e.what() << std::endl;

        // output also to script editor
        scAddOutput( e.what() );

        return false;
    }

    return true;
}

void GameLogicScript::scProcessCmd( const std::string& cmd )
{
    std::vector< std::string > args;
    yaf3d::explode( cmd, " ", &args );

    if ( args[ 0 ] == "help" )
    {
        std::string cmds;
        cmds += "help\n";
        cmds += "load [ -r | file name ]";

        scAddOutput( "list of valid commands:\n" + cmds );
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
            scAddOutput( "usage: load [ -r | file name ]\nuse -r for reload current script." );
        }
    }
    else
    {
        scAddOutput( "invalid command, use the command 'help' for getting valid commands." );
    }
}

bool GameLogicScript::requestAction( unsigned int actiontype, unsigned int objectID, unsigned int objectInstanceID, const std::vector< float >& params, std::vector< float >& returnvalues )
{
    // check if the script has been loaded without errors
    if ( !_valid )
    {
        scAddOutput( "*** " FCN_REQUEST_ACTION " cannot be executed. script not loaded!" );
        return false;
    }

    // fill in the parameters
    Params arguments;

    // push the fixed function parameters
    arguments.add( int( actiontype ) );
    arguments.add( int( objectID ) );
    arguments.add( int( objectInstanceID ) );

    // fill the generic parameters
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
        scAddOutput( std::string( "error on executing function " FCN_REQUEST_ACTION "\n" ) + e.what() );
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

void GameLogicScript::lgetObjectProperties( const Params& arguments, Params& returnvalues )
{
    unsigned int objectID = GET_SCRIPT_PARAMVALUE( arguments, 0, unsigned int );
    if ( arguments.size() != 1 )
    {
        scAddOutput( "*** error calling exposed method '" EXPOSED_METHOD_GET_OBJECT_PROPS "' for getting object properties!" );
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, int, -1 );
        SET_SCRIPT_PARAMVALUE( returnvalues, 1, std::string, "" );
        return;
    }

    //! TODO: ask the database! for now fill in dummy return values
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, int, 2 );
    SET_SCRIPT_PARAMVALUE( returnvalues, 1, std::string, "Hello Foo" );
}

void GameLogicScript::lgetObjectState( const Params& arguments, Params& returnvalues )
{
    unsigned int objectRefID = GET_SCRIPT_PARAMVALUE( arguments, 0, unsigned int );
    if ( arguments.size() != 1 )
    {
        scAddOutput( "*** error calling exposed method '" EXPOSED_METHOD_GET_OBJECT_STATE "' for getting object state!" );
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, int, -1 );
        SET_SCRIPT_PARAMVALUE( returnvalues, 1, int, -1 );
        return;
    }

    // get the instance ID
    unsigned int instanceID = GET_SCRIPT_PARAMVALUE( arguments, 0, unsigned int );
    // try to get the object instance
    BaseObject* p_object = BaseObject::getObject( instanceID );
    if ( !p_object )
    {
        scAddOutput( "*** " EXPOSED_METHOD_GET_OBJECT_STATE ": requested object instance does not exist!" );
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, int, -1 );
        SET_SCRIPT_PARAMVALUE( returnvalues, 1, int, -1 );
        return;
    }

    // fill in the function return values
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, unsigned int, p_object->isActive() );
    SET_SCRIPT_PARAMVALUE( returnvalues, 1, unsigned int, p_object->getObjectID() );
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
        scAddOutput( "# " + str );
    }
}

} // namespace vrc
