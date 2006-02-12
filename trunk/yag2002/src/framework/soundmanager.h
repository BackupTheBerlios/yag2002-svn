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
 # sound manager using fmod -- http://www.fmod.org
 #
 #   date of creation:  12/03/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _SOUNDMANAGER_H_
#define _SOUNDMANAGER_H_

#include <base.h>
#include <singleton.h>
#include <application.h>

namespace yaf3d
{

class Application;
class LevelManager;

//! Class for sound related exceptions
class SoundException : public std::runtime_error
{
    public:
                                                    SoundException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                    ~SoundException() throw() {}

                                                    SoundException( const SoundException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    SoundException();

        SoundException&                             operator = ( const SoundException& );
};

//! Sound manager
/**
* Singleton for setting up sound system and managing playing pausing etc.
* This class uses fmod ( http://www.fmod.org ).
*/
class SoundManager : public Singleton< SoundManager >
{
    public:

        //! Default flags for creation of common sound types.
        enum
        {
            fmodDefaultCreationFlags2D     = FMOD_LOOP_OFF    | FMOD_2D | FMOD_SOFTWARE,
            fmodDefaultCreationFlags2DLoop = FMOD_LOOP_NORMAL | FMOD_2D | FMOD_SOFTWARE,
            fmodDefaultCreationFlags3D     = FMOD_LOOP_OFF    | FMOD_3D | FMOD_SOFTWARE | FMOD_3D_LINEARROLLOFF,
            fmodDefaultCreationFlags3DLoop = FMOD_LOOP_NORMAL | FMOD_3D | FMOD_SOFTWARE | FMOD_3D_LINEARROLLOFF
        };

        //! Create sound given a file name and creation flags. Returns a sound ID > 0 which is used for furhter operations such as releasing the sound.
        unsigned int                                createSound( const std::string& file, float volume = 1.0f, bool autoplay = false, unsigned int flags = fmodDefaultCreationFlags2D ) throw ( SoundException );

        //! Release sound resouce given its id.
        void                                        releaseSound( unsigned int soundID ) throw ( SoundException );

        //! Return associated channel for given sound.
        FMOD::Channel*                              getSoundChannel( unsigned int soundID ) throw ( SoundException );

        //! Play sound with given ID. Pass true for 'autocheck' in order to avoid playing the sound when it is already playing.
        void                                        playSound( unsigned int soundID, bool autocheck = false );

        //! Pause sound with given ID.
        void                                        pauseSound( unsigned int soundID );

        //! Continue a paused sound with given ID.
        void                                        continueSound( unsigned int soundID );

        //! Stop sound with given ID. Next playSound call will start at begin of sound track.
        void                                        stopSound( unsigned int soundID );

        //! Set volume [0..1] for sound with given ID.
        void                                        setSoundVolume( unsigned int soundID, float volume );

        //! Set master volume [0..1]
        void                                        setMasterVolume( float volume );

        //! Mute / demute all channels
        void                                        mute( bool mut );

    protected:

                                                    SoundManager();

        virtual                                     ~SoundManager();

        //! Initialize the sound system
        void                                        initialize() throw ( SoundException );

        //! Shutdown
        void                                        shutdown();

        //! Update sound system
        void                                        update( float deltaTime );

        //! fmod's sound system instance
        FMOD::System*                               _p_system;

        //! Structure used for internal storage of Sound sources
        struct SoundData
        {
            FMOD::Sound*                                _p_sound;

            FMOD::Channel*                              _p_channel;
        };

        //! Type for sound data map
        typedef std::map< unsigned int, SoundData* >    MapSoundData;

        //! Sound data map for internal housekeeping
        MapSoundData                                _mapSoundData;

        //! Used for creating unique IDs for sound sources
        unsigned int                                _soundIDCounter;

        //! Listener related parameters updated every frame
        FMOD_VECTOR                                 _listenerPosition;
        
        FMOD_VECTOR                                 _listenerForward;
        
        FMOD_VECTOR                                 _listenerUp;

        osg::Vec3f                                  _lastPosition;

        osgUtil::SceneView*                         _p_sceneView;

    friend class Singleton< SoundManager >;
    friend class LevelManager;
    friend class Application;
};

} // namespace yaf3d

#endif //_SOUNDMANAGER_H_
