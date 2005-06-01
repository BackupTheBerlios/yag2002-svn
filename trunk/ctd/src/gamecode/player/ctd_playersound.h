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

#ifndef _CTD_PLAYERSOUND_H_
#define _CTD_PLAYERSOUND_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_PLSOUND   "PlayerSound"

class BasePlayerImplementation;

//! Entity for player sounds
class EnPlayerSound  : public BaseEntity
{

    public:

                                                    EnPlayerSound();

        virtual                                     ~EnPlayerSound();

        /**
        * Set player association, player must call this in post-initialize phase
        * \param p_player                           Player instance
        */
        void                                        setPlayer( BasePlayerImplementation* p_player );

        /**
        * Post-Initializing function
        */
        void                                        postInitialize();

        //! This entity does not need a transform node, which would be created by level manager on loading
        bool                                        isTransformable() { return false; }

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

        //! Helper method; stops all other sounds except given one ( p_state )
        void                                        stopOtherSounds( osgAL::SoundState* p_state );

        // entity attributes
        //----------------------------------------------------------//

        //! Resource directory for searching for sound files
        std::string                                 _soundFileDir;

        //! Sound volume
        float                                       _volume;

        //! Reference distance
        float                                       _referenceDist;

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
        osgAL::SoundState*                          createSound( const std::string& filename );

        //! All sound name / sound state pairs go here
        std::map< std::string, osg::ref_ptr< osgAL::SoundState > > _soundStates;

        //! Group where all sound nodes reside
        osg::ref_ptr< osg::Group >                  _p_soundGroup;

        BasePlayerImplementation*                   _p_playerImpl;

    friend class EnPlayerSound;
};

//! Entity type definition used for type registry
class PlayerSoundEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerSoundEntityFactory() :
                                                     BaseEntityFactory( ENTITY_NAME_PLSOUND, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerSoundEntityFactory() {}

        Macro_CreateEntity( EnPlayerSound );
};

} // namespace CTD

#endif // _CTD_PLAYERSOUND_H_
