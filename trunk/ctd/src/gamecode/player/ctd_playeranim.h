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

#include <ctd_main.h>

#include <osgCal/CoreModel>
#include <osgCal/Model>

namespace CTD
{

#define ENTITY_NAME_PLANIM    "PlayerAnimation"

class BasePlayerImplementation;

//! Class for controling player animations
class EnPlayerAnimation  : public BaseEntity
{

    public:

                                                    EnPlayerAnimation();

        virtual                                     ~EnPlayerAnimation();

        /**
        * Set player association before using this entity.
        * \param p_player                           Player instance
        */
        void                                        setPlayer( BasePlayerImplementation* p_player );

        /**
        * Enable / disable rendering
        */
        void                                        enableRendering( bool render );

        /**
        * Initializing function
        */
        void                                        initialize();

        //! Idle animation
        void                                        animIdle();

        //! Walk animation
        void                                        animWalk();

        //! Jump animation
        void                                        animJump();
 
        //! Turn animation
        void                                        animTurn();

        //! This entity does not need a transform node, which would be created by level manager on loading
        //!   We create an own one which is given to player after initialization ( see setPlayer ).
        const bool                                  isTransformable() const { return false; }

    protected:

        /**
        * Update entity
        * \param deltaTime                          Time passed since last update
        */
        void                                        updateEntity( float deltaTime );

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

        BasePlayerImplementation*                   _p_player;

        osg::ref_ptr< osgCal::Model >               _model;

        osg::ref_ptr< osgCal::CoreModel >           _coreModel;

        osg::ref_ptr< osg::PositionAttitudeTransform > _animNode;

        bool                                        _renderingEnabled;

        enum 
        {
            eIdle,
            eWalk,
            eJump,
            eTurn
        }                                           _anim;

        //! offset position
        osg::Vec3f                                  _position;

        //! offset rotation
        osg::Vec3f                                  _rotation;

        //! Uniform scale
        float                                       _scale;

        // animation ids
        int                                         _IdAnimIdle;
        int                                         _IdAnimWalk;
        int                                         _IdAnimRun;
        int                                         _IdAnimJump;
        int                                         _IdAnimLand;
        int                                         _IdAnimTurn;
};

//! Entity type definition used for type registry
class PlayerAnimationEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerAnimationEntityFactory() :
                                                     BaseEntityFactory( ENTITY_NAME_PLANIM, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerAnimationEntityFactory() {}

        Macro_CreateEntity( EnPlayerAnimation );
};

} // namespace CTD

#endif // _CTD_PLAYERANIM_H_
