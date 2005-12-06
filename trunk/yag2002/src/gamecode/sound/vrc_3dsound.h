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
 # entity 3D sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_3DSOUND_H_
#define _VRC_3DSOUND_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_3DSOUND    "3DSound"

//! 3DSound Entity
class En3DSound : public yaf3d::BaseEntity
{
    public:
                                                    En3DSound();

        virtual                                     ~En3DSound();

        //! Initialize 
        void                                        initialize();

        //! Start / continue playing sound
        void                                        startPlaying();

        //! Stop playing sound, pass 'true' in order to pause only, otherwise the sound is stopped
        void                                        stopPlaying( bool pause = false );

        //! Set sound volume (0..1)
        void                                        setVolume( float volume );

        //! Get sound volume (0..1)
        float                                       getVolume();

    protected:

        // Handler system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Sound file name
        std::string                                 _soundFile;

        //! Loop playing?
        bool                                        _loop;

        //! Sound position
        osg::Vec3f                                  _position;

        //! If true then the sound is immediately played after creation
        bool                                        _autoPlay;

        //! Sound volume [ 0..1 ]
        float                                       _volume;

        //! Begin of rolloff ( min distance )
        float                                       _minDistance;

        //! At this distance the volume will be 0
        float                                       _maxDistance;

        //! Flag indicating whether the sound is currently playing
        bool                                        _isPlaying;

        //! Flag indicating whether the sound was playing before entering the menu
        bool                                        _wasPlaying;

        //! Mesh file name sound source ( is usually used during level creation for debugging purpose )
        std::string                                 _sourceMesh;

        //! Sound ID
        unsigned int                                _soundID;

        //! Sound channel
        FMOD::Channel*                              _p_channel;
};

//! Entity type definition used for type registry
class ThreeDSoundEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    ThreeDSoundEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_3DSOUND, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~ThreeDSoundEntityFactory() {}

        Macro_CreateEntity( En3DSound );
};

}

#endif // _VRC_3DSOUND_H_
