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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_npcfish.h"

namespace vrc
{

//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( NPCFishEntityFactory )

EnNPCFish::EnNPCFish():
_updateRotation( false ),
_p_fcnArgUpdate ( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "scriptFile", _scriptFile  );
    getAttributeManager().addAttribute( "meshFile",   _meshFile    );
}

EnNPCFish::~EnNPCFish()
{
}

void EnNPCFish::initialize()
{
    // try to load the fish mesh
    _mesh = yaf3d::LevelManager::get()->loadMesh( _meshFile );
    if ( !_mesh.valid() )
    {
        log_error << "*** could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return;
    }
    addToTransformationNode( _mesh.get() );

    if ( !setupScript() )
        return;

    // register entity in order to get notifications (e.g. from menu entity)
    yaf3d::EntityManager::get()->registerNotification( this, true );
    // register for getting periodic updates
    yaf3d::EntityManager::get()->registerUpdate( this, true );
}

bool EnNPCFish::setupScript()
{
    try
    {
        // load script file and scope all exposed methods with 'fish'
        loadScript( "fish", _scriptFile, BaseScript< EnNPCFish >::MATH );

        // expose methods

        Params arguments;
        Params returnsvalues;

        // expose method getPosition having the pseudo-signatur: [ float, float, float ] getPosition()
        {
            returnsvalues.add( 0.0f );
            returnsvalues.add( 0.0f );
            returnsvalues.add( 0.0f );
            exposeMethod( "getPosition", &EnNPCFish::lgetPosition, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();

        // expose method setPosition having the pseudo-signatur: void getPosition( float, float, float )
        {
            arguments.add( 0.0f );
            arguments.add( 0.0f );
            arguments.add( 0.0f );
            exposeMethod( "setPosition", &EnNPCFish::lsetPosition, arguments );
        }

        arguments.clear();
        returnsvalues.clear();

        // expose method getDirection having the pseudo-signatur: [ float, float, float ] getDirection()
        {
            returnsvalues.add( 0.0f );
            returnsvalues.add( 0.0f );
            returnsvalues.add( 0.0f );
            exposeMethod( "getDirection", &EnNPCFish::lgetDirection, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();

        // expose method setDirection having the pseudo-signatur: void setDirection( float, float, float )
        {
            arguments.add( 0.0f );
            arguments.add( 0.0f );
            arguments.add( 0.0f );
            exposeMethod( "setDirection", &EnNPCFish::lsetDirection, arguments );
        }

        // execute the script after exposing methods
        execute();

        _p_fcnArgUpdate = new Params();
        _p_fcnArgUpdate->add( 0.01f );
    }
    catch( const ScriptingException& e )
    {
        log_error << "EnNPCFish: error occured during script setup: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void EnNPCFish::handleNotification( const yaf3d::EntityNotification& notification )
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

        // we have to trigger the deletion ourselves as it is persistent
        case YAF3D_NOTIFY_SHUTDOWN:
            yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnNPCFish::updateEntity( float /*deltaTime*/ )
{
    // call script function update: void update( float deltatime )
    callScriptFunction( "update", _p_fcnArgUpdate );

    // update position and rotation of mesh
    {
        setPosition( _position );

        if ( _updateRotation )
        {
            osg::Quat   rot(
                            _direction.x(), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                            _direction.y(), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                            _direction.z(), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                            );
            setRotation( rot );
            _updateRotation = false;
        }
    }
}

void EnNPCFish::lgetPosition( const Params& /*arguments*/, Params& returnvalues )
{
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, float, _position._v[ 0 ] );
    SET_SCRIPT_PARAMVALUE( returnvalues, 1, float, _position._v[ 1 ] );
    SET_SCRIPT_PARAMVALUE( returnvalues, 2, float, _position._v[ 2 ] );
}

void EnNPCFish::lsetPosition( const Params& arguments, Params& /*returnvalues*/ )
{
    _position._v[ 0 ] = GET_SCRIPT_PARAMVALUE( arguments, 0, float );
    _position._v[ 1 ] = GET_SCRIPT_PARAMVALUE( arguments, 1, float );
    _position._v[ 2 ] = GET_SCRIPT_PARAMVALUE( arguments, 2, float );
}

void EnNPCFish::lgetDirection( const Params& /*arguments*/, Params& returnvalues )
{
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, float, _direction._v[ 0 ] );
    SET_SCRIPT_PARAMVALUE( returnvalues, 1, float, _direction._v[ 1 ] );
    SET_SCRIPT_PARAMVALUE( returnvalues, 2, float, _direction._v[ 2 ] );
}

void EnNPCFish::lsetDirection( const Params& arguments, Params& /*returnvalues*/ )
{
    _direction._v[ 0 ] = GET_SCRIPT_PARAMVALUE( arguments, 0, float );
    _direction._v[ 1 ] = GET_SCRIPT_PARAMVALUE( arguments, 1, float );
    _direction._v[ 2 ] = GET_SCRIPT_PARAMVALUE( arguments, 2, float );

    _updateRotation = true;
}

} // namespace vrc
