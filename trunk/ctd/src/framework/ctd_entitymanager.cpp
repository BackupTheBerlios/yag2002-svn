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
#include "ctd_utils.h"

using namespace std;
using namespace CTD; 


CTD_SINGLETON_IMPL( EntityManager );


EntityManager::EntityManager() :
_internalState( None ),
_updateEntityListChanged( false )
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::shutdown()
{
    // first send out a shutdown notification
    EntityNotification ennotify( CTD_NOTIFY_SHUTDOWN );
    sendNotification( ennotify );
    flushNotificationQueue();

    // delete all entities ( be aware that persistent entities must trigger their deletion themself )
    deleteAllEntities();

    // check if some entities have been forgotten by game code to delete
    if ( _entityPool.size() > 0 )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "EntityManager::shutdown: attention, there are '" <<  _entityPool.size() << "' remaining entities after shutdown!" << endl;
        for ( size_t cnt = 0; cnt < _entityPool.size(); cnt ++ )
            log << Log::LogLevel( Log::L_ERROR ) << "  - " << _entityPool[ cnt ]->getInstanceName() << "(" << _entityPool[ cnt ]->getTypeName() << ")" << endl;
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
    vector< BaseEntity* >::iterator pp_delentity = _queueDeletedEntities.begin(), pp_delentityEnd = _queueDeletedEntities.end();
    for( ; pp_delentity != pp_delentityEnd; pp_delentity++ )
    {
        BaseEntity* p_rementity = *pp_delentity;
        // remove entity from pool and delete it
        removeFromEntityPool( p_rementity, true );
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
        for( ; pp_entity != pp_entityEnd; pp_entity++ )
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
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->getType() == type )
            return *pp_entity;
    }

    return NULL;
}
 
void EntityManager::getAllEntityFactories( std::vector< BaseEntityFactory* >& factories )
{
    vector< BaseEntityFactory* >::iterator pp_fac = _entityFactories.begin(), pp_facEnd = _entityFactories.end();
    for( ; pp_fac != pp_facEnd; pp_fac++ )
        factories.push_back( *pp_fac );
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

void EntityManager::getAllEntities( std::vector< BaseEntity* >& entities )
{
    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
        entities.push_back( *pp_entity );
}

bool EntityManager::registerUpdate( CTD::BaseEntity* p_entity, bool reg )
{
    // check whether the entity is registered for updates
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
            break;
    }
    if ( !reg && ( pp_entity == pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** EntityManager: the entity was previousely not registered for updating!, ignoring deregister request" << endl;
        log << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << endl;
        return false;
    }
    else if ( reg && ( pp_entity != pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** EntityManager: the entity is already registered for updating!, ignoring register request" << endl;
        log << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << endl;
        return false;
    }

    _queueUpdateEntities.push_back( make_pair( p_entity, reg ) );
    return true;
}

bool EntityManager::isRegisteredUpdate( const BaseEntity* p_entity )
{
    // check whether the entity is registered for updates
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity ) 
            return true;
    }
    return false;
}

bool EntityManager::registerNotification( BaseEntity* p_entity, bool reg )
{
    // check whether the entity is registered for notifications
    vector< BaseEntity* >::iterator pp_entity = _entityNotification.begin(), pp_entityEnd = _entityNotification.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
            break;
    }
    if ( !reg && ( pp_entity == pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** EntityManager: the entity was previousely not registered for getting notification!, ignoring deregister request" << endl;
        log << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << endl;
        return false;
    }
    else if ( reg && ( pp_entity != pp_entityEnd ) )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** EntityManager: the entity is already registered for getting notification!, ignoring register request" << endl;
        log << "    entity type:" << p_entity->getTypeName() << ", instance name: " << p_entity->getInstanceName() << endl;
        return false;
    }

    if ( reg )
        _entityNotification.push_back( p_entity );
    else
        _entityNotification.erase( pp_entity );

    return true;
}

bool EntityManager::isRegisteredNotification( const BaseEntity* p_entity )
{
    // check whether the entity is registered for notifications
    vector< BaseEntity* >::iterator pp_entity = _entityNotification.begin(), pp_entityEnd = _entityNotification.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( *pp_entity == p_entity )
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
    vector< EntityNotification >::iterator p_notify = _queueNotifications.begin(), p_notifyEnd = _queueNotifications.end();
    vector< BaseEntity* >::iterator pp_entity, pp_entityEnd = _entityNotification.end();
    for ( ; p_notify != p_notifyEnd; p_notify++ )
    {
        for( pp_entity = _entityNotification.begin(); pp_entity != pp_entityEnd; pp_entity++ )
        {
            ( *pp_entity )->handleNotification( *p_notify );
        }
    }
    _queueNotifications.clear();
}

void EntityManager::deregisterUpdate( BaseEntity* p_entity )
{
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

void EntityManager::addToScene( BaseEntity* p_entity, osg::Group *p_scenegr )
{
    assert( p_entity && p_entity->_p_transformNode.valid() && "adding to scene requires a transformation node in entity!" );
    osg::Group *p_grp = p_scenegr ? p_scenegr : static_cast< osg::Group* >( Application::get()->getViewer()->getTopMostSceneData() );
    p_grp->addChild( p_entity->_p_transformNode.get() );
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

    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for(; pp_entity != pp_entityEnd; pp_entity++ )
        if ( ( *pp_entity ) == p_entity )
            break;

    if ( pp_entity != pp_entityEnd )
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

            delete ( *pp_entity );
        }

        _entityPool.erase( pp_entity );
    } 
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "EntityManager::removeFromEntityPool: request for entity removal which does not exist!" << endl;
    }
}

void EntityManager::setupEntities( vector< BaseEntity* >& entities )
{
    osg::Timer      timer;
    osg::Timer_t    curTick         = 0;
    osg::Timer_t    startTick       = 0;
    float           time4Init       = 0;
    float           time4PostInit   = 0;

    log << Log::LogLevel( Log::L_INFO ) << "starting entity setup ..." << endl;

    vector< BaseEntity* >::iterator pp_beg = entities.begin(), pp_end = entities.end();
    BaseEntity* p_entity = NULL;
    log << Log::LogLevel( Log::L_INFO ) << "initializing entities ..." << endl;
    // setup entities
    {
        // set internal state
        _internalState = InitializingEntities;

        startTick  = timer.tick();
        // consider persistence and avoid re-initialization of persistent entities
        for( ; pp_beg != pp_end; pp_beg++ )
        {
            p_entity = *pp_beg;
            log << Log::LogLevel( Log::L_DEBUG ) << "# " << p_entity->getInstanceName() + "[ " + p_entity->getTypeName() + " ]" << endl;

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

    pp_beg = entities.begin(); pp_end = entities.end();
    log << Log::LogLevel( Log::L_INFO ) << "post-initializing entities ..." << endl;
    {
        // set internal state
        _internalState = PostInitializingEntities;

        startTick  = timer.tick();
        // consider persistence and avoid re-initialization of persistent entities
        for( ; pp_beg != pp_end; pp_beg++ )
        {
            p_entity = *pp_beg;
            log << Log::LogLevel( Log::L_DEBUG ) << "# " << p_entity->getInstanceName() + "[ " + p_entity->getTypeName() + " ]" << endl;

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
        vector< BaseEntity* >::iterator pp_addtopoolentity = _queueAddToPoolEntities.begin(), pp_addtopoolentitiyEnd = _queueAddToPoolEntities.end();
        for ( ; pp_addtopoolentity != pp_addtopoolentitiyEnd; pp_addtopoolentity++ )
            _entityPool.push_back( *pp_addtopoolentity );

        _queueAddToPoolEntities.clear();

        // set internal state
        _internalState = None;
    }

    log << Log::LogLevel( Log::L_INFO ) << "--------------------------------------------" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "needed time for initialization: " << time4Init << " seconds" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "needed time for post-initialization: " << time4PostInit << " seconds" <<  endl;
    log << Log::LogLevel( Log::L_INFO ) << "total time for setting up: " << time4Init + time4PostInit << " seconds" <<  endl;
    log << Log::LogLevel( Log::L_INFO ) << "--------------------------------------------" << endl;

}

void EntityManager::updateEntities( float deltaTime  )
{
    vector< BaseEntity* >::iterator pp_entity = _updateEntities.begin(), pp_entityEnd = _updateEntities.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        BaseEntity* p_ent = ( *pp_entity );
        p_ent->updateEntity( deltaTime );

        // check heap if enabled ( used for detecting heap corruptions )
        CTD_CHECK_HEAP();

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
    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        // delete only if entity is not persistent
        if ( !( ( *pp_entity )->isPersistent() ) )
        {
            _queueDeletedEntities.push_back( *pp_entity );
        }
    }

    // update entity lists in order to let the deletions take place immediately
    updateEntityLists();

    // reset update list, re-add persistent entities which are in current update list
    vector< BaseEntity* > newupdatelist;
    vector< BaseEntity* >::iterator pp_uentity = _entityPool.begin(), pp_uentityEnd = _entityPool.end();
    for( ; pp_uentity != pp_uentityEnd; pp_uentity++ )
    {
        if ( ( *pp_uentity )->isPersistent() )
        {            
            vector< BaseEntity* >::iterator pp_pentity = _updateEntities.begin(), pp_pentityEnd = _updateEntities.end();
            for( ; pp_pentity != pp_pentityEnd; pp_pentity++ )
                if ( *pp_uentity == *pp_pentity )
                    break;
            if ( pp_pentity != pp_pentityEnd )
                newupdatelist.push_back( *pp_uentity );
        }
    }
    _updateEntities.clear();
    _updateEntities = newupdatelist;

    // this flag is used in main update entity loop, used for allowing entity deletions during update phase
    _updateEntityListChanged = true;
}

void EntityManager::getPersistentEntities( std::vector< BaseEntity* >& entities )
{
    vector< BaseEntity* >::iterator pp_entity = _entityPool.begin(), pp_entityEnd = _entityPool.end();
    for( ; pp_entity != pp_entityEnd; pp_entity++ )
    {
        if ( ( *pp_entity )->isPersistent() )
            entities.push_back( *pp_entity );
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
