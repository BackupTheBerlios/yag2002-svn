/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # entity manager holding all game objects and provides services
 #  as updating and searching for entitiy by name etc.
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_application.h"
#include "ctd_entitymanager.h"

using namespace std;
using namespace CTD; 


CTD_SINGLETON_IMPL( EntityManager );


EntityManager::EntityManager() :
_stateDeletingEntities( false )
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::update( float deltaTime  )
{
    // update all entities
    updateEntities( deltaTime );

    // delete queued entities for deletion
    vector<BaseEntity*>::iterator pp_delentity = _queueDeletedEntities.begin(), pp_delentityEnd = _queueDeletedEntities.end();
    for( ; pp_delentity != pp_delentityEnd; pp_delentity++ )
    {
        vector<BaseEntity*>::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
        for( ; pp_entity != pp_entityEnd; pp_entity++ )
        {
            if ( *pp_entity == *pp_delentity )
            {
                deregisterUpdate( *pp_entity ); // removes entity from _updateEntities list
                BaseEntity* p_rementity = *pp_delentity;
                delete ( *pp_delentity );       // delete entit

                // often the update deregistration happens in entity destructor, so handle this here
                vector< std::pair< BaseEntity*, bool > >::iterator pp_updateentity = _queueUpdateEntities.begin(), pp_updateentityEnd = _queueUpdateEntities.end();
                for( ; pp_updateentity != pp_updateentityEnd; pp_updateentity++ )
                {
                    if ( pp_updateentity->first == p_rementity )
                    {
                        _queueUpdateEntities.erase( pp_updateentity );
                        break;
                    }
                }
                break;
            }
        }
    }
   _queueDeletedEntities.clear();

    // add new to be updated entities into update entity list, or remove those which are deregistered
    vector< std::pair< BaseEntity*, bool > >::iterator pp_entity = _queueUpdateEntities.begin(), pp_entityEnd = _queueUpdateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( pp_entity->second )
            _updateEntities.push_back( pp_entity->first );
        else
            deregisterUpdate( pp_entity->first );
    }
    _queueUpdateEntities.clear();
}

bool EntityManager::registerFactory( BaseEntityFactory* p_entityFactory )
{
    vector< BaseEntityFactory* >::iterator pp_entity = _entityFactories.begin(), pp_entityEnd = _entityFactories.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( **pp_entity == *p_entityFactory )
            return false;
    }
    _entityFactories.push_back( p_entityFactory );
    return true;
}

bool EntityManager::deregisterFactory( BaseEntityFactory* p_entityFactory )
{
    vector< BaseEntityFactory* >::iterator pp_entity = _entityFactories.begin(), pp_entityEnd = _entityFactories.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( **pp_entity == *p_entityFactory )
        {
            _entityFactories.erase( pp_entity );
            return true;
        }
    }

    return false;
}

BaseEntityFactory* EntityManager::getEntityFactory( const string& type )
{
    vector< BaseEntityFactory* >::iterator pp_entity = _entityFactories.begin(), pp_entityEnd = _entityFactories.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->getType() == type )
            return *pp_entity;
    }

    return NULL;
}
 
BaseEntity* EntityManager::createEntity( const string& type, const string& instanceName )
{
    BaseEntityFactory* p_type = getEntityFactory( type );
    if ( !p_type )
        return NULL;

    BaseEntity* p_ent = p_type->createEntity();
    p_ent->setInstanceName( instanceName );
    return p_ent;
}

BaseEntity* EntityManager::findEntity( const std::string& type, const std::string& instanceName )
{
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->getTypeName() == type )
        {
            if ( instanceName.empty() )
            {
                return *pp_entity;
            }
            else
            {
                if ( ( *pp_entity )->getInstanceName() == instanceName )
                    return *pp_entity;
            }
        }
    }

    return NULL;
}

BaseEntity* EntityManager::findInstance( const string& instanceName )
{
    assert ( instanceName.length() && "instance name is empty!" );
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->getInstanceName() == instanceName )
        {
            return *pp_entity;
        }
    }

    return NULL;
}

void EntityManager::registerUpdate( CTD::BaseEntity* p_entity, bool update )
{
    // check whether the entity is already registered
#ifdef _DEBUG
    if ( update )
    {
        vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
        for(; pp_entity != pp_entityEnd; pp_entity++ )
        {
            if ( *pp_entity == p_entity )
                break;
        }
        assert( pp_entity == pp_entityEnd );
    }
#endif

    _queueUpdateEntities.push_back( make_pair( p_entity, update ) );
}

void EntityManager::deregisterUpdate( BaseEntity* p_entity )
{
    // if we are in the phase of deleting entities so avoid entity list manipulation!
    if ( _stateDeletingEntities )
        return;

    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
        {
            _updateEntities.erase( pp_entity );
            return;
        }
    }
    assert( NULL && "request for entity update deregistration: entity was not registered before!" );
}

//! TODO implement a real adding functionality basing on spatial partitioning
void EntityManager::addToScene( BaseEntity* p_entity, osg::Group *p_scenegr )
{
    osg::Group *p_grp = p_scenegr ? p_scenegr : ( osg::Group* )Application::get()->getViewer()->getTopMostSceneData();
    p_grp->addChild( p_entity );
}

void EntityManager::addToInitList( BaseEntity* p_entity )
{
    _entityInitList.push_back( p_entity );
}

void EntityManager::clearInitList()
{
    _entityInitList.clear();
}

void EntityManager::initializeEntities()
{
    update(0);
    vector< BaseEntity* >::iterator pp_entity = _entityInitList.begin(), pp_entityEnd = _entityInitList.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
        ( *pp_entity )->initialize();
}

void EntityManager::postInitializeEntities()
{
    vector< BaseEntity* >::iterator pp_entity = _entityInitList.begin(), pp_entityEnd = _entityInitList.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
        ( *pp_entity )->postInitialize();
}

void EntityManager::updateEntities( float deltaTime  )
{
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->isActive() )
            ( *pp_entity )->updateEntity( deltaTime );
    }
}

void EntityManager::deleteEntity( BaseEntity* p_entity )
{// enqueue entity for deletion. the real deletion will occur on next update phase    
    // in debug build check for registration
#ifdef _DEBUG
    vector<BaseEntity*>::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
        {
            _queueDeletedEntities.push_back( p_entity );
            break;
        }
    }
    assert( pp_entity != pp_entityEnd && "entity is not registered, it cannot be deleted!" );
#else
    _queueDeletedEntities.push_back( p_entity );
#endif
}

void EntityManager::deleteAllEntities()
{
    _stateDeletingEntities = true; // set this state so deregistering has no effect on entity list in this phase
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
        delete *pp_entity;

    _stateDeletingEntities = false;
    _updateEntities.clear();
}

void EntityManager::shutdown()
{
    deleteAllEntities();
    // destroy singleton
    destroy();    
}

// entity factory base class
//-----------------------
BaseEntityFactory::BaseEntityFactory( const std::string& entityTypeName ) : 
_typeTypeName( entityTypeName ) 
{
    EntityManager::get()->registerFactory( this );
}
        
BaseEntityFactory::~BaseEntityFactory()
{
    EntityManager::get()->deregisterFactory( this );
}

