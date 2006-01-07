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
 # sound manager using fmod -- http://www.fmod.org
 #
 #   date of creation:  12/03/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include <fmod_errors.h>
#include "soundmanager.h"
#include "log.h"

//! Max sound sources ( software mixed )
#define SOUND_MAX_SOFTWARE_CHANNELS     100
#define SOUND_MAX_VIRTUAL_SOURCES       100

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( SoundManager );

SoundManager::SoundManager() :
_p_system( NULL ),
_soundIDCounter( 0 ),
_p_sceneView( NULL )
{
    memset( &_listenerPosition, 0, 3 * sizeof( float ) );
    memset( &_listenerForward,  0, 3 * sizeof( float ) );
    memset( &_listenerUp,       0, 3 * sizeof( float ) );
}

SoundManager::~SoundManager()
{
    if ( _p_system )
        log_warning << "*** SoundManager: the manager was not shutdown before destruction!" << std::endl;
}

void SoundManager::shutdown()
{
    // shutdown fmod
    if ( _p_system )
    {
        _p_system->close();
        _p_system->release();
    }

    _p_system       = NULL;
    _soundIDCounter = 0;

    if ( _mapSoundData.size() )
        log_warning << "*** SoundManager::shutdown: " << _mapSoundData.size() << " sound sources were not released before shutdown!" << std::endl;

    // destroy singleton
    destroy();
}

void SoundManager::update( float deltaTime )
{
    assert( _p_system && "sound system is not initialized" );

    if ( !_p_sceneView )
        _p_sceneView = Application::get()->getSceneView();

    // determine listener's position, forward, and up vectors
    osg::Vec3 eye;
    osg::Vec3 cforward;
    osg::Vec3 up;

    _p_sceneView->getViewMatrixAsLookAt( eye, cforward, up );
    cforward = eye - cforward;

    _listenerPosition.x = eye.x();
    _listenerPosition.y = eye.y();
    _listenerPosition.z = eye.z();
    _listenerForward.x  = cforward.x();
    _listenerForward.y  = cforward.y();
    _listenerForward.z  = cforward.z();
    _listenerUp.x       = up.x();
    _listenerUp.y       = up.y();
    _listenerUp.z       = up.z();

    // update listener, without velocity
    _p_system->set3DListenerAttributes( 0, &_listenerPosition, NULL, &_listenerForward, &_listenerUp );

    // update sound system
    _p_system->update();
}

void SoundManager::initialize() throw ( SoundException )
{
    FMOD_RESULT result;

    // create sound system
    result = FMOD::System_Create( &_p_system );
    if ( result != FMOD_OK )
        throw SoundException( "problem creating sound system, reason: " + std::string( FMOD_ErrorString( result ) ) );

    // check fmod version
    unsigned int version;
    result = _p_system->getVersion( &version );
    if ( version < FMOD_VERSION )
    {
        std::stringstream str;
        str << "*** SoundManager: detected fmod version " << version << ", this application needs at least version " << FMOD_VERSION;
        throw SoundException( str.str() );
    }

    // init software channels
    result = _p_system->setSoftwareChannels( SOUND_MAX_SOFTWARE_CHANNELS );
    if ( result != FMOD_OK )
        throw SoundException( "problem allocating required software mixed channels, reason: " + std::string( FMOD_ErrorString( result ) ) );

    // try to init hardware channels
    result = _p_system->setHardwareChannels( 32, 64, 32, 64 );
    if ( result != FMOD_OK )
        result = _p_system->setHardwareChannels( 16, 32, 16, 32 );
            if ( result != FMOD_OK )
                log_warning << "*** SoundManager: cannot allocate at least 16 hardware sound channels" << std::endl;


    result = _p_system->init( SOUND_MAX_VIRTUAL_SOURCES, FMOD_INIT_NORMAL, 0 );
    if ( result != FMOD_OK )
        throw SoundException( "problem initializing sound system, reason: " + std::string( FMOD_ErrorString( result ) ) );

}

unsigned int SoundManager::createSound( const std::string& file, float volume, bool autoplay, unsigned int flags ) throw ( SoundException )
{
    FMOD_RESULT result;

    SoundData* p_soundData  = new SoundData;
    p_soundData->_p_sound   = NULL;
    p_soundData->_p_channel = NULL;

    result = _p_system->createSound( std::string( Application::get()->getMediaPath() + file ).c_str(), flags, 0, &p_soundData->_p_sound );
    if ( result != FMOD_OK )
    {
        delete p_soundData;
        throw SoundException( "could not create sound for " + file + ", reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    result = _p_system->playSound( FMOD_CHANNEL_FREE, p_soundData->_p_sound, true, &p_soundData->_p_channel );
    if ( result != FMOD_OK )
    {
        delete p_soundData;
        throw SoundException( "could not create sound for " + file + ", problem pausing, reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    float tmpvol = std::max( std::min( volume, 1.0f ), 0.0f );
    if ( fabs( tmpvol - volume ) > 0.01f )
        log_warning << "SoundManager, createSound: volume must be in range [0..1], clamped to " << tmpvol << std::endl;

    result = p_soundData->_p_channel->setPaused( true );
    if ( result != FMOD_OK )
    {
        delete p_soundData;
        throw SoundException( "could not create sound for " + file + ", problem setting pause mode, reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    volume = tmpvol;
	result = p_soundData->_p_channel->setVolume( volume );
    if ( result != FMOD_OK )
    {
        delete p_soundData;
        throw SoundException( "could not create sound for " + file + ", problem setting volume, reason: " + std::string( FMOD_ErrorString( result ) ) );
    }    

    // store the created sound in internal map
    _soundIDCounter++;
    _mapSoundData[ _soundIDCounter ] = p_soundData;

    if ( autoplay )
        playSound( _soundIDCounter );

    log_debug << "SoundManager: created sound '" << file << "', id " << _soundIDCounter << std::endl;

    return _soundIDCounter;
}

void SoundManager::releaseSound( unsigned int soundID ) throw ( SoundException )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
    {
        std::stringstream str;
        str << "sound with given ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    // release sound
    SoundData* p_data = p_sounddata->second;
    p_data->_p_sound->release();
    // remove sound from internal map
    _mapSoundData.erase( p_sounddata );
}

FMOD::Channel* SoundManager::getSoundChannel( unsigned int soundID ) throw ( SoundException )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
    {
        std::stringstream str;
        str << "sound with given ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    SoundData* p_data = p_sounddata->second;
    return p_data->_p_channel;
}

void SoundManager::playSound( unsigned int soundID, bool autocheck )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
        return;

    SoundData* p_data = p_sounddata->second;

    // if autocheck is enabled then don't play the sound if it is already playing
    if ( autocheck )
    {
        bool isplaying = false;
        p_data->_p_channel->isPlaying( &isplaying );
        if ( isplaying )
            return;
    }

    p_data->_p_channel->setPaused( false );
    _p_system->playSound( FMOD_CHANNEL_REUSE, p_data->_p_sound, false, &p_data->_p_channel );
}

void SoundManager::pauseSound( unsigned int soundID )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
        return;

    SoundData* p_data = p_sounddata->second;
    p_data->_p_channel->setPaused( true );
}

void SoundManager::continueSound( unsigned int soundID )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
        return;

    SoundData* p_data = p_sounddata->second;
    p_data->_p_channel->setPaused( false );
}

void SoundManager::stopSound( unsigned int soundID )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
        return;

    SoundData* p_data = p_sounddata->second;
    p_data->_p_channel->stop();
}

void SoundManager::setSoundVolume( unsigned int soundID, float volume )
{
    MapSoundData::iterator p_sounddata = _mapSoundData.find( soundID );
    if ( p_sounddata == _mapSoundData.end() )
    {
        std::stringstream str;
        str << "sound with given ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    SoundData* p_data = p_sounddata->second;

    float tmpvol = std::max( std::min( volume, 1.0f ), 0.0f );
    if ( fabs( tmpvol - volume ) > 0.01f )
        log_warning << "*** SoundManager::setSoundVolume: volume must be in range [0..1], clamped to " << tmpvol << std::endl;

    volume = tmpvol;
    p_data->_p_channel->setVolume( volume );
}

} // namespace yaf3d
