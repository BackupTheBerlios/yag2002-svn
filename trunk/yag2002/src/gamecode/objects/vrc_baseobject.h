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
 # base of all kinds of pickable objects
 #
 #   date of creation:  10/31/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_BASEOBJECT_H_
#define _VRC_BASEOBJECT_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>

namespace vrc
{

//! Unique object IDs
enum ObjectIDs
{
    VRC_OBJECT_ID_WOOD  = 100
};

class EnPlayer;
class EnCamera;
class ObjectNetworking;
class ObjectInputHandler;

//! Base of pickable objects
class BaseObject : public yaf3d::BaseEntity
{
    public:

        //! Create the object with a unique object ID and entity type ( ID is one of ObjectIDs enums ).
                                                    BaseObject( unsigned int ID, const std::string& type );

        //! Destroy object
        virtual                                     ~BaseObject();

        //! Get the object ID
        unsigned int                                getObjectID() const;

        //! This method is used by networking on creation of the entity on clients.
        void                                        setNetworking( ObjectNetworking* p_networking );

    protected:

        //! Called when the object is used by user, e.g. by picking
        virtual void                                onObjectUse() = 0;

        //! Initializing function
        void                                        initialize();

        //! Post-initializing function
        void                                        postInitialize();

        //! Setup the object mesh
        void                                        setupMesh();

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Handle system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Check if the object is in camera focus
        void                                        checkCameraFocus();

        //! Check object distance
        bool                                        checkObjectDistance();

        // entity attributes
        // -----------

        //! Object's position
        osg::Vec3f                                  _position;

        //! Object's rotation
        osg::Vec3f                                  _rotation;

        //! Mesh file name
        std::string                                 _meshFile;

        //! Enable/disable shadow
        bool                                        _shadowEnable;

        //! Maximal allowed camera distance to object for highlightng
        float                                       _maxHeighlightDistance;

        //! Maximal allowed camera distance to object when picking
        float                                       _maxPickDistance;
        // -----------

        //! Unique object ID
        unsigned int                                _objectID;

        //! Is the object enabled?
        bool                                        _enable;

        //! Period of time for cyclic picking check
        float                                       _checkPickingPeriod;

        //! Period of time for cyclic object distance sorting
        float                                       _sortDistancePeriod;

        //! Square of maximal distance to camera for highlighting the object
        float                                       _maxHeighlightDistance2;

        //! Square of maximal distance to camera for picking the object
        float                                       _maxPickDistance2;

        //! Ray from object to camera
        osg::Vec3f                                  _ray;

        //! Current camera position updated periodically, but not every frame!
        osg::Vec3f                                  _currCamPosition;

        //! True if the object is highlighted
        bool                                        _highlight;

        //! Mesh animation time when object can be picked up
        float                                       _animTime;

        //! Local player entity
        EnPlayer*                                   _p_player;

        //! Player's camera entity
        EnCamera*                                   _p_playercamera;

        //! All available objects
        static std::vector< BaseObject* >           _objects;

        //! Object networking
        ObjectNetworking*                           _p_networking;

        //! Input handler for object used for picking
        class ObjectInputHandler : public vrc::gameutils::GenericInputHandler< ObjectInputHandler >
        {
            public:

                                                    ObjectInputHandler();

                virtual                             ~ObjectInputHandler() {}

                //! Handle input events.
                bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

                //! Set the current highlighted object
                void                                setHighlightedObject( BaseObject* p_object )
                                                    {
                                                        _p_highlightedObject = p_object;
                                                    }

                //! Get the current highlighted object
                 BaseObject*                        getHighlightedObject()
                                                    {
                                                        return _p_highlightedObject;
                                                    }

                //! Enable / disable input handling
                void                                enable( bool enable )
                                                    {
                                                        _enable = enable;
                                                    }

            protected:

                //! Current pickable object
                BaseObject*                         _p_highlightedObject;

                //! Enable / disable flag for handler
                bool                                _enable;

                //! Key code for picking
                unsigned int                        _keyCodePick;
        };

        //! Object input handler
        static ObjectInputHandler*                  _p_inputHandler;
};


//! Helper class for registration of object ID and Type.
/**
    Every object type must register its ID!
*/
class ObjectRegistry
{
    public:

        //! ID is one of ObjectIDs enums and entitytype is a unique entity type
                                                    ObjectRegistry( unsigned int ID, const std::string& entitytype )
                                                    {
                                                        ++_refCnt;

                                                        if ( !_p_objectTypes )
                                                            _p_objectTypes = new std::map< unsigned int, std::string >;

                                                        ( *_p_objectTypes )[ ID ] = entitytype;
                                                    }

        //! Destroy the object
        virtual                                     ~ObjectRegistry()
                                                    {
                                                        --_refCnt;
                                                        if ( !_refCnt )
                                                        {
                                                            delete _p_objectTypes;
                                                            _p_objectTypes = NULL;
                                                        }
                                                    }

        //! Register an object ID/Type
        static void                                 registerEntityType( unsigned int ID, const std::string& entitytype );

        //! Given an object ID return its entity type.
        static std::string                          getEntityType( unsigned int ID );

    protected:

        //! Ref count
        static unsigned int                             _refCnt;

        //! Object ID/Type lookup
        static std::map< unsigned int, std::string >*   _p_objectTypes;
};

//! Convenient macro for registring an object type
#define VRC_REGISTER_OBJECT( id, type )     static std::auto_ptr< ObjectRegistry > ObjectRegistry_impl_auto( new ObjectRegistry( ( id ), ( type ) ) );

} // namespace vrc

#endif // _VRC_BASEOBJECT_H_
