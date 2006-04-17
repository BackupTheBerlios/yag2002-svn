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
 # class for testing the script base class BaseScript
 #
 #   date of creation:  04/15/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_testscript.h"

namespace vrc
{

//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( TestScriptEntityFactory )

EnTestScript::EnTestScript()
{
    // register entity attributes
    getAttributeManager().addAttribute( "scriptFile", _scriptFile  );
}

EnTestScript::~EnTestScript()
{
}

void EnTestScript::initialize()
{
    try
    {
        // load script file and scope all exposed methods with 'testScript'
        loadScript( "testScript", _scriptFile );

        // expose methods
        
        Params arguments;
        Params returnsvalues;

        //Note: for adding a parameter type into a Params container one can define an initial value ( which is normally ignored ).
        //      it is important that the compiler can deduce the 'add' method by knowing the type which is added. 
        //      currently int, float, double, and std::string types are supported.
        //      take also care on the order you define the arguments or return values.

        // expose method 1 having the pseudo-signatur: [ std::string, float ] method1( int, float )
        {
            arguments.add( 0 );       // here just pass an int variable or a constant int, we take 0
            arguments.add( 0.0f );    // here just pass a float variable or a constant float, we take 0.0f
            returnsvalues.add( std::string() );
            returnsvalues.add( 0.0f );
            exposeMethod( "exposedMethod1", &EnTestScript::exposedMethod1, arguments, returnsvalues );
        }

        // some tests
        {
            size_t argumentcontainersize = arguments.size();
            assert( argumentcontainersize == 2 );

            size_t returnsvaluescontainersize = returnsvalues.size();
            assert( returnsvaluescontainersize == 2 );

            const type_info& typeofelementatindexZero = arguments.getTypeInfo( 0 );
            assert( typeofelementatindexZero == typeid( int ) );

            const type_info& typeofelementatindexOne  = arguments.getTypeInfo( 1 );
            assert( typeofelementatindexOne == typeid( float ) );
        }
        
        arguments.clear();
        returnsvalues.clear();

        // expose method 2 having the pseudo-signatur: void method1( int, std::string )
        {
            arguments.add( 0 );
            arguments.add( std::string() );
            exposeMethod( "exposedMethod2", &EnTestScript::exposedMethod2, arguments );
        }

        // execute the script after exposing methods
        execute();

        // call script function scriptFcn1: void scriptFcn1( int )
        {
            Params args;
            args.add( 100.42 );
            
            // the function has no return value
            callScriptFunction( "scriptFcn1", &args );
        }

        // call script function scriptFcn2: [ std::string, int, int ] scriptFcn2( int, int )
        {
            Params args;
            args.add( 100 );
            args.add( 150 );

            Params rets;
            rets.add( std::string() );
            rets.add( 0 );
            rets.add( 0 );
            
            callScriptFunction( "scriptFcn2", &args, &rets );

            std::stringstream msg;
            msg << "in Entity: scriptFcn2 returned: " << GET_SCRIPT_PARAMVALUE( rets, 0, std::string ) << " "
                << GET_SCRIPT_PARAMVALUE( rets, 1, int ) << " " << GET_SCRIPT_PARAMVALUE( rets, 2, int );
            log_debug << msg.str() << std::endl;           
        }

        // close the script and clean up its resources
        closeScript();
    }
    catch( const ScriptingException& e )
    {
        log_error << "TestScript: error occured during script test: " << e.what() << std::endl;
    }

    // register entity in order to get notifications (e.g. from menu entity)
    yaf3d::EntityManager::get()->registerNotification( this, true );
    // register for getting periodic updates
    yaf3d::EntityManager::get()->registerUpdate( this, true );
}

void EnTestScript::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_LOADING_LEVEL:
            break;

        case YAF3D_NOTIFY_UNLOAD_LEVEL:
            break;

        case YAF3D_NOTIFY_DELETING_ENTITIES:
            break;

        case YAF3D_NOTIFY_BUILDING_PHYSICSWORLD:
            break;

        case YAF3D_NOTIFY_DELETING_PHYSICSWORLD:
            break;

        case YAF3D_NOTIFY_NEW_LEVEL_INITIALIZED:
            break;

        case YAF3D_NOTIFY_MENU_ENTER:
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:
            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            break;

        case YAF3D_NOTIFY_SHUTDOWN:
            break;

        default:
            ;
    }
}

void EnTestScript::updateEntity( float deltaTime )
{
}

void EnTestScript::exposedMethod1( const Params& arguments, Params& returnvalues )
{
    // get parameters
    std::stringstream msg;
    msg << "in Entity: exposedMethod1( " << GET_SCRIPT_PARAMVALUE( arguments, 0, int ) << " " << GET_SCRIPT_PARAMVALUE( arguments, 1, float ) << " )";
    log_debug << msg.str() << std::endl;

    // set return values
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, std::string, "returnvalue is: " );
    SET_SCRIPT_PARAMVALUE( returnvalues, 1, float, 0.12345f );
}

void EnTestScript::exposedMethod2( const Params& arguments, Params& returnvalues )
{
    std::stringstream msg;
    msg << "in Entity: exposedMethod2( " << GET_SCRIPT_PARAMVALUE( arguments, 0, int ) << ", " << GET_SCRIPT_PARAMVALUE( arguments, 1, std::string ) << " )";
    log_debug << msg.str() << std::endl;
}

} // namespace vrc
