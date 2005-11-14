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

#include <ctd_main.h>
#include "ctd_playersound.h"
#include "ctd_playerimpl.h"

namespace CTD
{

// internal material names
#define SND_GROUND      "grd"
#define SND_WOOD        "wod"
#define SND_METALL      "met"
#define SND_GRASS       "grs"

//! Implement and register the player animation entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerSoundEntityFactory );

EnPlayerSound::EnPlayerSound() :
_volume( 0.8f ),
_referenceDist( 10.0f ),
_p_playerImpl( NULL )
{ 
    // register attributes
    getAttributeManager().addAttribute( "resourceDir"         , _soundFileDir  );
    getAttributeManager().addAttribute( "refDistance"         , _referenceDist );
    getAttributeManager().addAttribute( "volume"              , _volume        );
    getAttributeManager().addAttribute( "soundWalkGround"     , _walkGround    );
    getAttributeManager().addAttribute( "soundWalkWood"       , _walkWood      );
    getAttributeManager().addAttribute( "soundWalkMetal"      , _walkMetal     );
    getAttributeManager().addAttribute( "soundWalkGrass"      , _walkGrass     );
}

EnPlayerSound::~EnPlayerSound()
{
}

void EnPlayerSound::setPlayer( BasePlayerImplementation* p_player )
{
    _p_playerImpl = p_player;
}

void EnPlayerSound::postInitialize()
{
    // check if the player has already set its association
    assert( _p_playerImpl && "player implementation has to set its association in initialize phase!" );
    
    _p_soundGroup = new osg::Group;
    osgAL::SoundState* p_soundState;

    // set file search path for sound resources
    osgAL::SoundManager::instance()->addFilePath( Application::get()->getMediaPath() + _soundFileDir );

    p_soundState = createSound( _walkGround );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
       _soundStates.insert( std::make_pair( std::string( SND_GROUND ), p_soundState ) );
    }

    p_soundState = createSound( _walkWood );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
        _soundStates.insert( std::make_pair( std::string( SND_WOOD ), p_soundState ) );
    }

    p_soundState = createSound( _walkMetal );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
        _soundStates.insert( std::make_pair( std::string( SND_METALL ), p_soundState ) );
    }

    p_soundState = createSound( _walkGrass );
    if ( p_soundState )
    {
        p_soundState->setLooping( true );
        _soundStates.insert( std::make_pair( std::string( SND_GRASS ), p_soundState ) );
    }

    // add the sound group into player node
    _p_playerImpl->getPlayerEntity()->appendTransformationNode( _p_soundGroup.get() );
}

osgAL::SoundState* EnPlayerSound::createSound( const std::string& filename )
{
    openalpp::Sample* p_sample    = NULL;
    osgAL::SoundNode* p_soundNode = NULL;
    try {

        p_sample = osgAL::SoundManager::instance()->getSample( filename );
        if ( !p_sample )
            return NULL;

    } 
    catch ( const openalpp::Error& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "error loading sound file '" << filename << "' in '" << getInstanceName() << "'" << std::endl;
        log << Log::LogLevel( Log::L_ERROR ) << "  reason: " << e.what() << std::endl;
        return NULL;
    }
     
    // create a named sound state.
    // note: we have to make the state name unique as otherwise new need unique sound states for every entity instance
    std::stringstream uniquename;
    static unsigned int uniqueId = 0;
    uniquename << getInstanceName();
    uniquename << uniqueId;
    uniqueId++;
    osgAL::SoundState* p_soundState = new osgAL::SoundState( uniquename.str() );

    // Let the soundstate use the sample we just created
    p_soundState->setSample( p_sample );
    // Set its pitch to 1 (normal speed)
    p_soundState->setPitch( 1.0f );
    p_soundState->setPlay( false );
    p_soundState->setGain( std::max( std::min( _volume, 1.0f ), 0.0f ) );
    // Allocate a hardware soundsource to this soundstate (lower priority of 5)
    p_soundState->allocateSource( 5, false );

    p_soundState->setReferenceDistance( _referenceDist );
    p_soundState->setRolloffFactor( 4.0f );

    // set stopping method
    p_soundState->setStopMethod( openalpp::Stopped );

    p_soundState->setPosition( osg::Vec3f( 0, 0, 0 ) ); // relative to player

    // Create a sound node and attach the soundstate to it.
    p_soundNode = new osgAL::SoundNode;
    p_soundNode->setSoundState( p_soundState );

    // add the sound node into sound group
    _p_soundGroup->addChild( p_soundNode );

    return p_soundState;
}

void EnPlayerSound::playWalkGround()
{
    std::map< std::string, osg::ref_ptr< osgAL::SoundState > >::iterator p_state = _soundStates.find( SND_GROUND );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second.get() );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::playWalkWood()
{
    std::map< std::string, osg::ref_ptr< osgAL::SoundState > >::iterator p_state = _soundStates.find( SND_WOOD );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second.get() );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::playWalkMetal()
{
    std::map< std::string, osg::ref_ptr< osgAL::SoundState > >::iterator p_state = _soundStates.find( SND_METALL );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second.get() );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::playWalkGrass()
{
    std::map< std::string, osg::ref_ptr< osgAL::SoundState > >::iterator p_state = _soundStates.find( SND_GRASS );
    assert( p_state != _soundStates.end() );
    stopOtherSounds( p_state->second.get() );
    if ( !p_state->second->isPlaying() )
        p_state->second->setPlay( true );
}

void EnPlayerSound::stopPlayingAll()
{
    std::map< std::string, osg::ref_ptr< osgAL::SoundState > >::iterator p_beg = _soundStates.begin(), p_end = _soundStates.end();
    for ( ; p_beg != p_end; p_beg++ )
        p_beg->second->setPlay( false );
}

void EnPlayerSound::stopOtherSounds( const osgAL::SoundState* p_state )
{
    std::map< std::string, osg::ref_ptr< osgAL::SoundState > >::iterator p_beg = _soundStates.begin(), p_end = _soundStates.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( p_beg->second.get() != p_state ) 
            p_beg->second->setPlay( false );
}

} // namespace CTD
