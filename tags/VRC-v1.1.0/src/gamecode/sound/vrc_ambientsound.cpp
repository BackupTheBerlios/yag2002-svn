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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_ambientsound.h"

namespace vrc
{

//! Implement and register the ambient sound entity factory
YAF3D_IMPL_ENTITYFACTORY( AmbientSoundEntityFactory );


EnAmbientSound::EnAmbientSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_soundID( 0 ),
_p_channel( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "soundFile",    _soundFile );
    getAttributeManager().addAttribute( "loop",         _loop      );
    getAttributeManager().addAttribute( "autoPlay",     _autoPlay  );
    getAttributeManager().addAttribute( "volume",       _volume    );
}

EnAmbientSound::~EnAmbientSound()
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
            log_error << ENTITY_NAME_AMBIENTSOUND << ":" << getInstanceName() << " problem releasing sound, reason: " << e.what() << std::endl;
        }
    }
}

void EnAmbientSound::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle menu entring / leaving
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

            stopPlaying( true );
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            startPlaying();
            break;

        default:
            ;
    }
}

void EnAmbientSound::initialize()
{
    try 
    {
        unsigned int flags = 0;
        if ( _loop )
            flags = yaf3d::SoundManager::fmodDefaultCreationFlags2DLoop;
        else
            flags = yaf3d::SoundManager::fmodDefaultCreationFlags2D;

        _soundID    = yaf3d::SoundManager::get()->createSound( _soundFile, _volume, _autoPlay, flags );
        _p_channel  = yaf3d::SoundManager::get()->getSoundChannel( _soundID );
    } 
    catch ( const yaf3d::SoundException& e )
    {
        log_error << ENTITY_NAME_AMBIENTSOUND << ":" << getInstanceName() << "  error loading sound file " << _soundFile << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
    }
}

void EnAmbientSound::startPlaying( bool cont )
{
    if ( _soundID > 0 )
    {
        if ( cont )
            yaf3d::SoundManager::get()->continueSound( _soundID );
        else
            yaf3d::SoundManager::get()->playSound( _soundID );
    }
}

void EnAmbientSound::stopPlaying( bool pause )
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

//! Set sound volume (0..1)
void EnAmbientSound::setVolume( float volume )
{
    if ( _soundID > 0 )
    {
        _volume = std::max( std::min( volume, 1.0f ), 0.0f );
        _p_channel->setVolume( _volume );
    }    
}

float EnAmbientSound::getVolume()
{
    return _volume;
}

} // namespace vrc
