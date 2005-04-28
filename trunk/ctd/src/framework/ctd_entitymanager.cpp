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
 # entity manager holding all game objects and provides services
 #  such as updating and searching for entities by name etc.
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
#include "ctd_log.h"

using namespace std;
using namespace CTD; 


CTD_SINGLETON_IMPL( EntityManager );


EntityManager::EntityManager() :
_internalState( None ),
_shuttingDown( false )
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::shutdown()
{
    // first send out a shutdown notification
    EntityNotification ennotify( CTD_NOTIFY_SHUTDOWN );
    EntityManager::get()->sendNotification( ennotify );
    // handle all pending requests for last time
    //  here entities with autoDelete flag false have the last chance to be deleted
    updateEntityLists();

    // set shutdown flag to avoid entity list corruptions during cleanup
    _shuttingDown = true;
    deleteAllEntities();

    // destroy singleton
    destroy();    
}

void EntityManager::update( float deltaTime  )
{
    // update internal entity lists and queues
    updateEntityLists();

    // now update all entities
    updateEntities( deltaTime );
}

void EntityManager::updateEntityLists()
{
    // set internal state
    _internalState = UpdatingEntities;

    // delete queued entities for deletion
    vector< BaseEntity* >::iterator pp_delentity = _queueDeletedEntities.begin(), pp_delentityEnd = _queueDeletedEntities.end();
    for( ; pp_delentity != pp_delentityEnd; pp_delentity++ )
    {
        // remove entity from pool and delete it
        removeFromEntityPool( *pp_delentity, true );

        vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
        for( ; pp_entity != pp_entityEnd; pp_entity++ )
        {
            if ( *pp_entity == *pp_delentity )
            {
                deregisterUpdate( *pp_entity );                     // removes entity from _updateEntities list
                BaseEntity* p_rementity = *pp_delentity;

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

    // set internal state
    _internalState = None;
}

bool EntityManager::registerFactory( BaseEntityFactory* p_entityFactory, bool reg )
{
    vector< BaseEntityFactory* >::iterator pp_entity = _entityFactories.begin(), pp_entityEnd = _entityFactories.end();
    if ( reg )
    {
        for(; pp_entity != pp_entityEnd; pp_entity++ )
        {
            if ( **pp_entity == *p_entityFactory )
                return false;
        }
        _entityFactories.push_back( p_entityFactory );
        return true;
    }
    else
    {
        for(; pp_entity != pp_entityEnd; pp_entity++ )
        {
            if ( **pp_entity == *p_entityFactory )
            {
                _entityFactories.erase( pp_entity );
                return true;
            }
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
 
BaseEntity* EntityManager::createEntity( const string& type, const string& instanceName, bool addToPool )
{
    BaseEntityFactory* p_type = getEntityFactory( type );
    if ( !p_type )
        return NULL;

    BaseEntity* p_ent = p_type->createEntity();
    p_ent->setInstanceName( instanceName );
    
    // add entity into internal pool
    if ( addToPool )
        addToEntityPool( p_ent );
    
    return p_ent;
}

BaseEntity* EntityManager::findEntity( const std::string& type, const std::string& instanceName )
{
    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
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
    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->getInstanceName() == instanceName )
        {
            return *pp_entity;
        }
    }

    return NULL;
}

bool EntityManager::registerUpdate( CTD::BaseEntity* p_entity, bool reg )
{

    // check whether the entity is already registered
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
            break;
    }
    if ( !reg && ( pp_entity == pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** EntityManager: the entity was previousely not registered for updating!, ignoring deregister request" << endl;
        return false;
    }
    else if ( reg && ( pp_entity != pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** EntityManager: the entity is already registered for updating!, ignoring register request" << endl;
        return false;
    }

    _queueUpdateEntities.push_back( make_pair( p_entity, reg ) );
    return true;
}

bool EntityManager::registerNotification( BaseEntity* p_entity, bool reg )
{
    // check whether the entity is already registered
    vector< BaseEntity* >::iterator pp_entity = _entityNotification.begin(), pp_entityEnd = _entityNotification.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
            break;
    }
    if ( !reg && ( pp_entity == pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** EntityManager: the entity was previousely not registered for getting notification!, ignoring deregister request" << endl;
        return false;
    }
    else if ( reg && ( pp_entity != pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** EntityManager: the entity is already registered for getting notification!, ignoring register request" << endl;
        return false;
    }

    if ( reg )
        _entityNotification.push_back( p_entity );
    else
        _entityNotification.erase( pp_entity );

    return true;
}

void EntityManager::sendNotification( const EntityNotification& notify )
{
    // send notification to registered entities
    vector< BaseEntity* >::iterator pp_entity = _entityNotification.begin(), pp_entityEnd = _entityNotification.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        ( *pp_entity )->handleNotification( const_cast< EntityNotification& >( notify ) );
    }
}

void EntityManager::deregisterUpdate( BaseEntity* p_entity )
{
    // if we are in the phase of deleting entities so avoid entity list manipulation!
    if ( _shuttingDown )
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
#ifdef _DEBUG
    assert( NULL && "request for entity update deregistration: entity was not registered before!" );
#endif
}

//! TODO(?) implement a real adding functionality basing on spatial partitioning
void EntityManager::addToScene( BaseEntity* p_entity, osg::Group *p_scenegr )
{
    assert( p_entity && p_entity->_p_transformNode && "adding to scene requires a transformation node in entity!" );
    osg::Group *p_grp = p_scenegr ? p_scenegr : ( osg::Group* )Application::get()->getViewer()->getTopMostSceneData();
    p_grp->addChild( p_entity->_p_transformNode );
}

void EntityManager::addToEntityPool( BaseEntity* p_entity )
{
    // if entities cause this call (e.g. by entity cloning) during initialization phase then delay the actual
    //  adding to entity pool until next update (otherwise the pool list gets corrupted).
    if ( ( _internalState == InitializingEntities ) || ( _internalState == PostInitializingEntities ) )
        _queueAddToPoolEntities.push_back( p_entity );
    else
        _entityPool.push_back( p_entity );
}

void EntityManager::removeFromEntityPool( BaseEntity* p_entity, bool del )
{
    assert ( ( _internalState == UpdatingEntities ) && "internal error: this method can be called only during updating phase" );

    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
        if ( ( *pp_entity ) == p_entity )
            break;

    if ( pp_entity != pp_entityEnd )
    {
        if ( del )
            delete ( *pp_entity );

        _entityPool.erase( pp_entity );
    } 
    else
    {
        assert( NULL && "to be removed entity does not exist!" );
    }
}

void EntityManager::initializeEntities()
{
    // set internal state
    _internalState = InitializingEntities;

    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
    {
        log << Log::LogLevel( Log::L_DEBUG ) << " initializing entity '" << ( *pp_entity )->getInstanceName() << 
            "', of type '" << ( *pp_entity )->getTypeName() << "'" << endl;
        ( *pp_entity )->initialize();
    }

    // set internal state
    _internalState = None;
}

void EntityManager::postInitializeEntities()
{
    // set internal state
    _internalState = PostInitializingEntities;

    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
    {
        log << Log::LogLevel( Log::L_DEBUG ) << " post-initializing entity '" << ( *pp_entity )->getInstanceName() << 
            "', of type '" << ( *pp_entity )->getTypeName() << "'" << endl;
        ( *pp_entity )->postInitialize();
    }

    // add new entities to pool if the request came in during initialization or post-initialization
    vector< BaseEntity* >::iterator pp_addtopoolentity = _queueAddToPoolEntities.begin(), pp_addtopoolentitiyEnd = _queueAddToPoolEntities.end();
    for ( ; pp_addtopoolentity != pp_addtopoolentitiyEnd; pp_addtopoolentity++ )
        _entityPool.push_back( *pp_addtopoolentity );

    // set internal state
    _internalState = None;
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
{
    // if the deleteEntity is called during entity deletions via a e.g. entity destructor then ignore further handling
    if ( _shuttingDown )
        return;

    // enqueue entity for deletion. the real deletion will occur on next update phase
    _queueDeletedEntities.push_back( p_entity );
}

void EntityManager::deleteAllEntities()
{
    // deleting all entities is handled as shutdown
    _shuttingDown = true;

    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
        if ( ( *pp_entity )->getAutoDelete() ) // delete only if auto delete flag is set
            _queueDeletedEntities.push_back( *pp_entity );

    // update entity lists in order to let the deletions take place immediately
    updateEntityLists();
    
    // clean up lists
    _updateEntities.clear();
    _entityPool.clear();
    _queueDeletedEntities.clear();
    _queueUpdateEntities.clear();

    _shuttingDown = false;
}

// entity factory base class
//-----------------------
BaseEntityFactory::BaseEntityFactory( const std::string& entityTypeName ) : 
_typeTypeName( entityTypeName ) 
{
    // register factory
    EntityManager::get()->registerFactory( this, true );
}
        
BaseEntityFactory::~BaseEntityFactory()
{
    // deregister factory
    EntityManager::get()->registerFactory( this, false );
}

