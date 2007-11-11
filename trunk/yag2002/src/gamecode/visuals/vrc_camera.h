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
 # entity camera
 #  this entity can be used by other entities (e.g. player) for 
 #  controling the rendering view.
 #
 #   date of creation:  04/21/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CAMERA_H_
#define _VRC_CAMERA_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_CAMERA    "Camera"

class CameraFrameHandler;

//! This entity controls the camera
//! Note: take care that you don't have more than one enabled camera at same time!
class EnCamera :  public yaf3d::BaseEntity
{
    public:
                                                    EnCamera();

        virtual                                     ~EnCamera();

        //! Initializing function
        void                                        initialize();

        //! This entity does not need a transformation node.
        const bool                                  isTransformable() const { return false; }

        //! Set the persistence flag. 
        //! Note: this flag is checked by framework on destruction of a level.
        void                                        setPersistent( bool persistence ) { _isPersistent = persistence; }

        //! Set camera transformation
        inline void                                 setCameraTransformation( const osg::Vec3f& pos, const osg::Quat& rot );

        //! Set camera position
        inline void                                 setCameraPosition( const osg::Vec3f& pos );

        //! Get camera position
        inline const osg::Vec3f&                    getCameraPosition() const;

        //! Set camera rotation
        inline void                                 setCameraRotation( const osg::Quat& rot );

        //! Get camera rotation
        inline const osg::Quat&                     getCameraRotation() const;

        //! Rotate camera
        inline void                                 rotateCamera( const osg::Quat& rot );

        //! Add an offset to camera rotation.
        /**
        * Note: the methods setLocalPitchYaw and setLocalPitch override the results of this method and
        *       vice vera!
        */
        inline void                                 setCameraOffsetRotation( const osg::Quat& rotOffset );

        //! Get offset rotation
        inline const osg::Quat&                     getCameraOffsetRotation() const;

        //! Add an offset to camera position
        inline void                                 setCameraOffsetPosition( const osg::Vec3f& posOffset );

        //! Get offset position
        inline const osg::Vec3f&                    getCameraOffsetPosition() const;

        //! Set camera's pitch and yaw, used for looking around.
        /**
        * Note: the method setCameraOffsetRotation overrides the results of this method and
        *       vice vera!
        */
        inline void                                 setLocalPitchYaw( float pitch, float yaw );

        //! Get the current pitch and yaw
        inline void                                 getLocalPitchYaw( float& pitch, float& yaw ) const;

        //! Get local rotation ( pitch / yaw as quaternion that is )
        inline const osg::Quat                      getLocalRotation() const;

        //! Set camera's pitch, used for looking around.
        /**
        * Note: the method setCameraOffsetRotation overrides the results of this method and
        *       vice vera!
        */
        inline void                                 setLocalPitch( float pitch );

        //! Set camera's yaw, used for looking around.
        /**
        * Note: the method setCameraOffsetRotation overrides the results of this method and
        *       vice vera!
        */
        inline void                                 setLocalYaw( float yaw );

        //! Enable / disable this camera
        void                                        setEnable( bool enable );

        //! Update camera view, set forceupdate in order to ignore internal check for update need
        void                                        updateCameraView( bool forceupdate = false );

        //! This entity can be either persistent or not!
        const bool                                  isPersistent() const { return _isPersistent; }

    protected:

        //! This entity is persistent so we have to handle entity's update registration on every level loading and
        //  destruction ourselves.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup the camera
        void                                        setupCamera();

        //! Overridden, declared as protected and left as unimplemented as for this entity you must use the methods setCameraTransformation or setCameraPosition!
        void                                        setPosition( osg::Vec3f& pos );

        //! Overridden, declared as protected and left as unimplemented as for this entity you must use the methods setCameraTransformation or setCameraRotation!
        void                                        setRotation( osg::Quat& quats );

        // Entity parameters

        //! Initial camera position
        osg::Vec3f                                  _position;

        //! Initial camera rotation in degree
        osg::Vec3f                                  _rotation;

        //! Field of view in degree
        float                                       _fov;

        //! Near clip distance
        float                                       _nearClip;

        //! Far clip distance
        float                                       _farClip;

        //! Background color ( clear color )
        osg::Vec3f                                  _backgroundColor;

        bool                                        _isPersistent;

        //! Current camera position
        osg::Vec3f                                  _curPosition;

        //! Current camera rotation in randiant
        osg::Quat                                   _curRotation;

        //! Matrix used for offsetting rotation of camera
        osg::Matrixf                                _offsetMatrixRotation;

        //! Matrix used for offsetting position of camera
        osg::Matrixf                                _offsetMatrixPosition;

        //! Pitch for locally orienting the camera view
        float                                       _pitch;

        //! Yaw for locally orienting the camera view
        float                                       _yaw;

        //! Dirty flag
        bool                                        _needUpdate;

        //! Callback handler for setting view matrix in 'frame' callback phase
        CameraFrameHandler*                         _p_cameraHandler;

    friend class CameraFrameHandler;
};

//! Entity type definition used for type registry
class CameraEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    CameraEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_CAMERA, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~CameraEntityFactory() {}

        Macro_CreateEntity( EnCamera );
};


// inlines
inline void EnCamera::setCameraTransformation( const osg::Vec3f& pos, const osg::Quat& rot )
{
    _curPosition = pos;
    _curRotation = rot;
    _needUpdate = true;
}

inline void EnCamera::setCameraPosition( const osg::Vec3f& pos )
{
    _curPosition = pos;
    _needUpdate = true;
}

inline const osg::Vec3f& EnCamera::getCameraPosition() const
{
    return _curPosition;
}

inline void EnCamera::setCameraRotation( const osg::Quat& rot )
{
    _curRotation = rot;
    _needUpdate = true;
}

inline const osg::Quat& EnCamera::getCameraRotation() const
{
    return _curRotation;
}


inline void EnCamera::rotateCamera( const osg::Quat& rot )
{
    _curRotation = _curRotation * rot;
    _needUpdate = true;
}

inline void EnCamera::setCameraOffsetRotation( const osg::Quat& rotOffset )
{
    _offsetMatrixRotation.makeRotate( rotOffset );
    _needUpdate = true;
}

inline const osg::Quat& EnCamera::getCameraOffsetRotation() const
{
    static osg::Quat offset;
    _offsetMatrixRotation.get( offset );
    return offset;
}

inline void EnCamera::setCameraOffsetPosition( const osg::Vec3f& posOffset )
{
    _offsetMatrixPosition.makeTranslate( posOffset );
    _needUpdate = true;
}

inline const osg::Vec3f& EnCamera::getCameraOffsetPosition() const
{
    static osg::Vec3f offset;
    offset = _offsetMatrixPosition.getTrans();
    return offset;
}

inline void EnCamera::setLocalPitchYaw( float pitch, float yaw )
{
    _pitch = pitch;
    _yaw   = yaw;
    osg::Quat rot( 
                    pitch, osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                    0.0f,  osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                    yaw,   osg::Vec3f( 0.0f, 0.0f, 1.0f )
                 );

    _offsetMatrixRotation = osg::Matrixf( rot );
    _needUpdate = true;
}

inline const osg::Quat EnCamera::getLocalRotation() const
{
    osg::Quat rot( 
                    _pitch, osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                    0.0f,   osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                    _yaw,   osg::Vec3f( 0.0f, 0.0f, 1.0f )
                  );

    return rot;
}

inline void EnCamera::getLocalPitchYaw( float& pitch, float& yaw ) const
{
    pitch = _pitch;
    yaw   = _yaw;
}

inline void EnCamera::setLocalPitch( float pitch )
{
    _pitch = pitch;
    osg::Quat rot( 
                    pitch, osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                    0.0f,  osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                    0.0f,  osg::Vec3f( 0.0f, 0.0f, 1.0f )
                 );

    _offsetMatrixRotation = osg::Matrixf( rot );
    _needUpdate = true;
}

inline void EnCamera::setLocalYaw( float yaw )
{
    _yaw   = yaw;
    osg::Quat rot( 
                    0.0f, osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                    0.0f, osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                    yaw,  osg::Vec3f( 0.0f, 0.0f, 1.0f )
                 );

    _offsetMatrixRotation = osg::Matrixf( rot );
    _needUpdate = true;
}

}

#endif // _VRC_CAMERA_H_
