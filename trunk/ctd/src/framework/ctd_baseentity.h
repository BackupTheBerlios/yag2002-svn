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
 # base class for all entities
 #
 #   date of creation:  12/01/2003
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


#ifndef _CTD_BASEENTITY_H_
#define _CTD_BASEENTITY_H_

#include "ctd_attributemanager.h"

namespace CTD 
{

class BaseEntity;
class Application;
class BaseEntityFactory;

//! Base class for all kinds of entity notifications.
class EntityNotification
{
    public:
                                                    
                                                    EntityNotification( unsigned int id, BaseEntity* p_sender = NULL ) :
                                                     _id( id ),
                                                     _p_sender( p_sender )
                                                    {}

        virtual                                     ~EntityNotification() {}

        //! Return the notification id
        unsigned int                                getId()
                                                    {
                                                        return _id;
                                                    }

        BaseEntity*                                 getSender()
                                                    {
                                                        return _p_sender;
                                                    }

    protected:

        unsigned int                                _id;

        BaseEntity*                                 _p_sender;
};
//! Some standard notification ids
#define     CTD_NOTIFY_LOADING_LEVEL                0xF0000010  // sent when we load a level
#define     CTD_NOTIFY_DELETING_ENTITIES            0xF0000020  // sent at begin of entity deletions on level loading
#define     CTD_NOTIFY_BUILDING_PHYSICSWORLD        0xF0000021  // sent at begin of building physics world on level loading (see entity manager)
#define     CTD_NOTIFY_DELETING_PHYSICSWORLD        0xF0000022  // sent at begin of destruction of physics world on level loading (see entity manager)
#define     CTD_NOTIFY_MENU_ENTER                   0xF0000030  // sent when entering menu system
#define     CTD_NOTIFY_MENU_LEAVE                   0xF0000031  // sent when leaving menu system


//! Base of all game entities
class BaseEntity
{

    public:

                                                    BaseEntity() : _autoDelete(true), _active(true), _p_transformNode(0) {} 

        virtual                                     ~BaseEntity();

        /**
        * Get entity's type name.
        * \return                                   Type name
        */
        const std::string&                          getTypeName();

        /**
        * Set instance name. Instance name is used to find and attach entities to eachother.
        * If an entity is not attachable it does not need to set its instance name.
        * \param  name                              Instance name
        */
        void                                        setInstanceName( const std::string& name );

        /**
        * Get instance name. Instance name is used to find and attach entities to eachother.
        * \return                                   Instance name
        */
        const std::string&                          getInstanceName();

        /**
        * Get entity's attribute manager
        * \return                                   Attribute manager
        */
        AttributeManager&                           getAttributeManager();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        virtual void                                initialize() {}

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        virtual void                                postInitialize() {}

        /**
        * Update entity
        * \param fDeltaTime                         Time passed since last update
        */
        virtual void                                updateEntity( float deltaTime ) {}

        /**
        * Handle notifications. In order to get notifications the entity must register via EntityManager's registerNotification method.
        * \param notify                             The notification struct. It may be useful to cast it to appropriate type for 
        *                                           game-codespecific structs. 
        */
        virtual void                                handleNotification( EntityNotification& notify ) {}

        /**
        * Returns true if the entity is active.
        */
        bool                                        isActive() { return _active; }

        /**
        * Activate / deactivate entity. If an enitiy is deactivated then it is not updated in every step of game loop.
        */
        void                                        activate( bool active ) { _active = active; }

        /**
        * Set auto deletion flag.
        * \param autoDel                            Set to false when you are going to delete the entity manually.
        */
        void                                        setAutoDelete( bool autoDel );

        /**
        * Get auto delete flag.
        * \return                                   Auto delete flag
        */
        bool                                        getAutoDelete();

        /**
        * Override and return false if the entitiy does not need transformation.
        * This method is called during entity creation e.g. by the level loader.
        */
        virtual bool                                needTransformation() { return true; }

        /**
        * Set transformation node. An application developer does not need this method in normal case.
        * It is used by level loader.
        */
        void                                        setTransformationNode( osg::PositionAttitudeTransform* p_trans );

        /**
        * Get transformation node.
        */
        osg::PositionAttitudeTransform*             getTransformationNode();

        /** 
        * Set position of the transform node
        */
        void                                        setPosition( const osg::Vec3d& pos );

        /** 
        * Get position of the transform node
        */
        const osg::Vec3d&                           getPosition();

        /** 
        * Set rotation of the transform node
        */
        void                                        setRotation( const osg::Quat& quats );

        /**
        * Get rotation of the transform node
        */
        const osg::Quat&                            getRotation();

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

    private:

        /** 
        * Avoid usage of copy constructor
        */
                                                    BaseEntity( BaseEntity& );

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

    protected:

        /**
        * Add a node to our transformation node, assume that we use a transform node
        */
        void                                        addToTransformationNode( osg::Node* p_node );

        /**
        * Remove a node from our transformation node, assume that we use a transform node
        * For example it can be used to remove dynamically attached nodes such as rucksack, etc..
        */
        void                                        removeFromTransformationNode( osg::Node* p_node );

        //! Entity attribute manager
        AttributeManager                            _attributeManager;

        //! Deletion managed by Application or by game code?        
        bool                                        _autoDelete;

        //! Is entity active?
        bool                                        _active;

        //! Transformation node used if it is desired for an entity
        osg::PositionAttitudeTransform*             _p_transformNode;

    friend class Application;
    friend class EntityManager;
    friend class BaseEntityFactory;
};

#include "ctd_baseentity.inl"

}; // namespace CTD

#endif //_CTD_BASEENTITY_H_