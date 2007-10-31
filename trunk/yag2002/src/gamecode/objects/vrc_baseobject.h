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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_BASEOBJECT_H_
#define _VRC_BASEOBJECT_H_

#include <vrc_main.h>

namespace vrc
{

class EnPlayer;
class EnCamera;

//! Base of pickable objects
class BaseObject : public yaf3d::BaseEntity
{
    public:

                                                    BaseObject();

        virtual                                     ~BaseObject();

    protected:

        //! This method is called when the object is hit and can be picked up
        virtual void                                onHitObject() = 0;

        //! Initializing function
        void                                        initialize();

        //! Post-initializing function
        void                                        postInitialize();

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

        //! Maximal allowed camera distance to object when picking
        float                                       _maxViewDistance;

        // -----------

        //! Is the object enabled?
        bool                                        _enable;

        //! Period of time for cyclic picking check
        float                                       _checkPickingPeriod;

        //! Period of time for cyclic object distance sorting
        float                                       _sortDistancePeriod;

        //! Square of maximal distance to camera allowed in order to get picked
        float                                       _maxViewDistance2;

        //! Ray from object to camera
        osg::Vec3f                                  _ray;

        //! Current camera position updated periodically, but not every frame!
        osg::Vec3f                                  _currCamPosition;

        //! True if the object can be picked up
        bool                                        _hit;

        //! Mesh animation time when object can be picked up
        float                                       _animTime;

        //! Local player entity
        EnPlayer*                                   _p_player;

        //! Player's camera entity
        EnCamera*                                   _p_playercamera;

        //! All available objects
        static std::vector< BaseObject* >           _objects;
};

} // namespace vrc

#endif // _VRC_BASEOBJECT_H_
