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

#include <osgCal/CoreModel>
#include <osgCal/Model>

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
        //!   We create an own one which is given to player after initialization ( see setPlayer ).
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
        * Returns true if the initialization was successful.
        */
        bool                                        isReady() { return _ready; }

        /**
        * Destroy animation system. This must be called during Player's destruction.
        */
        void                                        destroy();

        //! Trigger idle animation
        void                                        actionIdle();

        //! Trigger walk animation
        void                                        actionWalk();

        //! Trigger jump animation
        void                                        actionJump();

    protected:

        //! Read config file and setup animation
        /**
        * \param rootDir                            The relative directory path to media directory which must contain all animation files and textures
        * \param configfilename                     Cfg file name
        */
        bool                                        setupAnimation( const std::string& rootDir, const std::string& configfilename );

        // entity attributes
        //----------------------------------------------------------//

        //! Animation config file
        std::string                                 _animCfgFile;

        //----------------------------------------------------------//

        //! Shows whether inititialization was successful
        bool                                        _ready;

        EnPlayer*                                   _p_player;

        osg::ref_ptr< osgCal::Model >               _model;

        osg::ref_ptr< osgCal::CoreModel >           _coreModel;

        CalCoreModel*                               _p_calCoreModel;

        osg::ref_ptr< osg::PositionAttitudeTransform > _animNode;

        enum 
        {
            eIdle,
            eWalk,
            eJump
        }                                           _action;

        //! offset position
        osg::Vec3f                                  _position;

        //! offset rotation
        osg::Vec3f                                  _rotation;

        float                                       _scale;

        // animation ids
        int                                         _IdAnimIdle;
        int                                         _IdAnimWalk;
        int                                         _IdAnimRun;
        int                                         _IdAnimJump;
        int                                         _IdAnimLand;        
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