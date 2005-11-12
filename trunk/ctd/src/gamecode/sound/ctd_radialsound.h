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
 # entity radial sound, this sound source is distributed with a 
 #  linear attenuation in all directions
 #
 #   date of creation:  11/11/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_RADIALSOUND_H_
#define _CTD_RADIALSOUND_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_RADIALSOUND    "RadialSound"

class RadialSoundIH;

//! RadialSound Entity
class EnRadialSound : public BaseEntity
{
    public:
                                                    EnRadialSound();

        virtual                                     ~EnRadialSound();

        //! Initialize 
        void                                        initialize();

        //! Start / continue playing sound
        void                                        startPlaying();

        //! Stop playing sound, pass 'true' in order to pause only, otherwise the sound is stopped
        void                                        stopPlaying( bool pause = false );

        //! Set sound volume (0..1)
        inline void                                 setVolume( float volume );

        //! Get sound volume (0..1)
        inline float                                getVolume();

        //! Set the volume damping factor
        inline void                                 setDamping( float damping );

        //! Get sound radius
        inline float                                getSoundRadius() const;

        //! Get the sound source position
        inline const osg::Vec3f&                    getSoundPosition() const;

    protected:

        // Handler system notifications
        void                                        handleNotification( const EntityNotification& notification );

        //! Resource directory for searching for sound files
        std::string                                 _soundFileDir;

        //! Sound file name
        std::string                                 _soundFile;

        //! Sound source position
        osg::Vec3f                                  _position;

        //! Loop playing?
        bool                                        _loop;

        //! If true then the sound is immediately played after creation
        bool                                        _autoPlay;

        //! Sound volume [ 0..1 ]
        float                                       _volume;

        //! Max distribution radious
        float                                       _radius;

        //! Flag indicating whether the sound is currently playing
        bool                                        _isPlaying;

        //! Flag indicating whether the sound was playing before entering the menu
        bool                                        _wasPlaying;

        //! Mesh file name sound source ( is usually used during level creation for debugging purpose )
        std::string                                 _sourceMesh;

        osgAL::SoundNode*                           _p_soundNode;

        osg::ref_ptr< osgAL::SoundState >           _soundState;

        osg::ref_ptr< RadialSoundIH >               _p_soundUpdater;
};

//! Entity type definition used for type registry
class RadialSoundEntityFactory : public BaseEntityFactory
{
    public:
                                                    RadialSoundEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_RADIALSOUND, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~RadialSoundEntityFactory() {}

        Macro_CreateEntity( EnRadialSound );
};


//! Inlines
inline void EnRadialSound::setVolume( float volume )
{
    _volume = std::max( std::min( volume, 1.0f ), 0.0f );
    if ( _soundState.get() )
        _soundState->setGain( _volume );
}

inline float EnRadialSound::getVolume()
{
    return _volume;
}

inline float EnRadialSound::getSoundRadius() const
{
    return _radius;
}

inline void EnRadialSound::setDamping( float damping )
{
    assert( damping >= 0.0f && "minimal value is 0.0" );
    _soundState->setGain( damping * _volume );
}

inline const osg::Vec3f& EnRadialSound::getSoundPosition() const
{
    return _position;
}

}

#endif // _CTD_RADIALSOUND_H_
