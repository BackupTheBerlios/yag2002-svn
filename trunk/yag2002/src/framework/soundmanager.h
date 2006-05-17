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

        //! Sound groups having own master volume
        enum
        {
            SoundGroupMaster = 0x10,  // the main sound group containing the following three groups
            SoundGroupCommon,         // common sound group used e.g. for GUI sounds   
            SoundGroupMusic,          // can be used e.g. for background sound
            SoundGroupFX              // special effect sound group
        };

        //! Class for storage of Sound resources
        class SoundResource
        {
            public:

                //! Get sound object
                FMOD::Sound* const                      getSound() const
                                                        {
                                                            return _p_sound;
                                                        }

                //! Get sound channel
                FMOD::Channel* const                    getChannel() const
                                                        {
                                                            return _p_channel;
                                                        }

                //! Get channel group
                FMOD::ChannelGroup* const               getChannelGroup() const
                                                        {
                                                            return _p_channelGroup;
                                                        }

            protected:
                                                        SoundResource() :
                                                         _p_sound( NULL ),
                                                         _p_channel( NULL ),
                                                         _p_channelGroup( NULL )
                                                        {
                                                        }

                virtual                                 ~SoundResource()
                                                        {
                                                            if ( _p_sound )
                                                                _p_sound->release();
                                                        }

                FMOD::Sound*                            _p_sound;

                FMOD::Channel*                          _p_channel;

                FMOD::ChannelGroup*                     _p_channelGroup;

            friend class yaf3d::SoundManager;
        };

        //! Given a string for sound group returns its id ( see above ). Throws an exception passing an invalid string.
        //! Valid strings are:
        //!    Master
        //!    Common
        //!    Music
        //!    FX
        unsigned int                                getSoundGroupIdFromString( const std::string& soundgroup ) throw ( SoundException );

        //! Given a sound group ID returns its name ( the opposite of the method above ).
        std::string                                 getSoundGroupStringFromId( unsigned int soundgroup ) throw ( SoundException );

        //! Create sound belonging to sound group 'soundgroup' given a file name and creation flags. 
        //! Returns a sound ID > 0 which is used for furhter operations such as releasing the sound.
        //! Returns 0 if something went wrong, in this case also an exception is thrown.
        unsigned int                                createSound( unsigned int soundgroup, const std::string& file, float volume = 1.0f, bool autoplay = false, unsigned int flags = fmodDefaultCreationFlags2D ) throw ( SoundException );

        //! Release sound resouce given its id.
        void                                        releaseSound( unsigned int soundID ) throw ( SoundException );

        //! Return associated fmod sound resources for given sound. Use this for performing advanced operations on sound, channel, and channel group.
        SoundManager::SoundResource*                getSoundResource( unsigned int soundID ) throw ( SoundException );

        //! Play sound with given ID. Set 'paused' in order to start the sound in paused state. You have to unpause it yourself using its channel in associated 'SoundResource' ( see 'getSoundResource' method ).
        void                                        playSound( unsigned int soundID, bool paused = false );

        //! Pause sound with given ID.
        void                                        pauseSound( unsigned int soundID );

        //! Continue a paused sound with given ID.
        void                                        continueSound( unsigned int soundID );

        //! Stop sound with given ID. Next playSound call will start at begin of sound track with a new channel assigned.
        void                                        stopSound( unsigned int soundID );

        //! Set volume [ 0..1 ] for sound with given ID.
        void                                        setSoundVolume( unsigned int soundID, float volume );

        //! Mute / demute a sound given its ID.
        //! Pass en = true for muting and en = false foe demuting.
        void                                        setSoundMute( unsigned int soundID, bool en );

        //! Set volume [ 0..1 ] of given sound group. Pass SoundGroupMaster for changing the master volume of all sound groups.
        void                                        setGroupVolume( unsigned int soundgroup, float volume );

        //! Get volume [ 0..1 ] of given sound group. Pass SoundGroupMaster for getting the master volume.
        float                                       getGroupVolume( unsigned int soundgroup );

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

        //! Map type for holding < group id / sound group object >
        typedef std::map< unsigned int, FMOD::ChannelGroup* > SoundGroupMap;

        //! Sound group map
        SoundGroupMap                               _soundGroupMap;

        //! Type for sound resource map
        typedef std::map< unsigned int, SoundResource* > MapSoundResource;

        //! Sound data map for internal housekeeping
        MapSoundResource                            _mapSoundResource;

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
