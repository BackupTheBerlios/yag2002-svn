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
 # entity 2D sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #   04/16/2008         changed EnAmbientSound entity to En2DSound
 #
 ################################################################*/

#ifndef _VRC_2DSOUND_H_
#define _VRC_2DSOUND_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_2DSOUND    "2DSound"

//! 2D sound Entity providing ambient and non-ambient ( distance-based ) sound sources
class En2DSound : public yaf3d::BaseEntity
{
    public:
                                                    En2DSound();

        virtual                                     ~En2DSound();

        //! This entity does not need a transform node, which would be created by level manager on loading
        const bool                                  isTransformable() const { return false; }

        //! Initialize 
        void                                        initialize();

        //! Start / continue playing sound, set 'cont' to true in order to continue.
        void                                        startPlaying( bool cont = false );

        //! Stop playing sound, pass 'true' in order to pause only, otherwise the sound is stopped
        void                                        stopPlaying( bool pause = false );

        //! Set sound volume [ 0..1 ]
        void                                        setVolume( float volume );

        //! Get sound volume [ 0..1 ]
        float                                       getVolume();

    protected:

        // Handler system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Update entity, needed for non-ambient sound
        void                                        updateEntity( float deltaTime );

        //! Setup or destroy sound depending on menu settings
        void                                        setupSound();

        //! Sound file name
        std::string                                 _soundFile;

        //! Loop playing?
        bool                                        _loop;

        //! If true then the sound is immediately played after creation
        bool                                        _autoPlay;

        //! Sound volume [ 0..1 ]
        float                                       _volume;

        //! If the sound is not ambient then use the min / max distance for a linear attenuation
        bool                                        _ambient;

        //! Sound source position used for non-ambient sound
        osg::Vec3f                                  _position;

        //! Minimum distance
        float                                       _minDistance;

        //! Maximum distance
        float                                       _maxDistance;

        //! Attenuation calculated for non-ambient sound basing on distance to player
        float                                       _attenuation;

        //! Upate timer used for non-ambient sound
        float                                       _updateTimer;

        //! Sound group ( see yaf3d::SoundManager for valid values )
        std::string                                 _soundGroup;

        //! Sound ID
        unsigned int                                _soundID;

        //! Sound channel
        FMOD::Channel*                              _p_channel;

        //! Player object needed for non-ambient sound
        yaf3d::BaseEntity*                          _p_player;
};

//! Entity type definition used for type registry
class TwoDSoundEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    TwoDSoundEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_2DSOUND, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~TwoDSoundEntityFactory() {}

        Macro_CreateEntity( En2DSound );
};

}

#endif // _VRC_2DSOUND_H_
