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
 # entity 3D sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_3dsound.h"

namespace CTD
{

//! Implement and register the platform entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( ThreeDSoundEntityFactory );


En3DSound::En3DSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_referenceDist( 70.0f ),
_rolloffFac( 4.0f ),
_showSource( false ),
_p_soundNode( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "resourceDir" , _soundFileDir   );
    getAttributeManager().addAttribute( "soundFile",    _soundFile      );
    getAttributeManager().addAttribute( "position",     _position       );
    getAttributeManager().addAttribute( "loop",         _loop           );
    getAttributeManager().addAttribute( "autoPlay",     _autoPlay       );
    getAttributeManager().addAttribute( "volume",       _volume         );
    getAttributeManager().addAttribute( "rollOff",      _rolloffFac     );
    getAttributeManager().addAttribute( "refDistance",  _referenceDist  );
    getAttributeManager().addAttribute( "showSource",   _showSource     );
}

En3DSound::~En3DSound()
{
    if ( _soundState.get() )
    {
        _soundState->setPlay( false );
        _soundState = NULL; // delete the sound object
    }
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
    catch ( const openalpp::Error& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading sound file in '" << getInstanceName() << "'" << std::endl;
        log << Log::LogLevel( Log::L_ERROR ) << "  reason: " << e.what() << std::endl;
        return;
    }
 
    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this );   

    // create a named sound state.
    // note: we have to make the state name unique as otherwise new need unique sound states for every entity instance
    std::stringstream uniquename;
    static unsigned int uniqueId = 0;
    uniquename << getInstanceName();
    uniquename << uniqueId;
    uniqueId++;
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
    // Allocate a hardware soundsource to this soundstate (priority 10)
    _soundState->allocateSource( 10, false );

    _soundState->setReferenceDistance( _referenceDist );
    _soundState->setRolloffFactor( _rolloffFac );

    _soundState->setPosition( _position );

    // Create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState.get() );

    // this is for debugging
    if ( _showSource )
    {
        osg::Node* p_mesh = LevelManager::get()->loadMesh( "sound/soundsrc.osg" );
        if ( p_mesh )
        {
            addToTransformationNode( p_mesh );
            setPosition( _position );
        }
        else
        {
            log << Log::LogLevel( Log::L_ERROR ) << "*** error loading mesh file for sound source 'sound/soundsrc.osg'" << std::endl;
        }

    }
}

void En3DSound::updateEntity( float deltaTime )
{
    if ( _showSource )
    {
        static float a = 0;
        a = ( a < 2.0f * osg::PI ) ? a + deltaTime : a - 2.0f * osg::PI + deltaTime;
        osg::Quat quat;
        quat.makeRotate( a,  osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
        setRotation( quat );
    }
}

void En3DSound::startPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( true );
}

void En3DSound::stopPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( false );
}

} // namespace CTD
