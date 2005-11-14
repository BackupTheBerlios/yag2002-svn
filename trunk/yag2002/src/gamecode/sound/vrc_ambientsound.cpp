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
_isPlaying( false ),
_wasPlaying( false ),
_p_soundNode( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "resourceDir" , _soundFileDir   );
    getAttributeManager().addAttribute( "soundFile",    _soundFile      );
    getAttributeManager().addAttribute( "loop",         _loop           );
    getAttributeManager().addAttribute( "autoPlay",     _autoPlay       );
    getAttributeManager().addAttribute( "volume",       _volume         );
}

EnAmbientSound::~EnAmbientSound()
{
    if ( _soundState.get() )
    {
        _soundState->setPlay( false );
        _soundState = NULL; // delete the sound object
    }
}

void EnAmbientSound::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle menu entring / leaving
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {   
            if ( _isPlaying )
                stopPlaying( true );

            _wasPlaying = _isPlaying;
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _wasPlaying )
                startPlaying();
        }
        break;

        default:
            ;
    }
}

void EnAmbientSound::initialize()
{
    osgAL::SoundManager::instance()->addFilePath( yaf3d::Application::get()->getMediaPath() + _soundFileDir );

    openalpp::Sample* p_sample = NULL;
    try {

        p_sample = osgAL::SoundManager::instance()->getSample( _soundFile );
        if ( !p_sample )
        {
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_WARNING ) << "*** cannot create sampler for '" << _soundFileDir + _soundFile << "'" << std::endl;
            return;
        }

    } 
    catch ( const openalpp::Error& e )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** error loading sound file" << std::endl;
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "  reason: " << e.what() << std::endl;
        return;
    }

    // create a named sound state.
    // note: we have to make the state name unique as otherwise new sound states with already defined names make problems
    std::stringstream uniquename;
    static unsigned int uniqueId = 0;
    uniquename << getInstanceName();
    uniquename << uniqueId;
    uniqueId++;
    _soundState = new osgAL::SoundState( uniquename.str() );
    // let the soundstate use the sample we just created
    _soundState->setSample( p_sample );
    _soundState->setGain( std::max( std::min( _volume, 1.0f ), 0.0f ) );
    // set its pitch to 1 (normal speed)
    _soundState->setPitch( 1.0f );

    // make it play
    _soundState->setPlay( _autoPlay );
    _isPlaying = _autoPlay;

    // the sound should loop over and over again
    _soundState->setLooping( _loop );
    _soundState->setAmbient( true );
    // allocate a hardware soundsource to this soundstate (priority 10)
    _soundState->allocateSource( 10, false );

    _soundState->setReferenceDistance( 50.0f );
    _soundState->setRolloffFactor( 5.0f );

    _soundState->apply();

    // create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState.get() );

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void EnAmbientSound::startPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( true );
}

void EnAmbientSound::stopPlaying( bool pause )
{
    if ( _soundState.valid() )
    {
        // first set stop mode
        if ( pause )
        {
            _soundState->setStopMethod( openalpp::Paused );
        }
        else
        {
            _soundState->setStopMethod( openalpp::Stopped );
            _isPlaying = false;
        }

        _soundState->setPlay( false );
    }
}

//! Set sound volume (0..1)
void EnAmbientSound::setVolume( float volume )
{
    _volume = std::max( std::min( volume, 1.0f ), 0.0f );
    if ( _soundState.get() )
        _soundState->setGain( _volume );
}

float EnAmbientSound::getVolume()
{
    return _volume;
}

} // namespace vrc
