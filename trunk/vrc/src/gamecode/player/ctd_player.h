/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
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

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_PLAYER    "Player"

// forward declaration of player entity and its composition
//-------
class PlayerPhysics;
class PlayerAnimation;
template< class PlayerPhysicsT, class PlayerAnimationT > class Player;
typedef Player< class PlayerPhysics, class PlayerAnimation > EnPlayer;
//-------

//! Class for player entity
template
< 
    class PlayerPhysicsT, 
    class PlayerAnimationT
>
class Player : public BaseEntity
{
    public:

                                                    Player();

                                                    ~Player();


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

        /**
        * Apply players movement force
        * \param force                              Force to apply to player for movement
        */
        void                                        applyForce( const osg::Vec3f& force );

    protected:

        // entity parameters
        //----------------------------------------------------------//

        //! Player name
        std::string                                 _playerName;

        //! Animation config file
        std::string                                 _animCfgFile;

        //! Dimentions of player: width / length / height
        osg::Vec3f                                  _dimensions;

        //! initial position
        osg::Vec3f                                  _position;

        //! Step height
        float                                       _stepHeight;

        //! Max force for movement
        float                                       _linearForce;

        //! Max force for rotation
        float                                       _angularForce;

        //! Player's mass
        float                                       _mass;

        //! Force for movement
        osg::Vec3f                                  _force;

        //! Rotation about up vector
        float                                       _rotation;

        //! Player's gravity, default is the Physics system gravity
        float                                       _gravity;

        //! Linear damping
        float                                       _linearDamping;

        // the following is for internal use
        //----------------------------------------------------------//

        //! Physics component
        PlayerPhysicsT*                             _p_playerPhysics;

        //! Animation control component
        PlayerAnimationT*                           _p_playerAnimation;

        //! Movement direction
        osg::Vec3f                                  _moveDir;

        //! Input handler class
        class InputHandler : public osgGA::GUIEventHandler
        {
            public:

                                                    InputHandler( Player< PlayerPhysicsT, PlayerAnimationT > *p_player ) : _p_player( p_player ) {}
                                                    
                                                    ~InputHandler() {}

                /**
                * Handle input events.
                */
                bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

            protected:

                Player< PlayerPhysicsT, PlayerAnimationT >* _p_player;
        };

    friend typename PlayerPhysicsT;
    friend typename PlayerAnimationT;
    friend class    InputHandler;
};

//! Entity type definition used for type registry
class PlayerEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerEntityFactory() : BaseEntityFactory(ENTITY_NAME_PLAYER) {}

        virtual                                     ~PlayerEntityFactory() {}

        Macro_CreateEntity( EnPlayer );
};

} // namespace CTD

#endif // _CTD_PLAYER_H_
