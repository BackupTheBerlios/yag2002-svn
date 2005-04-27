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
 # entity camera
 #  this entity can be used by other entities (e.g. player) for 
 #  controling the rendering view.
 #
 #   date of creation:  04/21/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CAMERA_H_
#define _CTD_CAMERA_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_CAMERA    "Camera"

class CameraFrameHandler;

//! This entity controls the camera
class EnCamera :  public BaseEntity
{
    public:
                                                    EnCamera();

        virtual                                     ~EnCamera();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Set camera translation
        inline void                                 setCameraTranslation( const osg::Vec3f& pos, const osg::Quat& rot );

        //! Set camera position
        inline void                                 setCameraPosition( const osg::Vec3f& pos );

        //! Set camera rotation
        inline void                                 setCameraRotation( const osg::Quat& rot );

        //! Rotate camera
        inline void                                 rotateCamera( const osg::Quat& rot );

        //! Add an offset to camera rotation.
        /**
        * Note: the methods setLocalPitchYaw and setLocalPitch override the results of this method and
        *       vice vera!
        */
        inline void                                 setCameraOffsetRotation( const osg::Quat& rotOffset );

        //! Add an offset to camera position
        inline void                                 setCameraOffsetPosition( const osg::Vec3f& posOffset );

        //! Set camera's pitch and yaw in degrees, used for looking around.
        /**
        * Note: the method setCameraOffsetRotation overrides the results of this method and
        *       vice vera!
        */
        inline void                                 setLocalPitchYaw( float pitch, float yaw );

        //! Get the current pitch and yaw
        inline void                                 getLocalPitchYaw( float& pitch, float& yaw );

        //! Set camera's pitch in degrees, used for looking around.
        /**
        * Note: the method setCameraOffsetRotation overrides the results of this method and
        *       vice vera!
        */
        inline void                                 setLocalPitch( float pitch );

        //! Set camera's yaw in degrees, used for looking around.
        /**
        * Note: the method setCameraOffsetRotation overrides the results of this method and
        *       vice vera!
        */
        inline void                                 setLocalYaw( float yaw );

    protected:

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
        osg::Vec3f                                  _backgroudColor;

    protected:

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
class CameraEntityFactory : public BaseEntityFactory
{
    public:
                                                    CameraEntityFactory() : BaseEntityFactory( ENTITY_NAME_CAMERA ) {}

        virtual                                     ~CameraEntityFactory() {}

        Macro_CreateEntity( EnCamera );
};


// inlines
inline void EnCamera::setCameraTranslation( const osg::Vec3f& pos, const osg::Quat& rot )
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

inline void EnCamera::setCameraRotation( const osg::Quat& rot )
{
    _curRotation = rot;
    _needUpdate = true;
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

inline void EnCamera::setCameraOffsetPosition( const osg::Vec3f& posOffset )
{
    _offsetMatrixPosition.makeTranslate( posOffset );
    _needUpdate = true;
}

inline void EnCamera::setLocalPitchYaw( float pitch, float yaw )
{
    _pitch = pitch;
    _yaw   = yaw;
    osg::Quat rot( 
                    osg::DegreesToRadians( pitch ), osg::Vec3f( 1, 0, 0 ),
                    osg::DegreesToRadians( 0.0f  ), osg::Vec3f( 0, 1, 0 ),
                    osg::DegreesToRadians( yaw   ), osg::Vec3f( 0, 0, 1 )
                 );

    _offsetMatrixRotation = osg::Matrixf( rot );
    _needUpdate = true;
}

inline void EnCamera::getLocalPitchYaw( float& pitch, float& yaw )
{
    pitch = _pitch;
    yaw   = _yaw;
}

inline void EnCamera::setLocalPitch( float pitch )
{
    _pitch = pitch;
    osg::Quat rot( 
                    osg::DegreesToRadians( pitch ), osg::Vec3f( 1, 0, 0 ),
                    osg::DegreesToRadians( 0.0f  ), osg::Vec3f( 0, 1, 0 ),
                    osg::DegreesToRadians( 0.0f  ), osg::Vec3f( 0, 0, 1 )
                 );

    _offsetMatrixRotation = osg::Matrixf( rot );
    _needUpdate = true;
}

inline void EnCamera::setLocalYaw( float yaw )
{
    _yaw   = yaw;
    osg::Quat rot( 
                    osg::DegreesToRadians( 0.0f  ), osg::Vec3f( 1, 0, 0 ),
                    osg::DegreesToRadians( 0.0f  ), osg::Vec3f( 0, 1, 0 ),
                    osg::DegreesToRadians( yaw   ), osg::Vec3f( 0, 0, 1 )
                 );

    _offsetMatrixRotation = osg::Matrixf( rot );
    _needUpdate = true;
}

}

#endif // _CTD_CAMERA_H_
