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
 # entity 3D sound
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
#include "ctd_3dsound.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the platform entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( ThreeDSoundEntityFactory );


En3DSound::En3DSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_rolloffFac( 4.0f ),
_referenceDist( 70.0f ),
_showSource( false ),
_p_soundNode( NULL ),
_soundState( NULL )
{
    EntityManager::get()->registerUpdate( this );   // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "resourcedir" , _soundFileDir   );
    _attributeManager.addAttribute( "soundfile",    _soundFile      );
    _attributeManager.addAttribute( "position",     _position       );
    _attributeManager.addAttribute( "loop",         _loop           );
    _attributeManager.addAttribute( "autoplay",     _autoPlay       );
    _attributeManager.addAttribute( "volume",       _volume         );
    _attributeManager.addAttribute( "rolloff",      _rolloffFac     );
    _attributeManager.addAttribute( "refdistance",  _referenceDist  );
    _attributeManager.addAttribute( "showsource",   _showSource     );
}

En3DSound::~En3DSound()
{
    _soundState->setPlay( false );

    // deregister entity, it is not necessary for entities which 'die' at application exit time
    //  as the entity manager clears the entity list on app exit
    EntityManager::get()->registerUpdate( this, false );
}

void En3DSound::initialize()
{
    // set file search path for sound resources
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
        activate( false );
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
    // Allocate a hardware soundsource to this soundstate (priority 10)
    _soundState->allocateSource( 10, false );

    _soundState->setReferenceDistance( _referenceDist );
    _soundState->setRolloffFactor( _rolloffFac );

    _soundState->setPosition( _position );

    _soundState->apply();

    // Add the soundstate to the sound manager, so we can find it later on if we want to
    osgAL::SoundManager::instance()->addSoundState( _soundState );

    // Create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState );

    // this is for debugging
    if ( _showSource )
    {
        osg::Node* p_mesh = LevelManager::get()->loadMesh( "sound/soundsrc.osg" );
        if ( p_mesh )
        {
            addToTransformableNode( p_mesh );
            setPosition( _position );
        } else
            cout << "*** error loading mesh file for sound source 'sound/soundsrc.osg'" << endl;

    }
}

void En3DSound::updateEntity( float deltaTime )
{
    if ( _showSource )
    {
        static float a = 0;
        a = ( a < 2.0f * PI ) ? a + deltaTime : a - 2.0f * PI + deltaTime;
        osg::Quat quat;
        quat.makeRotate( a,  Vec3f( 0, 0, 1 ) );
        setRotation( quat );
    }
}

void En3DSound::startPlaying()
{
    assert( _soundState );
    _soundState->setPlay( true );
}

void En3DSound::stopPlaying()
{
    assert( _soundState );
    _soundState->setPlay( false );
}
