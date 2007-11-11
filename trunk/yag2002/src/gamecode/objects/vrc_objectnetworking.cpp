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
#include "vrc_objectnetworking.h"
#include "vrc_baseobject.h"
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
 _p_objectEntity( p_objectEntity )
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


//we need also the functions: drop, pick


void ObjectNetworking::RequestUseObject( unsigned int userID )
{
    // make an RPC on server object
    MASTER_FUNCTION_CALL( RPC_RequestUse( userID ) );
}

void ObjectNetworking::RPC_RequestUse( unsigned int userID )
{ // this is called on server


    //! TODO: use the storage to use the object

    ALL_REPLICAS_FUNCTION_CALL( RPC_Use( userID ) );
}

void ObjectNetworking::RPC_Use( unsigned int userID )
{ // this method is called only on client

}

} // namespace vrc
