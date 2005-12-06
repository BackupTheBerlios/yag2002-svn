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
 # player sound
 #
 # this class implements the player sound control
 #
 #   date of creation:  03/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/19/2005 boto       creation of PlayerSound
 #
 ################################################################*/

#ifndef _VRC_PLAYERSOUND_H_
#define _VRC_PLAYERSOUND_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_PLSOUND   "PlayerSound"

class BasePlayerImplementation;

//! Entity for player sounds
class EnPlayerSound  : public yaf3d::BaseEntity
{

    public:

                                                    EnPlayerSound();

        virtual                                     ~EnPlayerSound();

        //! This entity does not need a transform node.
        const bool                                  isTransformable() const { return false; }

        //! Set player association, player must call this in post-initialize phase
        void                                        setPlayer( BasePlayerImplementation* p_player );

        //! Post-Initializing function
        void                                        postInitialize();

        //! Update the sound location
        void                                        updatePosition( const osg::Vec3f& pos );

        //! Play walk on ground sound
        void                                        playWalkGround();

        //! Play walk on wood sound
        void                                        playWalkWood();

        //! Play walk on metal sound
        void                                        playWalkMetal();

        //! Play walk on grass sound
        void                                        playWalkGrass();

        //! Stop any sound
        void                                        stopPlayingAll();

    protected:

        // Handler system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        // entity attributes
        //----------------------------------------------------------//

        //! Sound's position offset relative to player position
        osg::Vec3f                                  _offset;

        //! Sound volume
        float                                       _volume;

        //! Begin of rolloff ( min distance )
        float                                       _minDistance;

        //! At this distance the volume will be 0
        float                                       _maxDistance;

        //! Walk sound file, on ground
        std::string                                 _walkGround;

        //! Walk sound file, on wood
        std::string                                 _walkWood;

        //! Walk sound file, on metal
        std::string                                 _walkMetal;

        //! Walk sound file, on grass
        std::string                                 _walkGrass;

        //----------------------------------------------------------//

        //! Create a new sound
        unsigned int                                createSound( const std::string& filename );

        //! Type for sound name ( as uint ) / sound id maps
        typedef std::map< unsigned int, unsigned int > MapPlayerSounds;
        
        //! All sound name / sound id pairs go here
        MapPlayerSounds                             _mapSounds;

        BasePlayerImplementation*                   _p_playerImpl;
};

//! Entity type definition used for type registry
class PlayerSoundEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerSoundEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLSOUND, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerSoundEntityFactory() {}

        Macro_CreateEntity( EnPlayerSound );
};

} // namespace vrc

#endif // _VRC_PLAYERSOUND_H_
