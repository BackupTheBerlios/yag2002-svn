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

#include <ctd_main.h>
#include "ctd_ambientsound.h"

using namespace std;
using namespace osg; 

namespace CTD
{

//! Implement and register the platform entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( AmbientSoundEntityFactory );


EnAmbientSound::EnAmbientSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_p_soundNode( NULL ),
_soundState( NULL )
{
    // register entity attributes
    _attributeManager.addAttribute( "resourceDir" , _soundFileDir   );
    _attributeManager.addAttribute( "soundFile",    _soundFile      );
    _attributeManager.addAttribute( "loop",         _loop           );
    _attributeManager.addAttribute( "autoPlay",     _autoPlay       );
    _attributeManager.addAttribute( "volume",       _volume         );
}

EnAmbientSound::~EnAmbientSound()
{
    if ( _soundState.valid() )
        _soundState->setPlay( false );
}

void EnAmbientSound::initialize()
{
    osgAL::SoundManager::instance()->addFilePath( Application::get()->getMediaPath() + _soundFileDir );

    openalpp::Sample* p_sample = NULL;
    try {

        p_sample = osgAL::SoundManager::instance()->getSample( _soundFile );
        if ( !p_sample )
        {
            log << Log::LogLevel( Log::L_WARNING ) << "*** cannot create sampler for '" << _soundFileDir + _soundFile << "'" << endl;
            return;
        }

    } 
    catch ( openalpp::Error error )
    {
        cout << "*** error loading sound file" << endl;
        return;
    }

    // create a named sound state.
    // note: we have to make the state name unique as otherwise new sound states with already defined names make problems
    stringstream uniquename;
    static uniqueId = 0;
    uniquename << getInstanceName();
    uniquename << uniqueId;
    uniqueId++;
    string s = uniquename.str();
    _soundState = new osgAL::SoundState( uniquename.str() );
    // Let the soundstate use the sample we just created
    _soundState->setSample( p_sample );
    _soundState->setGain( std::max( std::min( _volume, 1.0f ), 0.0f ) );
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
//    osgAL::SoundManager::instance()->addSoundState( _soundState.get() );

    // Create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState.get() );
}

void EnAmbientSound::startPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( true );
}

void EnAmbientSound::stopPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( false );
}

} // namespace CTD
