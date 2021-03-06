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

#ifndef __ENTITYMANAGER_H_
#define __ENTITYMANAGER_H_

#include <baseentity.h>
#include <singleton.h>

//! Helper macro for implementing a new entity type factory
#define YAF3D_IMPL_ENTITYFACTORY( factory )   static std::auto_ptr< factory > factory##_impl_auto( new factory );

namespace yaf3d
{

//! Entity manager
/**
* This manager provides dynamic creation and deletion of game entities, searching for entities by type and instance name,
*  entity updating in game loop, and more.
*/
class EntityManager : public Singleton< EntityManager >
{
    public:

        //! Register an entity in order to get it updated in every frame or deregister an entity if "reg" is false.
        /*! Usually all active game objects should be registered. 
            Returns false if the given entity is already registered (or it was not previousely registered in case of deregistering).
        */
        bool                                        registerUpdate( BaseEntity* p_entity, bool reg = true );

        //! Returns true if the entity is registered for getting updates.
        bool                                        isRegisteredUpdate( const BaseEntity* p_entity );

        //! Register or deregister an entity for getting notifications.
        /*! Returns false if the given entity is already registered (or it was not previousely registered in case of deregistering).
        */
        bool                                        registerNotification( BaseEntity* p_entity, bool reg = true );

        //! Returns true if the entity is registered for getting notifications.
        bool                                        isRegisteredNotification( const BaseEntity* p_entity );

        //! Register / deregister an entity factory. If reg is true then the entity is registered, otherwise it is deregistered.
        /*! Every type must be unique.
            Returns false if the given factory is already registered.
            Use deregistration if you wish to completely remove an entity factory. Note that deregistering does not delete the registry object!
            Returns false if the given factory was not registered.
        */
        bool                                        registerFactory( BaseEntityFactory* p_entityName, bool reg = true );

        //! Return an entity factory object given entity's type name. Use it as creator instance of desired entity factory.
        /*! Returns NULL if entity type is not registered.
        */
        BaseEntityFactory*                          getEntityFactory( const std::string& type );

        //! Retrieve all existing entity factories
        void                                        getAllEntityFactories( std::vector< BaseEntityFactory* >& factories );

        //! Add the given entity to scene.
        /* \param p_entity                          Entity to add to scene
         * \param p_scenegrp                        Scene group into which the entity is attached. If it's NULL then the root group is used.
        */
        void                                        addToScene( BaseEntity* p_entity, osg::Group *p_scenegrp = NULL );

        //! Remove the given entity from the scene.
        /* \param p_entity                          Entity to remove from the scene
        */
        void                                        removeFromScene( BaseEntity* p_entity );

        //! Create a new entity given its type and optional instance name.
        /*! If addToPool is true then the entity will be added to entity pool. Set this to false if you are
        *   going to use the entity in a private fashion, so the entity will not be searchable and the destruction will not be managed by EntityManager.
        *   Returns NULL if entity type does not exist.
        */
        BaseEntity*                                 createEntity( const std::string& type, const std::string& instanceName = "", bool addToPool = true );

        //! Retrieve all existing entities
        void                                        getAllEntities( std::vector< BaseEntity* >& entities );

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
        * The notification will be sent out at begin of next game loop update.
        */
        void                                        sendNotification( const EntityNotification& notification );

        //! Send a notification to the given entity.
        /*! This notification is sent out only to the given entity. This method immediately sends out the notification to given entity regardless 
        *   of its registration for getting notifications. This mechanism is used by tools such as level editor, command console, etc. for notifying an
        *   entity about its attribute modifications from outside.
        */
        void                                        sendNotification( const EntityNotification& notification, BaseEntity* p_entity );

    protected:

                                                    EntityManager();

        virtual                                     ~EntityManager();

        //! Shutdown entity manager
        void                                        shutdown();

        //! Sets up given entities considering persistence etc.
        void                                        setupEntities( std::vector< BaseEntity* >& entities );

        //! Update entity manager, call this in every game loop step
        void                                        update( float deltaTime );

    private:


        //! Update all registered active entities
        void                                        updateEntities( float deltaTime );

        //! Remove entity's registration for updating. 
        void                                        deregisterUpdate( BaseEntity* p_entity );

        //! Delete all entities
        void                                        deleteAllEntities();

        //! Add given entity into pool ( used in level mananger ). the pool contains all entities
        void                                        addToEntityPool( BaseEntity* p_entity );

        //! Remove given entity from pool. If del is true then the entity is also deleted.
        void                                        removeFromEntityPool( BaseEntity* p_entity, bool del = true );

        //! Update internal maintained entity lists and queues.
        void                                        updateEntityLists();

        //! Flush the notification queue
        void                                        flushNotificationQueue();

        //! Fills the given list with persistent entities. This method is used by LevelManager.
        void                                        getPersistentEntities( std::vector< BaseEntity* >& entities );

        //! List of all registered update entities
        std::vector< BaseEntity* >                  _updateEntities;

        //! List of entities to be deleted on next update
        std::vector< BaseEntity* >                  _queueDeletedEntities;

        //! List of entities to be added / removed to / from update list on next update
        std::vector< std::pair< BaseEntity*, bool > > _queueUpdateEntities;

        //! This flag shows that the list has been modified
        bool                                        _updateEntityListChanged;

        //! List of all entities which are going to be initialized after level loading
        std::vector< BaseEntity* >                  _entityPool;

        //! List of all entities which registered for getting notification
        std::vector< BaseEntity* >                  _entityNotification;

        //! Notification queue which is flushed at begin of every update phase
        std::vector< EntityNotification >           _queueNotifications;

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
            PostInitializingEntities            
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

                                                    BaseEntityFactory( const std::string& entityTypeName, unsigned int ntype );

        virtual                                     ~BaseEntityFactory();

        virtual BaseEntity*                         createEntity() = 0;
       
        //! Get the entity type. Entity types must be unique and are described by a string.
        inline const std::string&                   getType() const;

        //! Get creation policy. It can be a bitwise-or of BaseEntityFactory::Standalone, BaseEntityFactory::Server, and BaseEntityFactory::Client.
        inline unsigned int                         getCreationPolicy() const;

        //! Comparision operator for entity factories
        inline bool                                 operator == ( const BaseEntityFactory& factory ) const;

        //! Entity's creation policy. This is used by level loader in order to decide whether to create an entity described by a level file
        //!  depending on the game mode ( server, client, or standalone ).
        enum CreationPolicy
        {
            Standalone = 0x1,
            Server     = 0x2,
            Client     = 0x4
        };

    protected:

        inline void                                 setEntityType( BaseEntity* p_entity ) const;

        const std::string                           _typeTypeName;

        unsigned int                                _creationPolicy;
};

} // namespace yaf3d

//! Use this convenient macro in your derived entity factories
#define Macro_CreateEntity( EntityClass )\
yaf3d::BaseEntity* createEntity() \
{ \
    EntityClass* p_entity =  new EntityClass; \
    setEntityType( p_entity );\
    return p_entity;\
}

// inlines
//--------
inline const std::string& yaf3d::BaseEntityFactory::getType() const
{
    return _typeTypeName;
}

inline bool yaf3d::BaseEntityFactory::operator == ( const yaf3d::BaseEntityFactory& factory ) const
{
    return ( _typeTypeName == factory.getType() );
}

inline unsigned int yaf3d::BaseEntityFactory::getCreationPolicy() const
{
    return _creationPolicy;
}

inline void yaf3d::BaseEntityFactory::setEntityType( yaf3d::BaseEntity* p_entity ) const
{ 
    p_entity->_typeName = _typeTypeName;
}

#endif // __ENTITYMANAGER_H_
