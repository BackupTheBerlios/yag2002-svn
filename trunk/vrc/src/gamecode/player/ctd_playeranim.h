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
#include "ctd_player.h"
#include <rbody/osg/ReplicantBodyMgr.h>
#include <rbody/osg/OsgBody.h>

namespace CTD
{

//! Class for controling player animations
class PlayerAnimation
{

    public:

                                                    PlayerAnimation( EnPlayer* p_player );

                                                    ~PlayerAnimation();


        /**
        * Initializing function
        */
        void                                        initialize();

        /**
        * Update
        * \param deltaTime                          Time passed since last update
        */
        void                                        update( float deltaTime );

        //! Returns the osg node where the character is attached to
        osg::Node*                                  getNode();

        //! Trigger idle animation
        void                                        actionIdle();

        //! Trigger walk animation
        void                                        actionWalk();

        //! Trigger jump animation
        void                                        actionJump();

    protected:

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

inline osg::Node* PlayerAnimation::getNode()
{
    return _p_node;
}

} // namespace CTD

#endif // _CTD_PLAYERANIM_H_
