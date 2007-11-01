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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_objectnetworking.h"
#include "vrc_baseobject.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

ObjectNetworking::ObjectNetworking( unsigned int ID ) :
 _objectID( ID ),
 _positionX( 0.0f ),
 _positionY( 0.0f ),
 _positionZ( 0.0f ),
 _rotationZ( 0.0f ),
 _maxPickDistance( 1.5f ),
 _maxHeighlightDistance( 10 )
{
    memset( _p_meshFile, 0, sizeof( _p_meshFile ) );
}

ObjectNetworking::~ObjectNetworking()
{
}

void ObjectNetworking::setPosition( const osg::Vec3f& pos )
{
    _positionX = pos.x();
    _positionY = pos.y();
    _positionZ = pos.z();
}

//! Set rotation about Z axis
void ObjectNetworking::setRotation( float rotZ )
{
    _rotationZ = rotZ;
}

void ObjectNetworking::setMeshFile( const std::string& meshFile )
{
    strcpy_s( _p_meshFile, sizeof( _p_meshFile ) - 1, meshFile.c_str() );
}

void ObjectNetworking::setMaxHeighlightDistance( float maxHeighlightDistance )
{
    maxHeighlightDistance = maxHeighlightDistance;
}

void ObjectNetworking::setMaxPickDistance( float maxPickDistance )
{
    _maxPickDistance = maxPickDistance;
}


void ObjectNetworking::PostObjectCreate()
{ // a new client has joined

    // this function is called only on clients
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    // create the object entity
    yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->createEntity( ObjectRegistry::getEntityType( _objectID ), "_obj_", true );
    if ( !p_entity )
    {
        log_error << "ObjectNetworking: entity type does not exist: " << _objectID << " '" << ObjectRegistry::getEntityType( _objectID ) << "'" << std::endl;
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
}

void ObjectNetworking::RPC_Respawn( tObjectData info )
{ // used by connecting client

    //assert( p_callback && "a valid callback for account request is needed!" );
    //assert( ( _p_accountInfoCallback == NULL ) && "only one request for account info can be handled at the same time!" );

    //_p_accountInfoCallback = p_callback;

    //// call the account info rpc on server
    //tAccountInfoData info;
    //memset( &info, 0, sizeof( info ) );
    //info._userID = userID;
    //MASTER_FUNCTION_CALL( RPC_RequestAccountInfo( info ) );
}

void ObjectNetworking::RPC_Use( tObjectData info )
{ // this method is called only on server

    // sent out the account info result
//    ALL_REPLICAS_FUNCTION_CALL( RPC_AccountInfoResult( info ) );
}

} // namespace vrc
