/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # game navigation related classes
 #
 #   date of creation:  02/01/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NAVIGATION_H_
#define _NAVIGATION_H_

#include <vrc_main.h>
#include <osgUtil/IntersectVisitor>


class GameInterface;

//! Callback class used for notifying about picking results
class CallbackNavigatorNotify
{
    public:
                                                        CallbackNavigatorNotify() {}

                                                        ~CallbackNavigatorNotify() {}

        //! This method is called when an entity was picked. If no entity was picked then 'p_entity' will be NULL.
        virtual void                                    onEntityPicked( yaf3d::BaseEntity* /*p_entity*/ ) {}

        //! This method is called when user uses left mouse btn when in arrow mode ( used for placing new entities ).
        virtual void                                    onArrowClick( const osg::Vec3f& /*pos*/ ) {}
};

//! Game navigation class running in game thread context.
class GameNavigator : public osgGA::GUIEventHandler, public yaf3d::Singleton< GameNavigator >
{
    public:

        //! Navigation modes
        enum NavigationMode
        {
            EntityPlace   = 0x01,
            EntityPick    = 0x02,
            ShowPickArrow = 0x04
        };

        //! Enable/disable navigation
        void                                            enable( bool en );

        //! Set navigation mode, one of NavigationMode enums.
        void                                            setMode( unsigned int mode );

        //! Get current navigation mode.
        unsigned int                                    getMode() const;

        //! Set movement speed.
        void                                            setSpeed( float speed );

        //! Get movement speed.
        float                                           getSpeed() const;

        //! There may be a better place to put this method :-/
        unsigned int                                    getFPS() const;

        //! Set camera position
        void                                            setCameraPosition( osg::Vec3f pos );

        //! Get current camera position
        const osg::Vec3f&                               getCameraPosition() const;

        //! Set camera's pitch and yaw (degree)
        void                                            setCameraPitchYaw( float pitch, float yaw );

        //! Get camera's pitch and yaw (degree)
        void                                            getCameraPitchYaw( float& pitch, float& yaw ) const;

        //! Set near and far distances
        void                                            setNearFarClip( float nearclip, float farclip );

        //! Get near and far distances
        void                                            getNearFarClip( float& nearclip, float& farclip ) const;

        //! Set the angle of focus of view (degree)
        void                                            setFOV( float fov );

        //! Get the angle of focus of view (degree)
        float                                           getFOV() const;

        //! Set camera background color.
        void                                            setBackgroundColor( const osg::Vec3f& color );

        //! Get camera background color.
        const osg::Vec3f&                               getBackgroundColor() const;

        //! Select entity. What with given entity happends depends on the current mode.
        //! If 'p_entity' is NULL then the highlight is removed from scene ( if any was active before ).
        void                                            selectEntity( yaf3d::BaseEntity* p_entity );

        //! Set notification callback
        void                                            setNotifyCallback( CallbackNavigatorNotify* p_cb );

    protected:

                                                        GameNavigator();

        virtual                                         ~GameNavigator();

        //! Initialize the navigator
        void                                            initialize();

        //! Shutdown the navigator
        void                                            shutdown();

        //! Update, called periodically in game thread context
        void                                            update( float deltatime );

        //! Input handler's callback
        bool                                            handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! Highlight given entity.
        void                                            highlightEntity( yaf3d::BaseEntity* p_entity );

        //! Picking entities. If p_entity is given then it is prefered by searching.
        yaf3d::BaseEntity*                              pick( unsigned short xpos, unsigned short ypos, yaf3d::BaseEntity* p_entity = NULL );

        //! Method checking for polygon hits
        bool                                            hit( unsigned short xpos, unsigned short ypos );

        //! Enable/disable navigation
        bool                                            _enable;

        //! Navigation mode
        unsigned int                                    _mode;

        //! Moving speed
        float                                           _moveSpeed;

        //! Rotation speed
        float                                           _rotationSpeed;

        //! Camera yaw
        float                                           _yaw;

        //! Camera pitch
        float                                           _pitch;

        //! Past time since last update
        float                                           _deltaTime;

        //! Current camera position
        osg::Vec3f                                      _position;

        //! Current camera rotation in randiant
        osg::Quat                                       _rotation;

        //! Near clip distances
        float                                           _nearClip;

        //! Far clip distance
        float                                           _farClip;

        //! Focus of view
        float                                           _fov;

        //! Background color
        osg::Vec3f                                      _backgroundColor;

        //! X coord of screen middle
        Uint16                                          _screenMiddleX;

        //! Y coord of screen middle
        Uint16                                          _screenMiddleY;

        //! Used for mesuring fps
        float                                           _fpsTimer;

        //! Used for mesuring fps
        unsigned int                                    _fpsCnt;

        //! Current fps
        unsigned int                                    _fps;

        //! Picking related stuff
        osg::ref_ptr< osg::PositionAttitudeTransform >  _marker;

        float                                           _iscreenWidth;

        float                                           _iscreenHeight;

        unsigned int                                    _pickClickCount;

        float                                           _lastX;

        unsigned short                                  _currX;

        float                                           _lastY;

        unsigned short                                  _currY;

        osg::ref_ptr< osg::Geode >                      _bboxGeode;

        osg::Geometry*                                  _p_linesGeom;

        osg::ref_ptr< osg::LineSegment >                _p_lineSegment;

        osg::Vec3f                                      _hitPosition;

        //! Possible inputs
        enum InputCode
        {
            NoCode    = 0x0000,
            Forward   = 0x0001,
            Backward  = 0x0002,
            Left      = 0x0004,
            Right     = 0x0008,
            Rotate    = 0x0010,
            Drag      = 0x0020,
            WheelUp   = 0x0040,
            WheelDown = 0x0080,
            Ctrl      = 0x0100,
            ObjFocus  = 0x0200
        };

        //! Current input code combination
        unsigned int                                    _inputCode;

        //! Picking callback
        CallbackNavigatorNotify*                        _p_cbNotify;

        yaf3d::BaseEntity*                              _p_selEntity;

//         yaf3d::BaseEntity*                             _p_placeEntity;

    friend class GameInterface;
    friend class yaf3d::Singleton< GameNavigator >;
};

#endif // _NAVIGATION_H_
