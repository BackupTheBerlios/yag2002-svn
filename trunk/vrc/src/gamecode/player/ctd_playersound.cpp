/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
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

#include <ctd_base.h>
#include <ctd_physics.h>
#include <ctd_log.h>
#include <ctd_application.h>
#include "ctd_playersound.h"
#include "ctd_player.h"

using namespace osg;
using namespace std;

namespace CTD
{

//! Implement and register the player animation entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerSoundEntityFactory );

EnPlayerSound::EnPlayerSound() :
_p_player( NULL ),
_volume( 0.8f ),
_referenceDist( 10.0f ),
_p_soundGroup( NULL )
{ 
    // the deletion must not be controled by entity manager, but by player
    setAutoDelete( false );

    // register attributes
    getAttributeManager().addAttribute( "resourcedir"         , _soundFileDir  );
    getAttributeManager().addAttribute( "refdistance"         , _referenceDist );
    getAttributeManager().addAttribute( "volume"              , _volume        );
    getAttributeManager().addAttribute( "soundwalkground"     , _walkGround    );
    getAttributeManager().addAttribute( "soundwalkwood"       , _walkWood      );
    getAttributeManager().addAttribute( "soundwalkmetal"      , _walkMetal     );
    getAttributeManager().addAttribute( "soundwalkgrass"      , _walkGrass     );

    // this entity does not need a periodic update
    activate( false );
}

EnPlayerSound::~EnPlayerSound()
{
}

void EnPlayerSound::setPlayer( EnPlayer* p_player )
{
    _p_player = p_player;
}

void EnPlayerSound::postInitialize()
{
    // check if the player has already set its association
    assert( _p_player && "player entitiy has to set its association in initialize phase!" );
    
    _p_soundGroup = new osg::Group;
    osgAL::SoundState* p_soundState;

    // set file search path for sound resources
    osgAL::SoundManager::instance()->addFilePath( Application::get()->getMediaPath() + _soundFileDir );

    p_soundState = createSound( _walkGround );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
       _soundStates.insert( make_pair( string( "walkgrd" ), p_soundState ) );
    }

    p_soundState = createSound( _walkWood );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
        _soundStates.insert( make_pair( string( "walkwd" ), p_soundState ) );
    }

    p_soundState = createSound( _walkMetal );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
        _soundStates.insert( make_pair( "walkmet", p_soundState ) );
    }

    p_soundState = createSound( _walkGrass );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
        _soundStates.insert( make_pair( "walkgrass", p_soundState ) );
    }

    // add the sound group into player node
    _p_player->addToTransformationNode( _p_soundGroup );
}

void EnPlayerSound::destroy()
{
}

osgAL::SoundState* EnPlayerSound::createSound( const string& filename )
{
    openalpp::Sample* p_sample    = NULL;
    osgAL::SoundNode* p_soundNode = NULL;
    try {

        p_sample = osgAL::SoundManager::instance()->getSample( filename, false );
        if ( !p_sample )
            return NULL;

    } 
    catch ( openalpp::Error error )
    {
        cout << "*** error loading sound file '" << filename << "'" << endl;
        return NULL;
    }
     
    // Create a named sound state
    string statename = _p_player->getInstanceName() + "_snd_" + filename;
    osgAL::SoundState* p_soundState = new osgAL::SoundState( statename );
    // Let the soundstate use the sample we just created
    p_soundState->setSample( p_sample );
    // Set its pitch to 1 (normal speed)
    p_soundState->setPitch( 1 );
    p_soundState->setPlay( false );
    p_soundState->setGain( max( min( _volume, 1.0f ), 0 ) );
    // Allocate a hardware soundsource to this soundstate (lower priority of 5)
    p_soundState->allocateSource( 5, false );

    p_soundState->setReferenceDistance( _referenceDist );
    p_soundState->setRolloffFactor( 4.0f );

    // set stopping method
    p_soundState->setStopMethod( openalpp::Stopped );

    p_soundState->setPosition( Vec3f( 0, 0, 0 ) ); // relative to player

    p_soundState->apply();

    //! Note: DO NOT add the soundstate to the sound manager! this causes problems with same filenames in its cache!
    // nono: osgAL::SoundManager::instance()->addSoundState( p_soundState );
    // finding out this subtile beast took 1 hour :-(

    // Create a sound node and attach the soundstate to it.
    p_soundNode = new osgAL::SoundNode;
    p_soundNode->setSoundState( p_soundState );

    // add the sound node into sound group
    _p_soundGroup->addChild( p_soundNode );

    return p_soundState;
}

void EnPlayerSound::playWalkGround()
{
    std::map< std::string, osgAL::SoundState* >::iterator p_state = _soundStates.find( "walkgrd" );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::playWalkWood()
{
    std::map< std::string, osgAL::SoundState* >::iterator p_state = _soundStates.find( "walkwd" );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::playWalkMetal()
{
    std::map< std::string, osgAL::SoundState* >::iterator p_state = _soundStates.find( "walkmet" );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::playWalkGrass()
{
    std::map< std::string, osgAL::SoundState* >::iterator p_state = _soundStates.find( "walkgrass" );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::stopPlayingAll()
{
    std::map< std::string, osgAL::SoundState* >::iterator p_beg = _soundStates.begin(), p_end = _soundStates.end();
    for ( ; p_beg != p_end; p_beg++ )
        p_beg->second->setPlay( false );
}

void EnPlayerSound::stopOtherSounds( osgAL::SoundState* p_state )
{
    std::map< std::string, osgAL::SoundState* >::iterator p_beg = _soundStates.begin(), p_end = _soundStates.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( p_beg->second != p_state ) 
            p_beg->second->setPlay( false );
}

} // namespace CTD
