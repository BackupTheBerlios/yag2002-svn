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
 #  this entity can be attached and used by other cameras 
 #
 #   date of creation:  04/21/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CAMERA_H_
#define _CTD_CAMERA_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_CAMERA    "Camera"

//! This entity controls the camera
class EnCamera :  public BaseEntity
{
    public:
                                                    EnCamera();

        virtual                                     ~EnCamera();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        /**
        * Update entity
        * \param deltaTime                          Time passed since last update
        */
        void                                        updateEntity( float deltaTime );

        //! Set camera translation
        inline void                                 setCameraTranslation( const osg::Vec3f& trans );

        //! Translate camera
        inline void                                 translateCamera( const osg::Vec3f& trans );

        //! Set camera rotation (use euler angles)
        inline void                                 setCameraRotation( const osg::Vec3f& rot );

        //! Rotate camera (use euler angles)
        inline void                                 rotateCamera( const osg::Vec3f& rot );

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

        //! Producer's camera which is controled by this entity
        Producer::Camera*                           _p_cam;

        //! Camera matrix
        Producer::Matrix                            _camMatrix;

        //! Current camera position
        osg::Vec3f                                  _curPosition;

        //! Current camera rotation in randiant
        osg::Vec3f                                  _curRotation;

        //! Dirty flag
        bool                                        _needUpdate;
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
inline void EnCamera::setCameraTranslation( const osg::Vec3f& trans )
{
    _curPosition = trans;
    _camMatrix = _camMatrix.translate( _curPosition.x(), _curPosition.y(), _curPosition.z() ) *
                 _camMatrix.rotate( _curRotation.x(), 1, 0, 0 ) *
                 _camMatrix.rotate( _curRotation.y(), 0, 1, 0 ) *
                 _camMatrix.rotate( _curRotation.z(), 0, 0, 1 );

    _needUpdate = true;
}

inline void EnCamera::translateCamera( const osg::Vec3f& trans )
{
    _curPosition += trans;
    _camMatrix = _camMatrix.translate( _curPosition.x(), _curPosition.y(), _curPosition.z() ) *
                 _camMatrix.rotate( _curRotation.x(), 1, 0, 0 ) *
                 _camMatrix.rotate( _curRotation.y(), 0, 1, 0 ) *
                 _camMatrix.rotate( _curRotation.z(), 0, 0, 1 );

    _needUpdate = true;
}

inline void EnCamera::setCameraRotation( const osg::Vec3f& rot )
{
    _curRotation = rot;
    _camMatrix = _camMatrix.translate( _curPosition.x(), _curPosition.y(), _curPosition.z() ) *
                 _camMatrix.rotate( _curRotation.x(), 1, 0, 0 ) *
                 _camMatrix.rotate( _curRotation.y(), 0, 1, 0 ) *
                 _camMatrix.rotate( _curRotation.z(), 0, 0, 1 );

    _needUpdate = true;
}

inline void EnCamera::rotateCamera( const osg::Vec3f& rot )
{
    _curRotation += rot;
    _camMatrix = _camMatrix.translate( _curPosition.x(), _curPosition.y(), _curPosition.z() ) *
                 _camMatrix.rotate( _curRotation.x(), 1, 0, 0 ) *
                 _camMatrix.rotate( _curRotation.y(), 0, 1, 0 ) *
                 _camMatrix.rotate( _curRotation.z(), 0, 0, 1 );

    _needUpdate = true;
}

}

#endif // _CTD_CAMERA_H_
