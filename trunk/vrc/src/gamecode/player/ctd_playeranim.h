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
 # player animation
 #
 # this class implements the player animation control
 #
 #   date of creation:  03/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/13/2005 boto       creation of PlayerAnimation
 #
 ################################################################*/

#ifndef _CTD_PLAYERANIM_H_
#define _CTD_PLAYERANIM_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include <rbody/osg/ReplicantBodyMgr.h>
#include <rbody/osg/OsgBody.h>

namespace CTD
{

#define ENTITY_NAME_PLANIM    "PlayerAnimation"

class EnPlayer;

//! Class for controling player animations
class EnPlayerAnimation  : public BaseEntity
{

    public:

                                                    EnPlayerAnimation();

        virtual                                     ~EnPlayerAnimation();

        //! This entity does not need a transform node, which would be created by level manager on loading
        bool                                        needTransformation() { return false; }

        /**
        * Set player association, player must call this in post-initialize phase
        * \param p_player                           Player instance
        */
        void                                        setPlayer( EnPlayer* p_player );

        /**
        * Initializing function
        */
        void                                        initialize();

        /**
        * Destroy animation system. This must be called during Player's destruction.
        */
        void                                        destroy();

        /**
        * Update called by EnPlayer entity. Note: this is not the framework update method!
        * \param deltaTime                          Time passed since last update
        */
        void                                        update( float deltaTime );

        //! Trigger idle animation
        void                                        actionIdle();

        //! Trigger walk animation
        void                                        actionWalk();

        //! Trigger jump animation
        void                                        actionJump();

    protected:

        // entity attributes
        //----------------------------------------------------------//

        //! Animation config file
        std::string                                 _animCfgFile;

        //----------------------------------------------------------//

        EnPlayer*                                   _p_player;

        osg::Group*                                 _p_characterGrp;

        rbody::ReplicantBodyMgr*                    _p_replicantBodyMgr;
        
        rbody::OsgBodyNode*                         _p_body;

        osg::PositionAttitudeTransform*             _p_node;

        enum 
        {
            eIdle,
            eWalk,
            eJump
        }                                           _action;

};

//! Entity type definition used for type registry
class PlayerAnimationEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerAnimationEntityFactory() : BaseEntityFactory(ENTITY_NAME_PLANIM) {}

        virtual                                     ~PlayerAnimationEntityFactory() {}

        Macro_CreateEntity( EnPlayerAnimation );
};

} // namespace CTD

#endif // _CTD_PLAYERANIM_H_
