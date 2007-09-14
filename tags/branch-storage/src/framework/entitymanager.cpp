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
 # entity manager holding all game objects and provides services
 #  such as updating and searching for entities by name etc.
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "levelmanager.h"
#include "entitymanager.h"
#include "log.h"
#include "utils.h"

namespace yaf3d
{

// Implement the singleton instance
YAF3D_SINGLETON_IMPL( EntityManager )


EntityManager::EntityManager() :
_updateEntityListChanged( false ),
_internalState( None )
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::shutdown()
{
    // first send out a shutdown notification
    EntityNotification ennotify( YAF3D_NOTIFY_SHUTDOWN );
    sendNotification( ennotify );
    flushNotificationQueue();

    // delete all entities ( be aware that persistent entities must trigger their deletion themself )
    deleteAllEntities();

    // check if some entities have been forgotten by game code to delete
    if ( _entityPool.size() > 0 )
    {
        log_error << "EntityManager: shutdown, attention, there are '" <<  _entityPool.size() << "' remaining entities after shutdown!" << std::endl;
        for ( size_t cnt = 0; cnt < _entityPool.size(); ++cnt )
            log_error << "  - " << _entityPool[ cnt ]->getInstanceName() << "(" << _entityPool[ cnt ]->getTypeName() << ")" << std::endl;
    }

    // destroy singleton
    destroy();    
}

void EntityManager::update( float deltaTime  )
{
    // flush the notification queue first as update registrations can occure during notifications
    flushNotificationQueue();

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
    std::vector< BaseEntity* >::iterator p_delentity = _queueDeletedEntities.begin(), p_delentityEnd = _queueDeletedEntities.end();
    for ( ; p_delentity != p_delentityEnd; ++p_delentity )
    {
        BaseEntity* p_rementity = *p_delentity;
        // remove entity from pool and delete it
        removeFromEntityPool( p_rementity, true );
    }
    _queueDeletedEntities.clear();

    // add new to be updated entities into update entity list, or remove those which are deregistered
    std::vector< std::pair< BaseEntity*, bool > >::iterator p_beg = _queueUpdateEntities.begin(), p_end = _queueUpdateEntities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( p_beg->second )
            _updateEntities.push_back( p_beg->first );
        else
            deregisterUpdate( p_beg->first );
    }
    _queueUpdateEntities.clear();

    // set internal state
    _internalState = None;
}

bool EntityManager::registerFactory( BaseEntityFactory* p_entityFactory, bool reg )
{
    std::vector< BaseEntityFactory* >::iterator p_beg = _entityFactories.begin(), p_end = _entityFactories.end();
    if ( reg )
    {
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( **p_beg == *p_entityFactory )
                return false;
        }
        _entityFactories.push_back( p_entityFactory );
        return true;
    }
    else
    {
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( **p_beg == *p_entityFactory )
            {
                _entityFactories.erase( p_beg );
                return true;
            }
        }
    }
    return false;    
}

BaseEntityFactory* EntityManager::getEntityFactory( const std::string& type )
{
    std::vector< BaseEntityFactory* >::iterator p_beg = _entityFactories.begin(), p_end = _entityFactories.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->getType() == type )
            return *p_beg;
    }

    return NULL;
}
 
void EntityManager::getAllEntityFactories( std::vector< BaseEntityFactory* >& factories )
{
    std::vector< BaseEntityFactory* >::iterator p_fac = _entityFactories.begin(), p_facEnd = _entityFactories.end();
    for ( ; p_fac != p_facEnd; ++p_fac )
        factories.push_back( *p_fac );
}

BaseEntity* EntityManager::createEntity( const std::string& type, const std::string& instanceName, bool addToPool )
{
    BaseEntityFactory* p_type = getEntityFactory( type );
    if ( !p_type )
        return NULL;

    BaseEntity* p_ent = p_type->createEntity();
    p_ent->setInstanceName( instanceName );
    
    // add entity into internal pool
    if ( addToPool )
        addToEntityPool( p_ent );
    
    // create a transformation node if needed
    if ( p_ent->isTransformable() ) 
    {
        osg::PositionAttitudeTransform  *p_trans = new osg::PositionAttitudeTransform;
        p_ent->setTransformationNode( p_trans );
    }

    return p_ent;
}

BaseEntity* EntityManager::findEntity( const std::string& type, const std::string& instanceName )
{
    std::vector< BaseEntity* >::iterator p_beg = _entityPool.begin(), p_end = _entityPool.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->getTypeName() == type )
        {
            if ( instanceName.empty() )
            {
                return *p_beg;
            }
            else
            {
                if ( ( *p_beg )->getInstanceName() == instanceName )
                    return *p_beg;
            }
        }
    }

    return NULL;
}

BaseEntity* EntityManager::findInstance( const std::string& instanceName )
{
    assert ( instanceName.length() && "instance name is empty!" );
    std::vector< BaseEntity* >::iterator p_beg = _entityPool.begin(), p_end = _entityPool.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->getInstanceName() == instanceName )
        {
            return *p_beg;
        }
    }

    return NULL;
}

void EntityManager::getAllEntities( std::vector< BaseEntity* >& entities )
{
    std::vector< BaseEntity* >::iterator p_beg = _entityPool.begin(), p_end = _entityPool.end();
    for ( ; p_beg != p_end; ++p_beg )
        entities.push_back( *p_beg );
}

bool EntityManager::registerUpdate( yaf3d::BaseEntity* p_entity, bool reg )
{
    // check whether the entity is registered for updates
    std::vector< BaseEntity* >::iterator p_beg = _updateEntities.begin(), p_end = _updateEntities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_entity )
            break;
    }
    if ( !reg && ( p_beg == p_end ) )
    {
        log_warning << "EntityManager: the entity was previousely not registered for updating!, ignoring deregister request" << std::endl;
        log_out << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << std::endl;
        return false;
    }
    else if ( reg && ( p_beg != p_end ) )
    {
        log_warning << "EntityManager: the entity is already registered for updating!, ignoring register request" << std::endl;
        log_out << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << std::endl;
        return false;
    }

    _queueUpdateEntities.push_back( std::make_pair( p_entity, reg ) );
    return true;
}

bool EntityManager::isRegisteredUpdate( const BaseEntity* p_entity )
{
    // check whether the entity is registered for updates
    std::vector< BaseEntity* >::iterator p_beg = _updateEntities.begin(), p_end = _updateEntities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_entity ) 
            return true;
    }
    return false;
}

bool EntityManager::registerNotification( BaseEntity* p_entity, bool reg )
{
    // check whether the entity is registered for notifications
    std::vector< BaseEntity* >::iterator p_beg = _entityNotification.begin(), p_end = _entityNotification.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_entity )
            break;
    }
    if ( !reg && ( p_beg == p_end ) )
    {
        log_warning << "EntityManager: the entity was previousely not registered for getting notification!, ignoring deregister request" << std::endl;
        log_out << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << std::endl;
        return false;
    }
    else if ( reg && ( p_beg != p_end ) )
    {
        log_warning << "EntityManager: the entity is already registered for getting notification!, ignoring register request" << std::endl;
        log_out << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << std::endl;
        return false;
    }

    if ( reg )
        _entityNotification.push_back( p_entity );
    else
        _entityNotification.erase( p_beg );

    return true;
}

bool EntityManager::isRegisteredNotification( const BaseEntity* p_entity )
{
    // check whether the entity is registered for notifications
    std::vector< BaseEntity* >::iterator p_beg = _entityNotification.begin(), p_end = _entityNotification.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_entity )
            return true;
    }
    return false;
}

void EntityManager::sendNotification( const EntityNotification& notification )
{
    _queueNotifications.push_back( notification );
}

void EntityManager::sendNotification( const EntityNotification& notification, BaseEntity* p_entity )
{
    p_entity->handleNotification( notification );
}

void EntityManager::flushNotificationQueue()
{
    std::vector< EntityNotification >::iterator p_notify = _queueNotifications.begin(), p_notifyEnd = _queueNotifications.end();
    std::vector< BaseEntity* >::iterator p_beg, p_end    = _entityNotification.end();
    for ( ; p_notify != p_notifyEnd; ++p_notify )
    {
        for ( p_beg = _entityNotification.begin(); p_beg != p_end; ++p_beg )
        {
            ( *p_beg )->handleNotification( *p_notify );
        }
    }
    _queueNotifications.clear();
}

void EntityManager::deregisterUpdate( BaseEntity* p_entity )
{
    std::vector< BaseEntity* >::iterator p_beg = _updateEntities.begin(), p_end = _updateEntities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_entity )
        {
            _updateEntities.erase( p_beg );
            return;
        }
    }

    assert( NULL && "request for entity update deregistration: entity was not registered before!" );
}

void EntityManager::addToScene( BaseEntity* p_entity, osg::Group *p_scenegr )
{
    assert( p_entity && p_entity->_p_transformNode.valid() && "adding to scene requires a transformation node in entity!" );
    osg::Group *p_grp = p_scenegr ? p_scenegr : LevelManager::get()->getEntityNodeGroup().get();
    p_grp->addChild( p_entity->_p_transformNode.get() );
}


void EntityManager::removeFromScene( BaseEntity* p_entity )
{
    assert( p_entity && p_entity->_p_transformNode.valid() && "removing from the scene requires a transformation node in entity!" );
    unsigned int parents = p_entity->_p_transformNode->getNumParents();
    if ( parents > 0 )
    {
        for ( unsigned int p = 0; p < parents; ++p )
        {
            // every removal of child also removes the parent form parent list! so remove always form index 0.
            p_entity->_p_transformNode->getParent( 0 )->removeChild( p_entity->_p_transformNode.get() );
        }
    }
}

void EntityManager::addToEntityPool( BaseEntity* p_entity )
{
    // if entities cause this call ( e.g. by entity cloning or creation ) during setup phase then delay the actual
    //  adding to entity pool until next update ( otherwise the pool list gets corrupted during setup phase ).
    if ( ( _internalState == InitializingEntities ) || ( _internalState == PostInitializingEntities ) )
        _queueAddToPoolEntities.push_back( p_entity );
    else
        _entityPool.push_back( p_entity );
}

void EntityManager::removeFromEntityPool( BaseEntity* p_entity, bool del )
{
    assert ( ( _internalState == UpdatingEntities ) && "internal error: this method can be called only during updating phase" );

    std::vector< BaseEntity* >::iterator p_beg = _entityPool.begin(), p_end = _entityPool.end();
    for (; p_beg != p_end; ++p_beg )
        if ( ( *p_beg ) == p_entity )
            break;

    if ( p_beg != p_end )
    {
        // if the entity must be deleted so we have to remove it also from update and notification list
        if ( del )
        {
            // remove notification registration
            if ( isRegisteredNotification( p_entity ) )
                registerNotification( p_entity, false );

            // remove update registration
            if ( isRegisteredUpdate( p_entity ) )
                deregisterUpdate( p_entity );

            delete ( *p_beg );
        }

        _entityPool.erase( p_beg );
    } 
    else
    {
        log_error << "EntityManager: removeFromEntityPool, request for entity removal which does not exist!" << std::endl;
    }
}

void EntityManager::setupEntities( std::vector< BaseEntity* >& entities )
{
    osg::Timer      timer;
    osg::Timer_t    curTick         = 0;
    osg::Timer_t    startTick       = 0;
    float           time4Init       = 0;
    float           time4PostInit   = 0;

    log_info << "EntityManager: starting entity setup ..." << std::endl;

    std::vector< BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
    BaseEntity* p_entity = NULL;
    log_info << "EntityManager: initializing entities ..." << std::endl;
    // setup entities
    {
        // set internal state
        _internalState = InitializingEntities;

        startTick  = timer.tick();
        // consider persistence and avoid re-initialization of persistent entities
        for ( ; p_beg != p_end; ++p_beg )
        {
            p_entity = *p_beg;
            log_debug << " # " << p_entity->getInstanceName() + "[ " + p_entity->getTypeName() + " ]" << std::endl;

            if ( p_entity->isPersistent() )
            {
                if ( !p_entity->isInitialized() )
                {
                    p_entity->initialize();
                }
            }
            else
            {
                p_entity->setInitialized( true ); // there is no actual sense for setting the flag for non-persitent entities!
                p_entity->initialize();
            }
        }
        
        curTick    = timer.tick();
        time4Init  = timer.delta_s( startTick, curTick );
    }

    p_beg = entities.begin(); p_end = entities.end();
    log_info << "EntityManager: post-initializing entities ..." << std::endl;
    {
        // set internal state
        _internalState = PostInitializingEntities;

        startTick  = timer.tick();
        // consider persistence and avoid re-initialization of persistent entities
        for ( ; p_beg != p_end; ++p_beg )
        {
            p_entity = *p_beg;
            log_debug << " # " << p_entity->getInstanceName() + "[ " + p_entity->getTypeName() + " ]" << std::endl;

            if ( p_entity->isPersistent() )
            {
                if ( !p_entity->isInitialized() )
                {
                    p_entity->postInitialize();
                    p_entity->setInitialized( true ); // now set the flag to avoid a new setup later
                }
            }
            else
            {
                p_entity->setInitialized( true ); // there is no actual sense for setting the flag for non-persitent entities, but we do it though!
                p_entity->postInitialize();
            }
        }
        curTick        = timer.tick();
        time4PostInit  = timer.delta_s( startTick, curTick );
    }
    {
        // add new entities to pool if the request came in during initialization or post-initialization
        std::vector< BaseEntity* >::iterator p_addtopoolentity = _queueAddToPoolEntities.begin(), p_addtopoolentitiyEnd = _queueAddToPoolEntities.end();
        for ( ; p_addtopoolentity != p_addtopoolentitiyEnd; ++p_addtopoolentity )
            _entityPool.push_back( *p_addtopoolentity );

        _queueAddToPoolEntities.clear();

        // set internal state
        _internalState = None;
    }

    log_info << "EntityManager: needed time for initialization: " << time4Init << " seconds" << std::endl;
    log_info << "EntityManager: needed time for post-initialization: " << time4PostInit << " seconds" <<  std::endl;
    log_info << "EntityManager: total time for setting up: " << time4Init + time4PostInit << " seconds" <<  std::endl;

}

void EntityManager::updateEntities( float deltaTime  )
{
    std::vector< BaseEntity* >::iterator p_beg = _updateEntities.begin(), p_end = _updateEntities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        BaseEntity* p_ent = ( *p_beg );
        p_ent->updateEntity( deltaTime );

        // check heap if enabled ( used for detecting heap corruptions )
        YAF3D_CHECK_HEAP();

        // this check enables entities to manipulate the update entity list in their update method
        //  such manipulation can occure e.g. when an entity requests a level loading in its update method
        if ( _updateEntityListChanged )
        {
            _updateEntityListChanged = false;
            break;
        }
    }
}

void EntityManager::deleteEntity( BaseEntity* p_entity )
{
    // enqueue entity for deletion. the real deletion will occur on next update phase
    _queueDeletedEntities.push_back( p_entity );
}

void EntityManager::deleteAllEntities()
{
    std::vector< BaseEntity* >::iterator p_beg = _entityPool.begin(), p_end = _entityPool.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        // delete only if entity is not persistent
        if ( !( ( *p_beg )->isPersistent() ) )
        {
            _queueDeletedEntities.push_back( *p_beg );
        }
    }

    // update entity lists in order to let the deletions take place immediately
    updateEntityLists();

    // reset update list, re-add persistent entities which are in current update list
    std::vector< BaseEntity* > newupdatelist;
    std::vector< BaseEntity* >::iterator p_uentity = _entityPool.begin(), p_uentityEnd = _entityPool.end();
    for ( ; p_uentity != p_uentityEnd; ++p_uentity )
    {
        if ( ( *p_uentity )->isPersistent() )
        {            
            std::vector< BaseEntity* >::iterator p_pentity = _updateEntities.begin(), p_pentityEnd = _updateEntities.end();
            for ( ; p_pentity != p_pentityEnd; ++p_pentity )
                if ( *p_uentity == *p_pentity )
                    break;
            if ( p_pentity != p_pentityEnd )
                newupdatelist.push_back( *p_uentity );
        }
    }
    _updateEntities.clear();
    _updateEntities = newupdatelist;

    // this flag is used in main update entity loop, used for allowing entity deletions during update phase
    _updateEntityListChanged = true;
}

void EntityManager::getPersistentEntities( std::vector< BaseEntity* >& entities )
{
    std::vector< BaseEntity* >::iterator p_beg = _entityPool.begin(), p_end = _entityPool.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->isPersistent() )
            entities.push_back( *p_beg );
    }
}

// entity factory base class
//-----------------------
BaseEntityFactory::BaseEntityFactory( const std::string& entityTypeName, unsigned int ntype ) : 
_typeTypeName( entityTypeName ),
_creationPolicy( ntype )
{
    // register factory
    EntityManager::get()->registerFactory( this, true );
}
        
BaseEntityFactory::~BaseEntityFactory()
{
    // deregister factory
    EntityManager::get()->registerFactory( this, false );
}

} // namespace yaf3d
