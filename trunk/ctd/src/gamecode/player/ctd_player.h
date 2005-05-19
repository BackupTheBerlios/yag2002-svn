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
 # player
 #
 # this class implements the player
 #
 #
 #   date of creation:  01/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/14/2005 boto       creation of Player
 #
 ################################################################*/

#ifndef _CTD_PLAYER_H_
#define _CTD_PLAYER_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_PLAYER    "Player"

class EnPlayerAnimation;
class EnPlayerPhysics;
class EnPlayerSound;
class EnCamera;
class PlayerChatGui;
class PlayerInputHandler;

//! Player entity
class EnPlayer : public BaseEntity
{
    public:

                                                    EnPlayer();

        virtual                                     ~EnPlayer();


        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        void                                        postInitialize();

        /**
        * Update entity
        * \param deltaTime                          Time passed since last update
        */
        void                                        updateEntity( float deltaTime );

        //! Override this method of BaseEntity to get notifications (from menu system)
        void                                        handleNotification( EntityNotification& notify );

        //! Return player's sound component, used by physics component
        EnPlayerSound*                              getPlayerSound() { return _p_playerSound; }

        //! Set player's position. It is used by physics.
        inline void                                 setPlayerPosition( const osg::Vec3f& pos );

        //! Set player's rotation. It is used by physics.
        inline void                                 setPlayerRotation( const osg::Quat& rot );

        //! Set camera mode to Spheric or Ego
        void                                        setCameraMode( unsigned int mode );

        //! Set next camera mode
        void                                        setNextCameraMode();

        //! Enable / disable input processing and control
        void                                        enableControl( bool en );

    protected:

        // entity attributes
        //----------------------------------------------------------//

        //! Player name
        std::string                                 _playerName;

        //! Physics entity's instance name which will be attached to player
        std::string                                 _physicsEntity;

        //! Animation entity's instance name which will be attached to player
        std::string                                 _animationEntity;

        //! Sound entity's instance name which will be attached to player
        std::string                                 _soundEntity;

        //! Initial position
        osg::Vec3f                                  _position;

        //! Rotation
        osg::Quat                                   _rotation;

        //! Camera's position offset for spheric mode
        osg::Vec3f                                  _camPosOffsetSpheric;

        //! Camera's rotation offset for spheric mode ( roll/pitch/yaw in degrees )
        osg::Vec3f                                  _camRotOffsetSpheric;

        //! Camera's position offset for ego mode
        osg::Vec3f                                  _camPosOffsetEgo;

        //! Camera's rotation offset for ego mode ( roll/pitch/yaw in degrees )
        osg::Vec3f                                  _camRotOffsetEgo;

        //! CEGUI layout file for chat
        std::string                                 _chatGuiConfig;

    protected:

        // the following is for internal use
        //----------------------------------------------------------//

        //! Set camera's pitch and yaw angles given the mouse position [-1..1, -1..1]
        //  for looking around
        void                                        setCameraPitchYaw( float pitch, float yaw );

        //! Get the configuration settings
        void                                        getConfiguration();

        //! Camera mode
        enum CameraMode
        {
            Ego,
            Spheric
        }                                           _cameraMode;

        //! Enable / disable input processing
        bool                                        _enabledControl;

        //! Physics component
        EnPlayerPhysics*                            _p_playerPhysics;

        //! Animation control component
        EnPlayerAnimation*                          _p_playerAnimation;

        //! Sound control component
        EnPlayerSound*                              _p_playerSound;

        //! Camera entity
        EnCamera*                                   _p_camera;

        //! Chat gui
        std::auto_ptr< PlayerChatGui >              _p_chatGui;

        //! Movement direction
        osg::Vec3f                                  _moveDir;

        //! Rotation about Z axis
        float                                       _rot;

        PlayerInputHandler*                         _p_inputHandler;

    friend class EnPlayerPhysics;
    friend class EnPlayerAnimation;
    friend class EnPlayerSound;
    friend class PlayerInputHandler;
};

//! Entity type definition used for type registry
class PlayerEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerEntityFactory() : BaseEntityFactory(ENTITY_NAME_PLAYER) {}

        virtual                                     ~PlayerEntityFactory() {}

        Macro_CreateEntity( EnPlayer );
};

// inlines
inline void EnPlayer::setPlayerPosition( const osg::Vec3f& pos )
{
    _position = pos;
}

inline void EnPlayer::setPlayerRotation( const osg::Quat& rot )
{
    _rotation = rot;
}

} // namespace CTD

#endif // _CTD_PLAYER_H_
