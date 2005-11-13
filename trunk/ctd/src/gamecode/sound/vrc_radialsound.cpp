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
 # entity radial sound, this sound source is distributed with a 
 #  linear attenuation in all directions
 #
 #   date of creation:  11/11/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_radialsound.h"

namespace vrc
{

class RadialSoundIH : public yaf3d::GenericInputHandler< EnRadialSound >
{
    public:

        explicit                            RadialSoundIH( EnRadialSound* p_soundentity ) :
                                             yaf3d::GenericInputHandler< EnRadialSound >( p_soundentity )
                                            {
                                            }
                                            
        virtual                             ~RadialSoundIH() {}

        //! Handle input events.
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
                                            {
                                                if ( ea.getEventType() != osgGA::GUIEventAdapter::FRAME )
                                                    return false;

                                                // calculate the attenuation depending on listener and sound source postion
                                                float x, y, z;
                                                osgAL::SoundManager::instance()->getListener()->getPosition( x, y, z );
                                                osg::Vec3f listenerpos( x, y, z );
                                                float      distance = ( listenerpos - getUserObject()->getSoundPosition() ).length();
                                                float      soundradius = getUserObject()->getSoundRadius();
                                                if ( distance > soundradius )
                                                {
                                                    getUserObject()->setDamping( 0.0f );
                                                }
                                                else
                                                {
                                                    float damping = std::max( 0.0f, 1.0f - ( distance / soundradius ) );
                                                    getUserObject()->setDamping( damping );
                                                }

                                                return false;
                                            }
};

//! Implement and register the radial sound entity factory
YAF3D_IMPL_ENTITYFACTORY( RadialSoundEntityFactory );

EnRadialSound::EnRadialSound() :
_loop( true ),
_autoPlay( true ),
_volume( 0.8f ),
_isPlaying( false ),
_wasPlaying( false ),
_p_soundNode( NULL ),
_radius( 10.0f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "soundFile",    _soundFile      );
    getAttributeManager().addAttribute( "resourceDir" , _soundFileDir   );
    getAttributeManager().addAttribute( "soundFile",    _soundFile      );
    getAttributeManager().addAttribute( "loop",         _loop           );
    getAttributeManager().addAttribute( "autoPlay",     _autoPlay       );
    getAttributeManager().addAttribute( "volume",       _volume         );
    getAttributeManager().addAttribute( "radius",       _radius         );
    getAttributeManager().addAttribute( "position",     _position       );
    getAttributeManager().addAttribute( "sourceMesh",   _sourceMesh     );
}

EnRadialSound::~EnRadialSound()
{
    if ( _soundState.get() )
    {
        _soundState->setPlay( false );
        _soundState = NULL; // delete the sound object
    }

    if ( _p_soundUpdater.valid() )
        _p_soundUpdater->destroyHandler();
}

void EnRadialSound::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle menu entring / leaving and attribute changing
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

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            // update sound from attributes
            setPosition( _position );
            if ( _soundState.valid() )
            {
                _volume = std::max( std::min( _volume, 1.0f ), 0.0f );
                _soundState->setGain( _volume );
                _soundState->setPlay( _autoPlay );
            }
        }
        break;

        default:
            ;
    }
}

void EnRadialSound::initialize()
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
    _soundState->allocateSource( 6, false );

    _soundState->apply();

    // create a sound node and attach the soundstate to it.
    _p_soundNode = new osgAL::SoundNode;
    _p_soundNode->setSoundState( _soundState.get() );

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );

    // create the sound updater instance now
    _p_soundUpdater = new RadialSoundIH( this );

    // this is for vieualizing the sound source, good for level design phase
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
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** error loading mesh file for sound source 'sound/soundsrc.osg'" << std::endl;
        }

    }
}

void EnRadialSound::startPlaying()
{
    if ( _soundState.valid() )
        _soundState->setPlay( true );
}

void EnRadialSound::stopPlaying( bool pause )
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

} // namespace vrc
