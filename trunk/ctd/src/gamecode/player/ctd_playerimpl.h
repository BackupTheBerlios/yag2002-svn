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
 # base clas for player implementations
 #
 #  the actual player behaviour is described in one of its implemnets
 #  depending on game mode ( server, client, or standalone )
 #
 #
 #   date of creation:  05/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/28/2005 boto       creation of PlayerImpl
 #
 ################################################################*/

#ifndef _CTD_PLAYERIMPL_H_
#define _CTD_PLAYERIMPL_H_

#include <ctd_main.h>
#include "ctd_player.h"

class PlayerNetworking;

namespace CTD
{

class EnCamera;
class PlayerChatGui;
class EnPlayerSound;
class EnPlayerPhysics;
class EnPlayerAnimation;
class PlayerInputHandler;

// entity name of player's camera
#define PLAYER_CAMERA_ENTITIY_NAME      "playercam"

//! Player implementation base class
/** 
 * For a concrete implementation you have to derive from this class and override necessary methods.
 */
class BasePlayerImplementation
{
    public:

                                                    BasePlayerImplementation( EnPlayer* player );

        virtual                                     ~BasePlayerImplementation();


        //! Initialize
        virtual void                                initialize() = 0;

        //! Post-initialize
        virtual void                                postInitialize() = 0;

        //! Update
        virtual void                                update( float deltaTime ) = 0;

        //! Implementation's notification callback
        virtual void                                handleNotification( EntityNotification& notify ) {}

        //! Set player's position.
        inline void                                 setPlayerPosition( const osg::Vec3f& pos );

        //! Set player's rotation.
        inline void                                 setPlayerRotation( const osg::Quat& rot );

        //! Get player's position.
        inline const osg::Vec3f&                    getPlayerPosition();

        //! Get player's rotation.
        inline const osg::Quat&                     getPlayerRotation();

        //! Get player's move direction
        inline const osg::Vec3f&                    getPlayerMoveDirection();

        //! Add the given message to chat box
        void                                        addChatMessage( const std::string& msg, const std::string& author );

        //! Distribute the given message to all other clients
        void                                        distributeChatMessage( const std::string& msg );

        //! Return player's animation component.
        inline EnPlayerAnimation*                   getPlayerAnimation();

        //! Set player's animation component.
        inline void                                 setPlayerAnimation( EnPlayerAnimation* p_anim );

        //! Return player's physics component.
        inline EnPlayerPhysics*                     getPlayerPhysics();

        //! Set player's physics component.
        inline void                                 setPlayerPhysics( EnPlayerPhysics* p_phys );

        //! Return player's sound component.
        inline EnPlayerSound*                       getPlayerSound();

        //! Set player's sound component.
        inline void                                 setPlayerSound( EnPlayerSound* p_snd );

        //! Return player's networking component.
        inline PlayerNetworking*                    getPlayerNetworking();

        //! Set player's networking component ( used by networking component itself when a new remote client is created ).
        inline void                                 setPlayerNetworking( PlayerNetworking* p_net );

        //! Get player entity
        inline EnPlayer*                            getPlayerEntity();

        //! Set a loading post-fix, this is used for loading players when remote clients connect.
        inline void                                 setLoadingPostfix( const std::string& postFix );

    protected:

        //! Set camera mode to Spheric or Ego
        void                                        setCameraMode( unsigned int mode );

        //! Set camera's pitch and yaw angles given the mouse position [-1..1, -1..1] for looking around 
        void                                        setCameraPitchYaw( float pitch, float yaw );

        //! Set next available camera mode
        void                                        setNextCameraMode();

        //! A copy of player attributes
        EnPlayer::PlayerAttributes                  _playerAttributes;

        //! Post-fix used for loading remote clients
        std::string                                 _loadingPostFix;

        //! Association to player entity
        EnPlayer*                                   _p_player;

        //! Camera mode
        enum CameraMode
        {
            Ego,
            Spheric
        }                                           _cameraMode;

        //! Physics component
        EnPlayerPhysics*                            _p_playerPhysics;

        //! Animation control component
        EnPlayerAnimation*                          _p_playerAnimation;

        //! Sound control component
        EnPlayerSound*                              _p_playerSound;

        //! Camera entity
        EnCamera*                                   _p_camera;

        //! Networking component
        PlayerNetworking*                           _p_playerNetworking;

        //! Chat gui
        std::auto_ptr< PlayerChatGui >              _p_chatGui;

        //! Movement direction
        osg::Vec3f                                  _moveDir;

        //! Current position
        osg::Vec3f                                  _currentPos;

        //! Current rotation
        osg::Quat                                   _currentRot;

        //! Rotation about Z axis (yaw), this is used in addition to _currentRot for a better networking performance
        float                                       _rotZ;
};

// inlines
inline void BasePlayerImplementation::setPlayerPosition( const osg::Vec3f& pos ) 
{ 
    _currentPos = pos;
}

inline void BasePlayerImplementation::setPlayerRotation( const osg::Quat& rot )
{ 
    _currentRot = rot;
}

inline const osg::Vec3f& BasePlayerImplementation::getPlayerPosition() 
{
    return _currentPos;
}

inline const osg::Quat& BasePlayerImplementation::getPlayerRotation()
{ 
    return _currentRot; 
}

inline const osg::Vec3f& BasePlayerImplementation::getPlayerMoveDirection() 
{ 
    return _moveDir; 
}

inline EnPlayerAnimation* BasePlayerImplementation::getPlayerAnimation() 
{ 
    return _p_playerAnimation; 
}

inline void BasePlayerImplementation::setPlayerAnimation( EnPlayerAnimation* p_anim )
{ 
    _p_playerAnimation = p_anim; 
}

inline EnPlayerPhysics* BasePlayerImplementation::getPlayerPhysics() 
{ 
    return _p_playerPhysics; 
}

inline void BasePlayerImplementation::setPlayerPhysics( EnPlayerPhysics*  p_phys ) 
{ 
    _p_playerPhysics = p_phys; 
}

inline EnPlayerSound* BasePlayerImplementation::getPlayerSound() 
{ 
    return _p_playerSound; 
}

inline void BasePlayerImplementation::setPlayerSound( EnPlayerSound* p_snd ) 
{ 
    _p_playerSound = p_snd; 
}

inline PlayerNetworking* BasePlayerImplementation::getPlayerNetworking() 
{ 
    return _p_playerNetworking; 
}

inline void BasePlayerImplementation::setPlayerNetworking( PlayerNetworking* p_net ) 
{ 
    _p_playerNetworking = p_net; 
}

inline EnPlayer* BasePlayerImplementation::getPlayerEntity() 
{ 
    return _p_player; 
}

inline void BasePlayerImplementation::setLoadingPostfix( const std::string& postFix )
{
    _loadingPostFix = postFix;
}

} // namespace CTD

#endif // _CTD_PLAYERIMPL_H_