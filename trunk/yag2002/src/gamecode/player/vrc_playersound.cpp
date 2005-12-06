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
 ################################################################*/

#include <vrc_main.h>
#include "vrc_playersound.h"
#include "vrc_playerimpl.h"

namespace vrc
{

// internal material names
#define SND_GROUND      1
#define SND_WOOD        2
#define SND_METALL      3
#define SND_GRASS       4

//! Implement and register the player animation entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerSoundEntityFactory );

EnPlayerSound::EnPlayerSound() :
_volume( 0.8f ),
_minDistance( 1.0f ),
_maxDistance( 15.0f ),
_offset( osg::Vec3f( 0.0f, 0.0f, -1.0f ) ),
_p_playerImpl( NULL )
{ 
    // register attributes
    getAttributeManager().addAttribute( "positionOffset"      , _offset      );
    getAttributeManager().addAttribute( "minDistance"         , _minDistance );
    getAttributeManager().addAttribute( "maxDistance"         , _maxDistance );
    getAttributeManager().addAttribute( "volume"              , _volume      );
    getAttributeManager().addAttribute( "soundWalkGround"     , _walkGround  );
    getAttributeManager().addAttribute( "soundWalkWood"       , _walkWood    );
    getAttributeManager().addAttribute( "soundWalkMetal"      , _walkMetal   );
    getAttributeManager().addAttribute( "soundWalkGrass"      , _walkGrass   );
}

EnPlayerSound::~EnPlayerSound()
{
    // release sounds
    MapPlayerSounds::iterator p_beg = _mapSounds.begin(), p_end = _mapSounds.end();
    for ( ; p_beg != p_end; ++p_beg )
        yaf3d::SoundManager::get()->releaseSound( p_beg->second );
}

void EnPlayerSound::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle attribute changing
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            _volume = std::max( std::min( _volume, 1.0f ), 0.0f );
        }
        break;

        default:
            ;
    }
}

void EnPlayerSound::setPlayer( BasePlayerImplementation* p_player )
{
    _p_playerImpl = p_player;
}

void EnPlayerSound::postInitialize()
{
    // check if the player has already set its association
    assert( _p_playerImpl && "player implementation has to set its association in initialize phase!" );

    unsigned int soundID = 0;

    soundID = createSound( _walkGround );
    if ( soundID > 0 )
        _mapSounds.insert( std::make_pair( SND_GROUND, soundID ) );

    soundID = createSound( _walkWood );
    if ( soundID > 0 )
        _mapSounds.insert( std::make_pair( SND_WOOD, soundID ) );

    soundID = createSound( _walkMetal );
    if ( soundID > 0 )
        _mapSounds.insert( std::make_pair( SND_METALL, soundID ) );

    soundID = createSound( _walkGrass );
    if ( soundID > 0 )
        _mapSounds.insert( std::make_pair( SND_GRASS, soundID ) );

    // register entity for getting notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

unsigned int EnPlayerSound::createSound( const std::string& filename )
{
    unsigned int soundID = 0;
    try 
    {
        float volume = std::max( std::min( _volume, 1.0f ), 0.0f );
        soundID      = yaf3d::SoundManager::get()->createSound( filename, volume, false, yaf3d::SoundManager::fmodDefaultCreationFlags3D );
        FMOD::Channel* p_channel = yaf3d::SoundManager::get()->getSoundChannel( soundID );
        p_channel->set3DMinMaxDistance( _minDistance, _maxDistance );
    } 
    catch ( const yaf3d::SoundExpection& e )
    {
        log_error << ENTITY_NAME_PLSOUND << ":" << getInstanceName() << "  error loading sound file " << filename << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
        return 0;
    }

    return soundID;    
}

void EnPlayerSound::playWalkGround()
{
    MapPlayerSounds::iterator p_soundID = _mapSounds.find( SND_GROUND );
    if ( p_soundID == _mapSounds.end() )
       return;

    yaf3d::SoundManager::get()->playSound( p_soundID->second, true );
}

void EnPlayerSound::playWalkWood()
{
    MapPlayerSounds::iterator p_soundID = _mapSounds.find( SND_WOOD );
    if ( p_soundID == _mapSounds.end() )
       return;

    yaf3d::SoundManager::get()->playSound( p_soundID->second, true );
}

void EnPlayerSound::playWalkMetal()
{
    MapPlayerSounds::iterator p_soundID = _mapSounds.find( SND_METALL );
    if ( p_soundID == _mapSounds.end() )
       return;

    yaf3d::SoundManager::get()->playSound( p_soundID->second, true );
}

void EnPlayerSound::playWalkGrass()
{
    MapPlayerSounds::iterator p_soundID = _mapSounds.find( SND_GRASS );
    if ( p_soundID == _mapSounds.end() )
       return;

    yaf3d::SoundManager::get()->playSound( p_soundID->second, true );
}

void EnPlayerSound::updatePosition( const osg::Vec3f& pos )
{
    FMOD_VECTOR soundpos;
    osg::Vec3f  destpos = pos - _offset;
    soundpos.x = destpos.x();
    soundpos.y = destpos.y();
    soundpos.z = destpos.z();

    FMOD::Channel* p_channel;
    MapPlayerSounds::iterator p_beg = _mapSounds.begin(), p_end = _mapSounds.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        p_channel = yaf3d::SoundManager::get()->getSoundChannel( p_beg->second );
        p_channel->set3DAttributes( &soundpos, NULL );
    }
}

void EnPlayerSound::stopPlayingAll()
{
    // currently nothing to do as we have non-looped sounds
}

} // namespace vrc
