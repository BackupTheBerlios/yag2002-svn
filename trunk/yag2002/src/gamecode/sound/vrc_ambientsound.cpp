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
 # entity ambient sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #   04/16/2008         changed EnAmbientSound entity to En2DSound
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_2dsound.h"

namespace vrc
{

//! Implement and register the 2d sound entity factory
YAF3D_IMPL_ENTITYFACTORY( TwoDSoundEntityFactory )

//! Attenuation update period for non-ambient sound
#define SND_UPDATE_ATTN_PERIOD  0.1f


En2DSound::En2DSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_ambient( true ),
_minDistance( 0.0f ),
_maxDistance( 100.0f ),
_attenuation( 1.0f ),
_updateTimer( 0.0f ),
_soundGroup( "Music" ),
_soundID( 0 ),
_p_channel( NULL ),
_p_player( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "soundFile",    _soundFile   );
    getAttributeManager().addAttribute( "soundGroup",   _soundGroup  );
    getAttributeManager().addAttribute( "loop",         _loop        );
    getAttributeManager().addAttribute( "autoPlay",     _autoPlay    );
    getAttributeManager().addAttribute( "volume",       _volume      );
    getAttributeManager().addAttribute( "ambient",      _ambient     );
    getAttributeManager().addAttribute( "position",     _position    );
    getAttributeManager().addAttribute( "minDistance",  _minDistance );
    getAttributeManager().addAttribute( "maxDistance",  _maxDistance );
}

En2DSound::~En2DSound()
{
    // release sound
    if ( _soundID > 0 )
    {
        try
        {
            yaf3d::SoundManager::get()->releaseSound( _soundID );
        }
        catch ( const yaf3d::SoundException& e )
        {
            log_error << ENTITY_NAME_2DSOUND << ":" << getInstanceName() << " problem releasing sound, reason: " << e.what() << std::endl;
        }
    }
}

void En2DSound::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle menu entring / leaving
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

            stopPlaying( true );
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            // respond to sound setting changes in menu only if the sound type is not Common
            if ( yaf3d::SoundManager::get()->getSoundGroupIdFromString( _soundGroup ) != yaf3d::SoundManager::SoundGroupCommon )
                setupSound();

            startPlaying( true );
            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            // recreate sound if any attribute has been changed
            if (  _soundID > 0 )
            {
                try
                {
                    stopPlaying( false );
                    yaf3d::SoundManager::get()->releaseSound( _soundID );
                }
                catch ( const yaf3d::SoundException& )
                {
                }
            }
            setupSound();
        }
        break;

        case YAF3D_NOTIFY_UNLOAD_LEVEL:
        {
            // nothing to do atm
        }
        break;

        default:
            ;
    }
}

void En2DSound::initialize()
{
    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );

    // setup the sound
    setupSound();

    // if the sound is ambient then we need no update
    if ( !_ambient && _soundID )
        yaf3d::EntityManager::get()->registerUpdate( this, true );
}

void En2DSound::updateEntity( float deltaTime )
{
    // we don't need to recalculate the attenuation every frame
    _updateTimer += deltaTime;
    if ( _updateTimer > SND_UPDATE_ATTN_PERIOD )
    {
        _updateTimer -= SND_UPDATE_ATTN_PERIOD;
    }
    else
    {
        return;
    }

    // we need a valid player object for calculating the attenuation of non-ambient sound
    if ( !_p_player )
    {
        _p_player = gameutils::PlayerUtils::get()->getLocalPlayer();
        if ( !_p_player )
            return;
    }

    const osg::Vec3f& playerpos = _p_player->getPosition();
    float             distance  = ( playerpos - _position ).length();
    if ( ( distance < _minDistance ) || ( distance > _maxDistance ) )
    {
        _attenuation = 0.0f;
    }
    else
    {
        _attenuation = 1.0f - ( ( distance - _minDistance ) / ( _maxDistance - _minDistance ) );
    }

    yaf3d::SoundManager::get()->setSoundVolume( _soundID, _attenuation * _volume );
}

void En2DSound::setupSound()
{
    // check for ambient sound
    if ( !_ambient )
    {
        if ( _minDistance > _maxDistance )
        {
            log_warning << ENTITY_NAME_2DSOUND << ": min distance > max distance for a non-ambient sound, fallback to ambient sound!" << std::endl;
            _ambient = true;
        }
    }

    // check if sound is enabled in menu system
    bool skiptypecheck = false;
    unsigned int soundgroup = yaf3d::SoundManager::get()->getSoundGroupIdFromString( _soundGroup );
    bool sndenable;
    std::string settingskey;
    if ( soundgroup == yaf3d::SoundManager::SoundGroupMusic )
        settingskey = VRC_GS_MUSIC_ENABLE;
    else if ( soundgroup == yaf3d::SoundManager::SoundGroupFX )
        settingskey = VRC_GS_FX_ENABLE;
    else if ( soundgroup == yaf3d::SoundManager::SoundGroupCommon )
        skiptypecheck = true;
    else
        log_error << ENTITY_NAME_2DSOUND << ":" << getInstanceName() << " invalid sound group type" << std::endl;


    // sound group Common is always created
    if ( !skiptypecheck )
    {
        yaf3d::Configuration::get()->getSettingValue( settingskey, sndenable );
        if ( _soundID > 0 )
        {
            // destroy sound if it exists but even disabled in menu
            if ( !sndenable )
            {
                try
                {
                    yaf3d::SoundManager::get()->releaseSound( _soundID );
                    _soundID = 0;
                }
                catch ( const yaf3d::SoundException& e )
                {
                    log_error << ENTITY_NAME_2DSOUND << ":" << getInstanceName() << " problem releasing sound, reason: " << e.what() << std::endl;
                }
                return;
            }
            else return;
        }
        // no sound exists and no sound enabling in menu? then we have nothing to do
        else if ( !sndenable )
            return;
    }
    else if ( _soundID > 0 )
        return;

    try 
    {
        unsigned int flags = 0;
        if ( _loop )
            flags = yaf3d::SoundManager::fmodDefaultCreationFlags2DLoop;
        else
            flags = yaf3d::SoundManager::fmodDefaultCreationFlags2D;

        _soundID    = yaf3d::SoundManager::get()->createSound( soundgroup, _soundFile, _volume, _autoPlay, flags );
        _p_channel  = yaf3d::SoundManager::get()->getSoundResource( _soundID )->getChannel();
    } 
    catch ( const yaf3d::SoundException& e )
    {
        log_error << ENTITY_NAME_2DSOUND << ":" << getInstanceName() << "  error loading sound file " << _soundFile << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
    }
}

void En2DSound::startPlaying( bool cont )
{
    if ( _soundID > 0 )
    {
        if ( cont )
            yaf3d::SoundManager::get()->continueSound( _soundID );
        else
            yaf3d::SoundManager::get()->playSound( _soundID );
    }
}

void En2DSound::stopPlaying( bool pause )
{
    if ( _soundID > 0 )
    {
        if ( pause )
        {
            yaf3d::SoundManager::get()->pauseSound( _soundID );
        }
        else
        {
            yaf3d::SoundManager::get()->stopSound( _soundID );
        }
    }
}

void En2DSound::setVolume( float volume )
{
    if ( _soundID > 0 )
    {
        _volume = std::max( std::min( volume, 1.0f ), 0.0f );
        _p_channel->setVolume( _volume );
    }    
}

float En2DSound::getVolume()
{
    return _volume;
}

} // namespace vrc
