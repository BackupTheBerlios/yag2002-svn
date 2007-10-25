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

#include <base.h>
#include <fmod_errors.h>
#include "soundmanager.h"
#include "log.h"

//! Max sound sources ( software mixed )
#define SOUND_MAX_SOFTWARE_CHANNELS     100
#define SOUND_MAX_VIRTUAL_SOURCES       100

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( SoundManager )

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
    log_info << "SoundManager: shutting down" << std::endl;

    // shutdown fmod
    if ( _p_system )
    {
        // release sound channel groups
        FMOD::ChannelGroup* p_soundgroup = NULL;
        p_soundgroup = _soundGroupMap[ SoundGroupCommon ];
        p_soundgroup->release();

        p_soundgroup = _soundGroupMap[ SoundGroupMusic ];
        p_soundgroup->release();

        p_soundgroup = _soundGroupMap[ SoundGroupFX ];
        p_soundgroup->release();

        _p_system->close();
        _p_system->release();
    }

    _p_system       = NULL;
    _soundIDCounter = 0;

    if ( _mapSoundResource.size() )
        log_warning << "*** SoundManager::shutdown: " << _mapSoundResource.size() << " sound sources were not released before shutdown!" << std::endl;

    // destroy singleton
    destroy();
}

void SoundManager::update( float /*deltaTime*/ )
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
    assert( ( _p_system == NULL ) && "sound system is already initialized, shutdown first!" );

    FMOD_RESULT result;

    // create sound system
    result = FMOD::System_Create( &_p_system );
    if ( result != FMOD_OK )
        throw SoundException( "Problem creating sound system, reason: " + std::string( FMOD_ErrorString( result ) ) );

    // check fmod version
    unsigned int version;
    result = _p_system->getVersion( &version );
    if ( version < FMOD_VERSION )
    {
        std::stringstream str;
        str << "SoundManager: detected fmod version " << version << ", this application needs at least version " << FMOD_VERSION;
        throw SoundException( str.str() );
    }

    // init software channels
    result = _p_system->setSoftwareChannels( SOUND_MAX_SOFTWARE_CHANNELS );
    if ( result != FMOD_OK )
        throw SoundException( "Problem allocating required software mixed channels, reason: " + std::string( FMOD_ErrorString( result ) ) );

    // try to init hardware channels
    result = _p_system->setHardwareChannels( 30, 62, 30, 62 );
    if ( result != FMOD_OK )
    {
        result = _p_system->setHardwareChannels( 14, 30, 14, 30 );
        if ( result != FMOD_OK )
            log_warning << "*** SoundManager: cannot allocate at least 16 hardware sound channels" << std::endl;
    }

    result = _p_system->init( SOUND_MAX_VIRTUAL_SOURCES, FMOD_INIT_NORMAL, 0 );
    if ( result != FMOD_OK )
        throw SoundException( "Problem initializing sound system, reason: " + std::string( FMOD_ErrorString( result ) ) );

    FMOD::ChannelGroup* p_mastergroup;
    result = _p_system->getMasterChannelGroup( &p_mastergroup );
    if ( result != FMOD_OK )
        throw SoundException( "Problem getting main sound group" );

    // put the master sound group into map
    _soundGroupMap[ SoundGroupMaster ] = p_mastergroup;

    // setup common sound group
    {
        FMOD::ChannelGroup* p_groupCommon;
        result = _p_system->createChannelGroup( "SoundGroupCommon", &p_groupCommon );
        if ( result != FMOD_OK )
            throw SoundException( "Problem creating common sound group" );

        result = p_mastergroup->addGroup( p_groupCommon );
        if ( result != FMOD_OK )
            throw SoundException( "Problem adding common sound group" );

        _soundGroupMap[ SoundGroupCommon ] = p_groupCommon;
    }
    // setup music sound group
    {
        FMOD::ChannelGroup* p_groupMusic;
        result = _p_system->createChannelGroup( "SoundGroupMusic", &p_groupMusic );
        if ( result != FMOD_OK )
            throw SoundException( "Problem creating music sound group" );

        result = p_mastergroup->addGroup( p_groupMusic );
        if ( result != FMOD_OK )
            throw SoundException( "Problem adding music sound group" );

        _soundGroupMap[ SoundGroupMusic ] = p_groupMusic;
    }
    // setup fx sound group
    {
        FMOD::ChannelGroup* p_groupFX;
        result = _p_system->createChannelGroup( "SoundGroupFX", &p_groupFX );
        if ( result != FMOD_OK )
            throw SoundException( "Problem creating fx sound group" );

        result = p_mastergroup->addGroup( p_groupFX );
        if ( result != FMOD_OK )
            throw SoundException( "Problem adding fx sound group" );

        _soundGroupMap[ SoundGroupFX ] = p_groupFX;
    }
}

unsigned int SoundManager::getSoundGroupIdFromString( const std::string& soundgroup ) throw ( SoundException )
{
    if ( soundgroup == "Master" )
        return SoundGroupMaster;
    else if ( soundgroup == "Common" )
        return SoundGroupCommon;
    else if ( soundgroup == "Music" )
        return SoundGroupMusic;
    else if ( soundgroup == "FX" )
        return SoundGroupFX;

    throw SoundException( "Invalid sound group name: " + soundgroup );
    return 0;
}

std::string SoundManager::getSoundGroupStringFromId( unsigned int soundgroup ) throw ( SoundException )
{
    if ( soundgroup == SoundGroupMaster )
        return "Master";
    else if ( soundgroup == SoundGroupCommon )
        return "Common";
    else if ( soundgroup == SoundGroupMusic )
        return "Music";
    else if ( soundgroup == SoundGroupFX )
        return "FX";

    std::stringstream str;
    str << "Invalid sound group id: " <<  soundgroup;
    throw SoundException( str.str() );

    return 0;
}

unsigned int SoundManager::createSound( unsigned int soundgroup, const std::string& file, float volume, bool autoplay, unsigned int flags ) throw ( SoundException )
{
    // check the given group id
    FMOD::ChannelGroup* p_soundgroup;
    if ( _soundGroupMap.find( soundgroup ) == _soundGroupMap.end() )
        throw SoundException( "Invalid sound group for creating sound for: " + file );

    p_soundgroup = _soundGroupMap[ soundgroup ];

    FMOD_RESULT result;

    SoundResource* p_resource   = new SoundResource;
    p_resource->_p_sound        = NULL;
    p_resource->_p_channel      = NULL;
    p_resource->_p_channelGroup = NULL;

    // create sound
    result = _p_system->createSound( std::string( Application::get()->getMediaPath() + file ).c_str(), flags, 0, &p_resource->_p_sound );
    if ( result != FMOD_OK )
    {
        delete p_resource;
        throw SoundException( "Could not create sound for " + file + ", reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    result = _p_system->playSound( FMOD_CHANNEL_FREE, p_resource->_p_sound, true, &p_resource->_p_channel );
    if ( result != FMOD_OK )
    {
        delete p_resource;
        throw SoundException( "Could not create sound for " + file + ", problem pausing, reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    float tmpvol = std::max( std::min( volume, 1.0f ), 0.0f );
    if ( fabs( tmpvol - volume ) > 0.01f )
        log_warning << "SoundManager, createSound: volume must be in range [0..1], clamped to " << tmpvol << std::endl;

    // limit the volume range to 0..1
    volume = tmpvol;
	result = p_resource->_p_channel->setVolume( volume );
    if ( result != FMOD_OK )
    {
        delete p_resource;
        throw SoundException( "could not create sound for " + file + ", problem setting volume, reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    // set the channel group
    p_resource->_p_channelGroup = p_soundgroup;
    p_resource->_p_channel->setChannelGroup( p_soundgroup );
    if ( result != FMOD_OK )
    {
        delete p_resource;
        throw SoundException( "could not set requested channel group: " + getSoundGroupStringFromId( soundgroup ) + " for " + file + ", reason: " + std::string( FMOD_ErrorString( result ) ) );
    }

    // store the created sound in internal map
    _soundIDCounter++;
    _mapSoundResource[ _soundIDCounter ] = p_resource;

    if ( autoplay )
        playSound( _soundIDCounter );

    log_verbose << "SoundManager: created sound '" << file << "', id: " << _soundIDCounter << ", group: " << getSoundGroupStringFromId( soundgroup ) << std::endl;

    return _soundIDCounter;
}

void SoundManager::releaseSound( unsigned int soundID ) throw ( SoundException )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
    {
        std::stringstream str;
        str << "sound with given ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    // release sound
    delete p_soundresource->second;

    // remove sound from internal map
    _mapSoundResource.erase( p_soundresource );

    log_verbose << "SoundManager: releasing sound, id '" << soundID << "'" << std::endl;
}


SoundManager::SoundResource* SoundManager::getSoundResource( unsigned int soundID ) throw ( SoundException )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
    {
        std::stringstream str;
        str << "sound data with given sound ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    SoundResource* p_resource = p_soundresource->second;
    return p_resource;
}

void SoundManager::playSound( unsigned int soundID, bool paused )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
        return;

    SoundResource* p_resource = p_soundresource->second;

    _p_system->playSound( FMOD_CHANNEL_REUSE, p_resource->_p_sound, true, &p_resource->_p_channel );

    // re-set the channel group, fmod seems to lose the channel group when start playing
    p_resource->_p_channel->setChannelGroup( p_resource->_p_channelGroup );

    if ( !paused )
        p_resource->_p_channel->setPaused( false );
}

void SoundManager::pauseSound( unsigned int soundID )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
        return;

    SoundResource* p_resource = p_soundresource->second;
    p_resource->_p_channel->setPaused( true );
}

void SoundManager::continueSound( unsigned int soundID )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
        return;

    SoundResource* p_resource = p_soundresource->second;
    p_resource->_p_channel->setPaused( false );
}

void SoundManager::stopSound( unsigned int soundID )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
        return;

    SoundResource* p_resource = p_soundresource->second;
    p_resource->_p_channel->stop();
}

void SoundManager::setSoundVolume( unsigned int soundID, float volume )
{
    if ( ( volume > 1.0f ) || ( volume < 0.0f ) )
        throw SoundException( "Volume out of range, it must be in range 0..1" );

    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
    {
        std::stringstream str;
        str << "sound with given ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    p_soundresource->second->_p_channel->setVolume( volume );
}

void SoundManager::setSoundMute( unsigned int soundID, bool en )
{
    MapSoundResource::iterator p_soundresource = _mapSoundResource.find( soundID );
    if ( p_soundresource == _mapSoundResource.end() )
    {
        std::stringstream str;
        str << "sound with given ID does not exist: " << soundID;
        throw SoundException( str.str() );
    }

    p_soundresource->second->_p_channel->setMute( en );
}

void SoundManager::setGroupVolume( unsigned int soundgroup, float volume )
{
    if ( ( volume > 1.0f ) || ( volume < 0.0f ) )
        throw SoundException( "Volume out of range, it must be in range 0..1" );

    // first check the given group id
    if ( _soundGroupMap.find( soundgroup ) == _soundGroupMap.end() )
        throw SoundException( "Invalid sound group" );

    FMOD::ChannelGroup* p_soundgroup = _soundGroupMap[ soundgroup ];
    p_soundgroup->setVolume( volume );
}

float SoundManager::getGroupVolume( unsigned int soundgroup )
{
    // first check the given group id
    if ( _soundGroupMap.find( soundgroup ) == _soundGroupMap.end() )
        throw SoundException( "Invalid sound group" );

    FMOD::ChannelGroup* p_soundgroup = _soundGroupMap[ soundgroup ];
    float volume;
    p_soundgroup->getVolume( &volume );
    return volume;
}

} // namespace yaf3d
