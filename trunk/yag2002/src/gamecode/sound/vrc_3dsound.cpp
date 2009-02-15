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
 # entity 3D sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_3dsound.h"

namespace vrc
{

//! Implement and register the 3d sound entity factory
YAF3D_IMPL_ENTITYFACTORY( ThreeDSoundEntityFactory )


En3DSound::En3DSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_soundGroup( "Music" ),
_minDistance( 1.0f ),
_maxDistance( 15.0f ),
_soundID( 0 ),
_p_channel( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "soundFile",        _soundFile   );
    getAttributeManager().addAttribute( "soundGroup",       _soundGroup  );
    getAttributeManager().addAttribute( "position",         _position    );
    getAttributeManager().addAttribute( "loop",             _loop        );
    getAttributeManager().addAttribute( "autoPlay",         _autoPlay    );
    getAttributeManager().addAttribute( "volume",           _volume      );
    getAttributeManager().addAttribute( "minDistance",      _minDistance );
    getAttributeManager().addAttribute( "maxDistance",      _maxDistance );
    getAttributeManager().addAttribute( "meshFile",         _sourceMesh  );
}

En3DSound::~En3DSound()
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
            log_error << ENTITY_NAME_3DSOUND << ":" << getInstanceName() << " problem releasing sound, reason: " << e.what() << std::endl;
        }
    }
}

void En3DSound::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle menu entring / leaving
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {   
            stopPlaying( true );
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            // respond to sound setting changes in menu only if the sound type is not Common
            if ( yaf3d::SoundManager::get()->getSoundGroupIdFromString( _soundGroup ) != yaf3d::SoundManager::SoundGroupCommon )
                setupSound();

            startPlaying( true );
        }
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

        default:
            ;
    }
}

void En3DSound::initialize()
{
    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
    setupSound();
}

void En3DSound::setupSound()
{
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
        log_error << ENTITY_NAME_3DSOUND << ":" << getInstanceName() << " invalid sound group type" << std::endl;

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
                    log_error << ENTITY_NAME_3DSOUND << ":" << getInstanceName() << " problem releasing sound, reason: " << e.what() << std::endl;
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
            flags = yaf3d::SoundManager::fmodDefaultCreationFlags3DLoop;
        else
            flags = yaf3d::SoundManager::fmodDefaultCreationFlags3D;

        _soundID    = yaf3d::SoundManager::get()->createSound( soundgroup, _soundFile, _volume, _autoPlay, flags );
        _p_channel  = yaf3d::SoundManager::get()->getSoundResource( _soundID )->getChannel();

        setVolume( _volume );

        // set position and velocity
        FMOD_VECTOR pos;
        pos.x = _position.x();
        pos.y = _position.y();
        pos.z = _position.z();
        _p_channel->set3DAttributes( &pos, NULL );
        _p_channel->set3DMinMaxDistance( _minDistance, _maxDistance );
    } 
    catch ( const yaf3d::SoundException& e )
    {
        log_error << ENTITY_NAME_3DSOUND << ":" << getInstanceName() << "  error loading sound file " << _soundFile << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
    }

    // re-add the debug mesh if one exists
    if ( getTransformationNode()->getNumChildren() )
    {
        // first remove the current mesh
        osg::Node* p_currmesh = getTransformationNode()->getChild( 0 );
        if ( p_currmesh )
            removeFromTransformationNode( p_currmesh );
    }
    if ( _sourceMesh.length() )
    {
        osg::Node* p_mesh = yaf3d::LevelManager::get()->loadMesh( _sourceMesh );
        if ( p_mesh )
        {
            addToTransformationNode( p_mesh );
            setPosition( _position );
        }
        else
        {
            log_warning << "2DSound: error loading mesh file for sound source: " << _sourceMesh << std::endl;
        }
    }
}

void En3DSound::startPlaying( bool cont )
{
    if ( _soundID > 0 )
    {
        if ( cont )
            yaf3d::SoundManager::get()->continueSound( _soundID );
        else
            yaf3d::SoundManager::get()->playSound( _soundID );
    }
}

void En3DSound::stopPlaying( bool pause )
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

void En3DSound::setVolume( float volume )
{
    if ( _soundID > 0 )
    {
        _volume = std::max( std::min( volume, 1.0f ), 0.0f );
        _p_channel->setVolume( _volume );
    }
}

float En3DSound::getVolume()
{
    return _volume;
}

} // namespace vrc
