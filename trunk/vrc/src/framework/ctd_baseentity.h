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

class Application;
class BaseEntityFactory;

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
        virtual void                                initialize() {};

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        virtual void                                postInitialize() {};

        /**
        * Update entity
        * \param fDeltaTime                         Time passed since last update
        */
        virtual void                                updateEntity( float deltaTime ) {};

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
        * This method is called during entity creation by the level loader.
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
