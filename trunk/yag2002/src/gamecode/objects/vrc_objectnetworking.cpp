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
 # networking for interactive objects; this is a server side object.
 #
 #   date of creation:  11/01/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_baseobject.h"
#include "vrc_objectnetworking.h"
#include "../gamelogic/vrc_gamelogic.h"
#include "../storage/vrc_storageserver.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

ObjectNetworking::ObjectNetworking( BaseObject* p_objectEntity ) :
 _objectID( 0 ),
 _positionX( 0.0f ),
 _positionY( 0.0f ),
 _positionZ( 0.0f ),
 _rotationZ( 0.0f ),
 _maxPickDistance( 1.5f ),
 _maxHeighlightDistance( 10 ),
 _p_objectEntity( p_objectEntity ),
 _p_cbResult( NULL )
{
    memset( _p_meshFile, 0, sizeof( _p_meshFile ) );

    if ( p_objectEntity )
    {
        // entity is only on the server != NULL
        assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server );

        // get the entity attributes
        osg::Vec3f pos;
        osg::Vec3f rot;
        std::string meshfile;

        p_objectEntity->getAttributeManager().getAttributeValue( "meshFile", meshfile );
        p_objectEntity->getAttributeManager().getAttributeValue( "position", pos );
        p_objectEntity->getAttributeManager().getAttributeValue( "rotation", rot );
        p_objectEntity->getAttributeManager().getAttributeValue( "maxViewDistance", _maxPickDistance );
        p_objectEntity->getAttributeManager().getAttributeValue( "maxHeighlightDistance", _maxHeighlightDistance );

        _positionX = pos.x();
        _positionY = pos.y();
        _positionZ = pos.z();
        _rotationZ = rot.z();
        strcpy_s( _p_meshFile, sizeof( _p_meshFile ) - 1, meshfile.c_str() );

        // get also the object ID
        _objectID = p_objectEntity->getObjectID();
    }
}

ObjectNetworking::~ObjectNetworking()
{
}

void ObjectNetworking::PostObjectCreate()
{ // a new client has joined, this is called only on server

    // this function is called only on clients
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    // create the object entity
    std::string entitytype = ObjectRegistry::getEntityType( _objectID );
    yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->createEntity( entitytype, "_obj_" + entitytype, true );
    if ( !p_entity )
    {
        log_error << "ObjectNetworking: entity type does not exist: " << _objectID << " '" << entitytype << "'" << std::endl;
        return;
    }

    // set the entity attributes
    osg::Vec3f pos( _positionX, _positionY, _positionZ );
    osg::Vec3f rot( 0.0f, 0.0f, _rotationZ );
    _p_meshFile[ sizeof( _p_meshFile ) - 1 ] = 0;

    p_entity->getAttributeManager().setAttributeValue( "meshFile", std::string( _p_meshFile ) );
    p_entity->getAttributeManager().setAttributeValue( "position", pos );
    p_entity->getAttributeManager().setAttributeValue( "rotation", rot );
    p_entity->getAttributeManager().setAttributeValue( "maxViewDistance", _maxPickDistance );
    p_entity->getAttributeManager().setAttributeValue( "maxHeighlightDistance", _maxHeighlightDistance );

    p_entity->initialize();
    p_entity->postInitialize();

    _p_objectEntity = dynamic_cast< BaseObject* >( p_entity );
    assert( _p_objectEntity && "wrong entity type!" );
    // set object networking
    _p_objectEntity->setNetworking( this );
}

bool ObjectNetworking::RequestAction( tActionData& action, CallbackActionResult* p_cb )
{
    if ( _p_cbResult )
        return false;

    // make an RPC on server object
    MASTER_FUNCTION_CALL( RPC_RequestAction( action ) );

    // set the callback object
    _p_cbResult = p_cb;

    return true;
}

void ObjectNetworking::RPC_RequestAction( tActionData action )
{ // this is called on server

    // let the storage server validate the client first!
    if ( !StorageServer::get()->validateClient( action._userID, action._sessionCookie ) )
        return;

    std::vector< float > args;
    std::vector< float > result;
    result.push_back( 0.0f );

    if ( !GameLogic::get()->requestAction( action._actionType, action._paramUint[ 0 ], args, result ) )
        log_error << "ObjectNetworking: problem executing required action: " << action._actionType << std::endl;

    // push the result of action into first uint parameter
    action._actionResult = int( result[ 0 ] );

    ALL_REPLICAS_FUNCTION_CALL( RPC_ActionResult( action ) );
}

void ObjectNetworking::RPC_ActionResult( tActionData action )
{ // this method is called only on client

    if ( _p_cbResult )
        _p_cbResult->actionResult( action );

    _p_cbResult = NULL;
}

} // namespace vrc
