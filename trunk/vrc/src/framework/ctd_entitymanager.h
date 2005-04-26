/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
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
 #  such as updating and searching for entities by name etc.
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_ENTITYMANAGER_H_
#define _CTD_ENTITYMANAGER_H_

#include <ctd_baseentity.h>
#include <ctd_singleton.h>

namespace CTD
{

class Application;
class BaseEntityFactory;

//! Entity manager
/**
* This manager provides dynamic creation and deletion of game entities, searching for entities by type and instance name,
*  and entity updating in game loop.
*/
class EntityManager : public Singleton< EntityManager >
{
    public:

        //! Register an entity in order to get it updated in every frame or deregister an entity if "reg" is false.
        /*! Usually all active game objects should be registered. 
            Returns false if the given entity is already registered (or it was not previousely registered in case of deregistering).
        */
        bool                                        registerUpdate( BaseEntity* p_entity, bool reg = true );

        //! Register or deregister an entity for getting notifications.
        /*! Returns false if the given entity is already registered (or it was not previousely registered in case of deregistering).

        */
        bool                                        registerNotification( BaseEntity* p_entity, bool reg = true );

        //! Register / deregister an entity factory. If reg is true then the entity is registered, otherwise it is deregistered.
        /*! Every type must be unique.
            Returns false if the given factory is already registered.
            Use deregistration if you wish to completely remove an entity factory. Note that deregistering does not delete the registry object!
            Returns false if the given factory was not registered.
        */
        bool                                        registerFactory( BaseEntityFactory* p_entityName, bool reg = true );

        //! Add the given entity to scene.
        /* \param p_entity                          Entity to add to scene
         * \param p_scenegrp                        Scene group into which the entity is attached. If it's NULL then the root group is used.
        */
        void                                        addToScene( BaseEntity* p_entity, osg::Group *p_scenegrp = NULL );

        //! Return an entity factory object given entity's name. Use it as creator instance of desired entity factory.
        /*! Returns NULL if entity type is not registered.
        */
        BaseEntityFactory*                          getEntityFactory( const std::string& entityName );

        //! Create a new entity given its type and optional instance name.
        /*! If addToPool is true then the entity will be added to entity pool. Set this to false if you are
        *   going to use the entity in a private fashion, so the entity will not be searchable and the destruction will not be managed by EntityManager.
        *   Returns NULL if entity type does not exist.
        */
        BaseEntity*                                 createEntity( const std::string& type, const std::string& instanceName = "", bool addToPool = true );

        //! Delete given entitiy
        /*! \param  p_entity                        Entity to delete
        */
        void                                        deleteEntity( BaseEntity* p_entity );

        //! Find an entity of given type.
        /*! If instanceName is empty then the first enity matching type will be returned.
        *   Returns NULL if no entity found.
        */
        BaseEntity*                                 findEntity( const std::string& type, const std::string& instanceName = "" );

        //! Find an entity instance.
        /*! Returns NULL if no entity found.
        */
        BaseEntity*                                 findInstance( const std::string& instanceName );

        //! Send a notification to all notification-registered entities.
        /*! This mechanism allows to notify all entities about actions such as entering or leaving menu.
        */
        void                                        sendNotification( const EntityNotify& notify );

    protected:

                                                    EntityManager();

                                                    EntityManager( const EntityManager& );

        virtual                                     ~EntityManager();


        EntityManager&                              operator = ( const EntityManager& );

        //! Shutdown entity manager
        void                                        shutdown();

    private:

        //! Initialize all registered entities
        void                                        initializeEntities();

        //! Post-initialize all registered entities
        void                                        postInitializeEntities();

        //! Update entity manager, call this in every game loop step
        void                                        update( float deltaTime  );

        //! Update all registered active entities
        void                                        updateEntities( float deltaTime  );

        //! Remove entity's registration for updating. 
        void                                        deregisterUpdate( BaseEntity* p_entity );

        //! Delete all entities
        void                                        deleteAllEntities();

        //! Add given entity into pool ( used in level mananger ). the pool contains all entities
        void                                        addToEntityPool( BaseEntity* p_entity );

        //! Remove given entity from pool. If del is true then the entity is also deleted.
        void                                        removeFromEntityPool( BaseEntity* p_entity, bool del = true );

        //! List of all registered update entities
        std::vector< BaseEntity* >                  _updateEntities;

        //! List of entities to be deleted on next update
        std::vector< BaseEntity* >                  _queueDeletedEntities;

        //! List of entities to be added / removed to / from update list on next update
        std::vector< std::pair< BaseEntity*, bool > > _queueUpdateEntities;

        //! List of all entities which are going to be initialized after level loading
        std::vector< BaseEntity* >                  _entityPool;

        //! List of all entities which registered for getting notification
        std::vector< BaseEntity* >                  _entityNotification;

        //! List of entities to be added to pool gathered in initialization / post-initialization phases
        /**
        * This is used when adding to pool entity requests arise during initialization / post-initialization phases.
        * Such request is triggered e.g. by cloning an entity. In order to avoid the manipulation of entity pool list 
        * the actual action is delayed until to end of post-initialization phase.
        */
        std::vector< BaseEntity* >                  _queueAddToPoolEntities;

        //! List of all registered entity types
        std::vector< BaseEntityFactory* >           _entityFactories;

        //! Flag indicating the various internal states
        enum
        {
            None,
            InitializingEntities,
            UpdatingEntities,
            PostInitializingEntities,
            DeletingEntities
        }                                           _internalState;

    friend class Singleton< EntityManager >;
    friend class LevelManager;
    friend class Application;
};

//! Base class for all kinds of entity types used for entity type registry.
/*! Use entity type for creating entites of desired type.
*/
class BaseEntityFactory
{
    public:
                                                    BaseEntityFactory( const std::string& entityTypeName );

        virtual                                     ~BaseEntityFactory();

        virtual BaseEntity*                         createEntity() = 0;

        inline const std::string&                   getType();

        inline bool                                 operator == ( BaseEntityFactory& factory );

    protected:

        void                                        setEntityType( BaseEntity* p_entity ) 
                                                    { 
                                                        p_entity->_typeName = _typeTypeName;
                                                    }

        const std::string                           _typeTypeName;
};

//! Use this convenient macro in your derived entity factories
#define Macro_CreateEntity( EntityClass )\
BaseEntity* createEntity() \
{ \
    EntityClass* p_entity =  new EntityClass; \
    setEntityType( p_entity );\
    return p_entity;\
}


//! Helper macro for implementing a new entity type factory with automatic deletion
//  Use this if you don't intent to remove the entity type factory during the lifetime of the application
#define CTD_IMPL_ENTITYFACTORY_AUTO( factory )   std::auto_ptr< factory > factory##_impl_auto( new factory );

//! Helper macro for implementing a new entity type factory
#define CTD_IMPL_ENTITYFACTORY( factory )        factory* factory##_impl = new factory;

// inlines
//--------
inline const std::string& BaseEntityFactory::getType()
{
    return _typeTypeName;
}

inline bool BaseEntityFactory::operator == ( BaseEntityFactory& factory )
{
    return ( _typeTypeName == const_cast< std::string& >( factory.getType() ) );
}

}

#endif // _CTD_ENTITYMANAGER_H_
