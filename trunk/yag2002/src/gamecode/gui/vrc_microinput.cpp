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

bool MicrophoneInput::initialize()
{
    FMOD_RESULT result;

    // need to create a soundsystem?
    if ( _p_soundSystem )
        return false;

    result = FMOD::System_Create( &_p_soundSystem );
    if ( result != FMOD_OK )
        return false;

    unsigned int version;
    result = _p_soundSystem->getVersion( &version );

    if ( version < FMOD_VERSION )
        return false;

    // we need only one single channel in sound system
    result = _p_soundSystem->init( 1, FMOD_INIT_NORMAL, 0 );
    if ( result != FMOD_OK )
        return false;

    return true;
}

void MicrophoneInput::getInputDevices( MicrophoneInput::InputDeviceMap& devices )
{
    assert( _p_soundSystem && "MicrophoneInput has not been initialized!" );

    FMOD_RESULT result;
    int numdrivers = 0;
    result = _p_soundSystem->getRecordNumDrivers( &numdrivers );
    if ( result != FMOD_OK )
        return;

    int inputdevice = -1;
    for ( int count = 0; count < numdrivers; ++count )
    {
        char name[ 256 ];
        result = _p_soundSystem->getRecordDriverName( count, name, 256 );
        if ( result != FMOD_OK )
            log_error << "MicrophoneInput: error getting input device name" << std::endl;
        else
        {
            devices[ count ] = std::string( name );
        }
    }
}

void MicrophoneInput::setInputDevice( unsigned int deviceid )
{
    assert( _p_soundSystem && "MicrophoneInput has not been initialized!" );

    if ( _microTestRunning )
        _p_soundSystem->recordStop();

    _p_soundSystem->setRecordDriver( deviceid );

    if ( _microTestRunning )
        _p_soundSystem->recordStart( _p_sound, true );
}

void MicrophoneInput::setInputGain( float gain )
{
    assert( _p_channel && "MicrophoneInput: micro test is already running!" );

    // limit the gain to [ 0 ... 2 ]
    _inputGain = std::min( std::max( 0.0f, gain ), 2.0f );
    // we simulate separate input / output gains here
    float totalgain = std::min( std::max( 0.0f, _inputGain * _outputGain ), 1.0f );
    _p_channel->setVolume( totalgain );
}

void MicrophoneInput::setOutputGain( float gain )
{
    assert( _p_channel && "MicrophoneInput: micro test is already running!" );

    // limit the gain to [ 0 ... 2 ]
    _outputGain = std::min( std::max( 0.0f, gain ), 2.0f );
    // we simulate separate input / output gains here
    float totalgain = std::min( std::max( 0.0f, _inputGain * _outputGain ), 1.0f );
    _p_channel->setVolume( totalgain );
}

void MicrophoneInput::beginMicroTest()
{
    assert( ( _p_sound == NULL ) && "MicrophoneInput: micro test is already running!" );

    _microTestRunning = true;

    FMOD_RESULT result;

    // create a sound object
    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE | FMOD_LOOP_NORMAL;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    int                     channels = 1;

    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );
    createsoundexinfo.decodebuffersize  = 0;
    createsoundexinfo.length            = MICINPUT_SAMPLE_RATE * sizeof( short ) * 0.5f; // buffer for 0.5 seconds recording 
    createsoundexinfo.numchannels       = channels;
    createsoundexinfo.defaultfrequency  = MICINPUT_SAMPLE_RATE;
    createsoundexinfo.format            = MICINPUT_SOUND_FORMAT;

    result = _p_soundSystem->createSound( NULL, mode, &createsoundexinfo, &_p_sound );
    if ( result != FMOD_OK )
        return;

    _p_soundSystem->playSound( FMOD_CHANNEL_FREE, _p_sound, false, &_p_channel );

    // start recording
    _p_soundSystem->recordStart( _p_sound, true );

    // start the sound update thread
    start();
}

void MicrophoneInput::endMicroTest()
{
    // stop recording
    _p_soundSystem->recordStop();

    // try to kill the update thread if it is running
    if ( isRunning() )
    {
        cancel();
        // wait until the thread has been shut down
        while( isRunning() );
    }

    if ( _p_sound )
    {
        _p_sound->release();
        _p_sound = NULL;
    }

    _microTestRunning = false;
}

void MicrophoneInput::run()
{
    while ( !testCancel() )
    {
        _p_soundSystem->update();
    }
}

} // namespace vrc