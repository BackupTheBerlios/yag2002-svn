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
 # player animation
 #
 # this class implements the player animation control
 #
 #   date of creation:  03/13/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/13/2005 boto       creation of PlayerAnimation
 #
 ################################################################*/

#ifndef _VRC_PLAYERANIM_H_
#define _VRC_PLAYERANIM_H_

#include <vrc_main.h>

#include <osgCal/CoreModel>
#include <osgCal/Model>
//! TODO: put this header to framework's base.h
#include <osgText/Text>

namespace vrc
{

#define ENTITY_NAME_PLANIM    "PlayerAnimation"

class BasePlayerImplementation;

//! Class for controling player animations
class EnPlayerAnimation  : public yaf3d::BaseEntity
{

    public:

                                                    EnPlayerAnimation();

        virtual                                     ~EnPlayerAnimation();

        //! Set player association before using this entity.
        void                                        setPlayer( BasePlayerImplementation* p_player );

        //! Initializing function
        void                                        initialize();

        //! Enable / disable the text display
        void                                        enableTextDisplay( bool en );

        //! Set the default player text for display
        void                                        setPlayerText( const std::string& text );

        //! Display a text for given duration, after the duration is expired the player text is displayed.
        void                                        displayText( const std::string& text, float duration = 3.0f );

        //! Idle animation
        void                                        animIdle();

        //! Walk animation
        void                                        animWalk();

        //! Jump animation
        void                                        animJump();
 
        //! Turn animation
        void                                        animTurn();

        //! Get current animation as flags ( used for networking )
        unsigned char                               getAnimationFlags();

        //! Set current animation as flags ( used for networking )
        void                                        setAnimation( unsigned char flags );

        //! This entity does not need a transform node, which would be created by level manager on loading
        //!   We create an own one which is given to player after initialization ( see setPlayer ).
        const bool                                  isTransformable() const { return false; }

        //! Animation flags
        enum AnimationFlag
        {
            eIdle = 0x0,
            eWalk,
            eJump,
            eTurn
        }                                           _anim;

    protected:

        //! Override this method of yaf3d::BaseEntity to get notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Setup the shader
        void                                        setupShader();

        //! Read config file and setup animation
        bool                                        setupAnimation( const std::string& rootDir, const std::string& configfilename );

        //! Create a tex node for displaying the player specific info (e.g. player name)
        osg::ref_ptr< osgText::Text >               createTextNode();

        // entity attributes
        //----------------------------------------------------------//

        //! Animation config file
        std::string                                 _animCfgFile;

        //----------------------------------------------------------//

        BasePlayerImplementation*                   _p_player;

        osg::ref_ptr< osgCal::Model >               _model;

        osg::ref_ptr< osgCal::CoreModel >           _coreModel;

        osg::ref_ptr< osg::PositionAttitudeTransform > _animNode;

        osg::ref_ptr< osg::Geode >                  _playerTextGeode;

        osg::ref_ptr < osgText::Text >              _playerText;

        bool                                        _renderingEnabled;

        bool                                        _useTexture;

        //! Enable / disable the player text
        bool                                        _enableDisplayText;

        // Default player text to display
        std::string                                 _displayText;

        //! Timer used for displaying a text
        float                                       _textDisplayTimer;

        //! Flag identifying whether the player text is currently visible
        bool                                        _textVisible;

        //! offset position
        osg::Vec3f                                  _position;

        //! offset rotation
        osg::Vec3f                                  _rotation;

        //! Uniform scale
        float                                       _scale;

        //! Maximal distance for getting the lowest LOD
        float                                       _maxLODDistance;

        //! One over max LOD distance
        float                                       _divMaxLODDistance;

        // animation ids
        int                                         _IdAnimIdle;
        int                                         _IdAnimWalk;
        int                                         _IdAnimRun;
        int                                         _IdAnimJump;
        int                                         _IdAnimLand;
        int                                         _IdAnimTurn;
};

//! Entity type definition used for type registry
class PlayerAnimationEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerAnimationEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLANIM, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerAnimationEntityFactory() {}

        Macro_CreateEntity( EnPlayerAnimation );
};

} // namespace vrc

#endif // _VRC_PLAYERANIM_H_
