/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_levelmanager.h>
#include "ctd_ambientsound.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the platform entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( AmbientSoundEntityFactory );


EnAmientSound::EnAmientSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_p_soundNode( NULL ),
_soundState( NULL )
{
    //! Note: this entity needs no periodic updating

    // register entity attributes
    _attributeManager.addAttribute( "resourcedir" , _soundFileDir   );
    _attributeManager.addAttribute( "soundfile",    _soundFile      );
    _attributeManager.addAttribute( "loop",         _loop           );
    _attributeManager.addAttribute( "autoplay",     _autoPlay       );
    _attributeManager.addAttribute( "volume",       _volume         );

    // this entity does not need a periodic update
    activate( false );
}

EnAmientSound::~EnAmientSound()
{
    _soundState->setPlay( false );
}

void EnAmientSound::initialize()
{
    osgAL::SoundManager::instance()->addFilePath( Application::get()->getMediaPath() + _soundFileDir );

    openalpp::Sample* p_sample = NULL;
    try {

        p_sample = osgAL::SoundManager::instance()->getSample( _soundFile );
        if ( !p_sample )
            return;

    } 
    catch ( openalpp::Error error )
    {
        cout << "*** error loading sound file" << endl;
        return;
    }

    // Create a named sound state.
    _soundState = new osgAL::SoundState( getInstanceName() );
    // Let the soundstate use the sample we just created
    _soundState->setSample( p_sample );
    _soundState->setGain( max( min( _volume, 1.0f ), 0 ) );
    // Set its pitch to 1 (normal speed)
    _soundState->setPitch( 1 );
    // Make it play
    _soundState->setPlay( _autoPlay );
    // The sound should loop over and over again
    _soundState->setLooping( _loop );
    _soundState->setAmbient( true );
    // Allocate a hardware soundsource to this soundstate (priority 10)
    _soundState->allocateSource( 10, false );

    _soundState->setReferenceDistance( 50.0f );
    _soundState->setRolloffFactor( 5 );

    Vec3f pos;
    osgAL::SoundManager::instance()->getListener()->getPosition( pos._v[ 0 ], pos._v[ 1 ], pos._v[ 2 ] );
    _soundState->setPosition( pos );

    _soundState->apply();

    // Add the soundstate to the sound manager, so we can find it later on if we want to
    osgAL::SoundManager::instance()->addSoundState( _soundState );

    // Create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState );
}

void EnAmientSound::startPlaying()
{
    assert( _soundState );
    _soundState->setPlay( true );
}

void EnAmientSound::stopPlaying()
{
    assert( _soundState );
    _soundState->setPlay( false );
}
