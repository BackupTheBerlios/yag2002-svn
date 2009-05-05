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
 # player sound
 #
 # this class implements the player sound control
 #
 #   date of creation:  03/19/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
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

#define ENTITY_NAME_PLSOUND     "PlayerSound"

//! Internal material names
#define SND_GROUND              1
#define SND_WOOD                2
#define SND_STONE               3
#define SND_METAL               4
#define SND_GRASS               5

class BasePlayerImplementation;

//! Entity for player sounds
class EnPlayerSound  : public yaf3d::BaseEntity
{

    public:

                                                    EnPlayerSound();

        virtual                                     ~EnPlayerSound();

        //! This entity does not need a transform node.
        bool                                        isTransformable() const { return false; }

        //! Set player association, player must call this in post-initialize phase
        void                                        setPlayer( BasePlayerImplementation* p_player );

        //! Post-Initializing function
        void                                        postInitialize();

        //! Update the sound location
        void                                        updatePosition( const osg::Vec3f& pos );

        //! Stop any sound
        void                                        stopPlayingAll();

        //! Play walk on ground sound
        inline void                                 playWalkGround();

        //! Play walk on wood sound
        inline void                                 playWalkWood();

        //! Play walk on stone sound
        inline void                                 playWalkStone();

        //! Play walk on metal sound
        inline void                                 playWalkMetal();

        //! Play walk on grass sound
        inline void                                 playWalkGrass();

    protected:

        // Handler system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup all sounds
        void                                        setupSounds();

        //! Release all sounds
        void                                        releaseSounds();

        //! Create a new sound
        unsigned int                                createSound( const std::string& filename );

        //! Play sound given its name ( as uint )
        void                                        playSoundFx( unsigned int soundName );

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

        //! Walk sound file, on stone
        std::string                                 _walkStone;

        //! Walk sound file, on metal
        std::string                                 _walkMetal;

        //! Walk sound file, on grass
        std::string                                 _walkGrass;

        //----------------------------------------------------------//


        //! Type for sound name ( as uint ) / sound id maps
        typedef std::map< unsigned int, unsigned int > MapPlayerSounds;

        //! All sound name / sound id pairs go here
        MapPlayerSounds                             _mapSounds;

        //! Sound position
        FMOD_VECTOR                                 _soundPosition;

        //! Player implementation
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

//! Inlines
inline void EnPlayerSound::playWalkGround()
{
    playSoundFx( SND_GROUND );
}

inline void EnPlayerSound::playWalkWood()
{
    playSoundFx( SND_WOOD );
}

inline void EnPlayerSound::playWalkMetal()
{
    playSoundFx( SND_METAL );
}

inline void EnPlayerSound::playWalkStone()
{
    playSoundFx( SND_STONE );
}

inline void EnPlayerSound::playWalkGrass()
{
    playSoundFx( SND_GRASS );
}

} // namespace vrc

#endif // _VRC_PLAYERSOUND_H_
