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

#include <vrc_main.h>
#include "vrc_3dsound.h"

namespace vrc
{

//! Implement and register the 3d sound entity factory
YAF3D_IMPL_ENTITYFACTORY( ThreeDSoundEntityFactory );


En3DSound::En3DSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_referenceDist( 70.0f ),
_rolloffFac( 1.0f ),
_isPlaying( false ),
_wasPlaying( false ),
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
    getAttributeManager().addAttribute( "sourceMesh",   _sourceMesh     );
}

En3DSound::~En3DSound()
{
    if ( _soundState.get() )
    {
        _soundState->setPlay( false );
        _soundState = NULL; // delete the sound object
    }
}

void En3DSound::handleNotification( const yaf3d::EntityNotification& notification )
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

void En3DSound::initialize()
{
    // set file search path for sound resources
    osgAL::SoundManager::instance()->addFilePath( yaf3d::Application::get()->getMediaPath() + _soundFileDir );

    openalpp::Sample* p_sample = NULL;
    try {

        p_sample = osgAL::SoundManager::instance()->getSample( _soundFile );
        if ( !p_sample )
            return;

    } 
    catch ( const openalpp::Error& e )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** error loading sound file in '" << getInstanceName() << "'" << std::endl;
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "  reason: " << e.what() << std::endl;
        return;
    }
 
    // create a named sound state.
    // note: we have to make the state name unique as otherwise new need unique sound states for every entity instance
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
    _soundState->setPitch( 1 );
    
    // automatically start playing?
    _soundState->setPlay( _autoPlay );
    _isPlaying = _autoPlay;

    // the sound should loop over and over again
    _soundState->setLooping( _loop );
    // allocate a hardware soundsource to this soundstate (priority 10)
    _soundState->allocateSource( 10, false );

    // to make a radial 3d sound source we first have to make the source ambient then add rolloff
    _soundState->setAmbient( true );
    _soundState->setRolloffFactor( _rolloffFac );
    _soundState->setReferenceDistance( _referenceDist );

    _soundState->setPosition( _position );

    // create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState.get() );

    // this is for debugging
    if ( _sourceMesh.length() )
    {
        osg::Node* p_mesh = yaf3d::LevelManager::get()->loadMesh( _sourceMesh );
        if ( p_mesh )
        {
            addToTransformationNode( p_mesh );
            setPosition( _position );
            // register entity in order to get updated per simulation step
            yaf3d::EntityManager::get()->registerUpdate( this );   
        }
        else
        {
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** error loading mesh file for sound source 'sound/soundsrc.osg'" << std::endl;
        }
    }

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void En3DSound::updateEntity( float deltaTime )
{
    static float a = 0;
    a = ( a < 2.0f * osg::PI ) ? a + deltaTime : a - 2.0f * osg::PI + deltaTime;
    osg::Quat quat;
    quat.makeRotate( a,  osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
    setRotation( quat );
}

void En3DSound::startPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( true );

    _isPlaying = true;
}

void En3DSound::stopPlaying( bool pause )
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
void En3DSound::setVolume( float volume )
{
    _volume = std::max( std::min( volume, 1.0f ), 0.0f );
    if ( _soundState.get() )
        _soundState->setGain( _volume );
}

float En3DSound::getVolume()
{
    return _volume;
}

} // namespace vrc
