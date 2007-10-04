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
 # base class for all entities
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #
 #  02/17/2005 boto       creation of BaseEntity
 #
 #  02/26/2005 boto       added clone method
 #
 ################################################################*/


#ifndef _BASEENTITY_H_
#define _BASEENTITY_H_

#include "attributemanager.h"

namespace yaf3d
{

class BaseEntity;
class Application;
class BaseEntityFactory;

//! Base class for all kinds of entity notifications.
class EntityNotification
{
    public:

                                                    EntityNotification( unsigned int id ) :
                                                     _id( id )
                                                    {}

        virtual                                     ~EntityNotification() {}

        //! Return the notification id
        const unsigned int                          getId() const
                                                    {
                                                        return _id;
                                                    }

    protected:

        unsigned int                                _id;
};
//! Some standard notification ids
#define     YAF3D_NOTIFY_LOADING_LEVEL                0xF0000010  // sent when we load a level
#define     YAF3D_NOTIFY_UNLOAD_LEVEL                 0xF0000015  // sent when we unload a level
#define     YAF3D_NOTIFY_DELETING_ENTITIES            0xF0000020  // sent at begin of entity deletions on level loading
#define     YAF3D_NOTIFY_BUILDING_PHYSICSWORLD        0xF0000021  // sent at begin of building physics world on level loading (see entity manager)
#define     YAF3D_NOTIFY_DELETING_PHYSICSWORLD        0xF0000022  // sent at begin of destruction of physics world on level loading (see entity manager)
#define     YAF3D_NOTIFY_NEW_LEVEL_INITIALIZED        0xF0000023  // sent when a new level has been loaded and initialized, register your entities for updates and other things in this phase
#define     YAF3D_NOTIFY_MENU_ENTER                   0xF0000030  // sent when entering menu system
#define     YAF3D_NOTIFY_MENU_LEAVE                   0xF0000031  // sent when leaving menu system
#define     YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED     0xF0000040  // sent when an entity attribute has been changed ( e.g. by an level editor )
#define     YAF3D_NOTIFY_ENTITY_TRANSNODE_CHANGED     0xF0000050  // sent when entity's transformation node had been manipulated ( e.g. re-arranged in scenegraph ), this is sent out in level manager on unloading a level
#define     YAF3D_NOTIFY_SHUTDOWN                     0xF00000FF  // sent when shutting down the system


//! Base of all game entities
class BaseEntity
{

    public:

                                                    BaseEntity();

        virtual                                     ~BaseEntity();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        virtual void                                initialize() {}

        /**
        * Post-initializing function, this is called after all plugins' entities are initialized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        virtual void                                postInitialize() {}

        /**
        * Set instance name. Instance name is used to find and attach entities to eachother.
        * If an entity is not attachable it does not need to set its instance name.
        * \param  name                              Instance name
        */
        void                                        setInstanceName( const std::string& name );

        /**
        * Get entity's type name.
        * \return                                   Type name
        */
        inline const std::string&                   getTypeName();

        /**
        * Get instance name. Instance name is used to find and attach entities to eachother.
        * \return                                   Instance name
        */
        inline const std::string&                   getInstanceName();

        /**
        * Get entity's attribute manager
        * \return                                   Attribute manager
        */
        inline AttributeManager&                    getAttributeManager();

        /**
        * Get transformation node.
        */
        osg::PositionAttitudeTransform*             getTransformationNode();

        /**
        * Set position of the transform node
        */
        inline void                                 setPosition( const osg::Vec3d& pos );

        /**
        * Get position of the transform node
        */
        inline const osg::Vec3d&                    getPosition();

        /**
        * Set rotation of the transform node
        */
        inline void                                 setRotation( const osg::Quat& quats );

        /**
        * Get rotation of the transform node
        */
        inline const osg::Quat&                     getRotation();

        /**
        * Clone this entity and name it instanceName.
        * Cloning an entity means that also the accociated scenegraph node is cloned.
        * If p_scenegroup is not NULL then the cloned scene node will be added to this group.
        * If p_scenegroup is NULL then the cloned scene node will be added to clonee's parent group thus getting a sibling.
        */
        virtual BaseEntity*                         clone( const std::string& instanceName, osg::Group* p_scenegroup = NULL );

        /**
        * A convenient way to clone and initialize / post-initialize an entity.
        */
        virtual BaseEntity*                         cloneAndInitialize( const std::string& instanceName, osg::Group* p_scenegroup = NULL );

        /**
        * Override and return false if the entitiy does not need transformation.
        * This method is called during entity creation e.g. by the level loader.
        */
        virtual const bool                          isTransformable() const { return true; }

        /**
        * Override and return true if your entity must resist loading of new levels.
        * A persistent entity keeps its update and notification registration and is not deleted
        * by EntityManager when a new level is loaded.
        */
        virtual const bool                          isPersistent() const { return false; }

    protected:

        /**
        * Update entity
        * \param fDeltaTime                         Time passed since last update
        */
        virtual void                                updateEntity( float /*deltaTime*/ ) {}

        /**
        * Handle notifications. In order to get notifications the entity must register via EntityManager's registerNotification method.
        * \param notification                       The notification struct. It may be useful to cast it to appropriate type for
        *                                           game-codespecific structs.
        */
        virtual void                                handleNotification( const EntityNotification& /*notification*/ ) {}

        /**
        * Set transformation node. An application developer does not need this method in normal case.
        * It is used by level loader.
        */
        void                                        setTransformationNode( osg::PositionAttitudeTransform* p_trans );

        /**
        * Add a node to our transformation node, assume that we use a transform node
        */
        inline void                                 addToTransformationNode( osg::Node* p_node );

        /**
        * Remove a node from our transformation node, assume that we use a transform node.
        * For example it can be used to remove dynamically attached nodes such as rucksack, etc.
        */
        inline void                                 removeFromTransformationNode( osg::Node* p_node );

        /**
        * This method is used by EntityManager to avoid a reinitialization of and persistent entity on every level loading.
        * \param  init                              Initialization flag
        */
        inline void                                 setInitialized( bool init );

        /**
        * Returns the information wether the entity is already initialized ( used by EntityManager ).
        * \return                                   Initialization flag
        */
        inline bool                                 isInitialized();

        //! Entity attribute manager
        AttributeManager*                           _p_attributeManager;

        //! Transformation node used if it is desired for an entity
        osg::ref_ptr< osg::PositionAttitudeTransform > _p_transformNode;

    private:

        /**
        * Avoid usage of copy constructor
        */
                                                    BaseEntity( const BaseEntity& );

        /**
        * Avoid usage of assignment operator
        */
        BaseEntity&                                 operator = ( BaseEntity& );

        /**
        * Entity type name
        */
        std::string                                 _typeName;

        /**
        * Entity instance name
        */
        std::string                                 _instanceName;

        /**
        * Initialization flag used by EntityManger.
        */
        bool                                        _initialized;

    friend class Application;
    friend class EntityManager;
    friend class BaseEntityFactory;
};

#include "baseentity.inl"

} // namespace yaf3d

#endif //_BASEENTITY_H_
