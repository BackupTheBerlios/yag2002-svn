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
 # voice input, support for microphone and wave file for tests
 #
 #   date of creation:  02/19/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_microinput.h"
#include <fmod_errors.h>

namespace vrc
{

#define MICINPUT_SAMPLE_RATE        16000
#define MICINPUT_SOUND_FORMAT       FMOD_SOUND_FORMAT_PCM16

MicrophoneInput::MicrophoneInput() :
_p_soundSystem( NULL ),
_p_sound( NULL ),
_p_channel( NULL ),
_microTestRunning( false ),
_inputGain( 1.0f ),
_outputGain( 1.0f )
{
}

MicrophoneInput::~MicrophoneInput()
{
    // just for the case that the user forgot to end a running micro test
    if ( _microTestRunning )
        endMicroTest();

    FMOD_RESULT result;
    if ( _p_soundSystem )
    {
        result = _p_soundSystem->release();
    }
}

bool MicrophoneInput::getInputDevices( MicrophoneInput::InputDeviceMap& devices )
{
    FMOD_RESULT result;

    FMOD::System* p_system;
    result = FMOD::System_Create( &p_system );
    if ( result != FMOD_OK )
    {
        log_error << "+gui MicrophoneInput: cannot create sound system!" << std::endl;
        return false;
    }

    unsigned int version;
    result = p_system->getVersion( &version );
    if ( version < FMOD_VERSION )
    {
        log_error << "+gui MicrophoneInput: wrong fmod version!" << std::endl;
        return false;
    }

#ifdef LINUX
    result = _p_soundSystem->setOutput( FMOD_OUTPUTTYPE_ALSA );
    if ( result != FMOD_OK )
    {
        log_error << "Mirophone input cannot use ALSA driver" << std::endl;
        return false;
    }
    result = _p_soundSystem->setSoftwareChannels( 1 );
    if ( result != FMOD_OK )
    {
        log_error << "Mirophone cannot allocate software mixing channel" << std::endl;
        return false;
    }
    result = _p_soundSystem->setHardwareChannels( 0, 0, 0, 0 );
    if ( result != FMOD_OK )
    {
        log_error << "Mirophone input cannot disable hardware channel mixing" << std::endl;
        return false;
    }
#endif

    // we need only one single channel in sound system
    result = p_system->init( 1, FMOD_INIT_NORMAL, 0 );
    if ( result != FMOD_OK )
    {
        log_error << "+gui MicrophoneInput: cannot initialize sound system" << std::endl;
        return false;
    }

    int numdrivers = 0;
    result = p_system->getRecordNumDrivers( &numdrivers );
    if ( result != FMOD_OK )
        return false;

    for ( int count = 0; count < numdrivers; ++count )
    {
        char name[ 256 ];
        result = p_system->getRecordDriverName( count, name, 256 );
        if ( result != FMOD_OK )
            log_error << "MicrophoneInput: error getting input device name" << std::endl;
        else
            devices[ count ] = std::string( name );
    }

    p_system->release();

    return true;
}

bool MicrophoneInput::setupInputCapturing( unsigned int deviceid )
{
    assert( ( _p_soundSystem == NULL ) && "MicrophoneInput: sound system already created!" );

    FMOD_RESULT result;

    result = FMOD::System_Create( &_p_soundSystem );
    if ( result != FMOD_OK )
    {
        log_error << "gui MicrophoneInput: cannot create sound system!" << std::endl;
        return false;
    }

    unsigned int version;
    result = _p_soundSystem->getVersion( &version );
    if ( version < FMOD_VERSION )
    {
        log_error << "gui MicrophoneInput: wrong fmod version!" << std::endl;
        return false;
    }

    _p_soundSystem->setRecordDriver( deviceid );

    // we need only one single channel in sound system
    result = _p_soundSystem->init( 1, FMOD_INIT_NORMAL, 0 );
    if ( result != FMOD_OK )
    {
        log_error << "gui MicrophoneInput: cannot initialize sound system" << std::endl;
        return false;
    }

    return true;
}

bool MicrophoneInput::setInputDevice( unsigned int deviceid )
{
    if ( _microTestRunning )
    {
        endMicroTest();

        if ( !setupInputCapturing( deviceid ) )
            return false;
    
        beginMicroTest();

        return true;
    }

    return setupInputCapturing( deviceid );
}

void MicrophoneInput::setInputGain( float gain )
{
    // limit the gain to [ 0 ... 2 ]
    _inputGain = std::min( std::max( 0.0f, gain ), 2.0f );
    
    // we simulate separate input / output gains here
    if ( _p_channel )
    {
        float totalgain = std::min( std::max( 0.0f, _inputGain * _outputGain ), 1.0f );
        _p_channel->setVolume( totalgain );
    }
}

void MicrophoneInput::setOutputGain( float gain )
{
    // limit the gain to [ 0 ... 2 ]
    _outputGain = std::min( std::max( 0.0f, gain ), 2.0f );

    if ( _p_channel )
    {
        // we simulate separate input / output gains here
        float totalgain = std::min( std::max( 0.0f, _inputGain * _outputGain ), 1.0f );
        _p_channel->setVolume( totalgain );
    }
}

void MicrophoneInput::beginMicroTest()
{
    assert( _p_soundSystem && "MicrophoneInput: first set the input device using 'setInputDevice'!" );

    if ( _microTestRunning )
        return;

    _microTestRunning = true;

    FMOD_RESULT result;

    // create a sound object
    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE | FMOD_LOOP_NORMAL;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    int                     channels = 1;

    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );
    createsoundexinfo.decodebuffersize  = 0;
    createsoundexinfo.length            = MICINPUT_SAMPLE_RATE * sizeof( short ); // buffer for 1 second recording 
    createsoundexinfo.numchannels       = channels;
    createsoundexinfo.defaultfrequency  = MICINPUT_SAMPLE_RATE;
    createsoundexinfo.format            = MICINPUT_SOUND_FORMAT;

    result = _p_soundSystem->createSound( NULL, mode, &createsoundexinfo, &_p_sound );
    if ( result != FMOD_OK )
    {
        std::stringstream msg;
        msg << result;
        log_error << "cannot set micro input device, error code: " << msg.str() << std::endl;
        return;
    }

    _p_soundSystem->playSound( FMOD_CHANNEL_FREE, _p_sound, false, &_p_channel );

    // start recording
    _p_soundSystem->recordStart( _p_sound, true );
}

void MicrophoneInput::endMicroTest()
{
    if ( !_microTestRunning )
        return;

    // stop recording
    _p_soundSystem->recordStop();
    _p_soundSystem->release();
    _p_soundSystem = NULL;
    _p_channel     = NULL;
    _p_sound       = NULL;

    _microTestRunning = false;
}

} // namespace vrc
