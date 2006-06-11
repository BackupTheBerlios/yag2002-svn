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
#include "vrc_voiceinput.h"
#include "vrc_codec.h"
#include <fmod_errors.h>

#include "vrc_voicetestutils.h"

namespace vrc
{

BaseVoiceInput::BaseVoiceInput( FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec ) :
_p_soundSystem( p_sndsystem ),
_p_codec( p_codec ),
_p_sound( NULL ),
_p_channel( NULL ),
_inputGain( 1.0f ),
_createSoundSystem( true ),
_createCodec( true ),
_active( false ),
_lastSoundTrackPos( 0 )
{
    // remember if sound system was given for destruction phase later
    if ( p_sndsystem )
        _createSoundSystem = false;

    if ( p_codec )
        _createCodec = false;

}

BaseVoiceInput::~BaseVoiceInput()
{
    if ( _p_soundSystem && _createSoundSystem )
    {
        _p_soundSystem->close();
        _p_soundSystem->release();
        _p_soundSystem = NULL;
    }

    if ( _p_codec && _createCodec )
        delete _p_codec;
}

void BaseVoiceInput::initialize() throw( NetworkSoundExpection )
{
    // need to create a soundsystem?
    if ( !_p_soundSystem )
    {
        FMOD_RESULT result;
        result = FMOD::System_Create( &_p_soundSystem );
        if ( result != FMOD_OK )
            throw NetworkSoundExpection( "Cannot create sound system" );

        unsigned int version;
        result = _p_soundSystem->getVersion( &version );

        if ( version < FMOD_VERSION )
            throw NetworkSoundExpection( "FMOD version conflict." );
 
        //result = _p_soundSystem->setSoftwareFormat( VOICE_SAMPLE_RATE, VOICE_SOUND_FORMAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR );
        //if ( result != FMOD_OK )
        //    throw NetworkSoundExpection( "Cannot set software format" );

        // we need only one single channel in sound system
        result = _p_soundSystem->init( 1, FMOD_INIT_STREAM_FROM_UPDATE, 0 );
        if ( result != FMOD_OK )
            throw NetworkSoundExpection( "Cannot initialize sound system" );
    }

    // need for creating codec?
    if ( !_p_codec )
    {
        _p_codec = new NetworkSoundCodec;
        _p_codec->setupEncoder();
    }
}

void BaseVoiceInput::setInputGain( float gain )
{
    _inputGain = gain;
}

void BaseVoiceInput::update()
{
    assert( _p_soundSystem && "invalid sound system!" );
    _p_soundSystem->update();
}

void BaseVoiceInput::registerStreamSink( FCaptureInput* p_functor, bool reg )
{
    bool isinlist = false;
    std::vector< FCaptureInput* >::iterator p_beg = _sinks.begin(), p_end = _sinks.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_functor )
        {
            isinlist = true;
            break;
        }
    }

    // check the registration / deregistration
    assert( !( isinlist && reg ) && "stream functor is already registered" );
    assert( !( !isinlist && !reg ) && "stream functor has not been previousely registered" );

    if ( reg )
        _sinks.push_back( p_functor );
    else
        _sinks.erase( p_beg );    
}

void BaseVoiceInput::distributeSamples( void* const p_rawpointer1, void* const p_rawpointer2, unsigned int len1, unsigned int len2 )
{
    // codec's encoder modifies the sample buffer, hence we need a sample temprary buffer
    static char s_samplebuffer[ CODEC_MAX_BUFFER_SIZE ];

    if ( p_rawpointer1 && len1 )
    {
        // copy raw data to sample buffer
        memcpy( s_samplebuffer, p_rawpointer1, len1 );
    }
    if ( p_rawpointer2 && len2 )
    {
        // append the wrapped data segment to sample buffer
        char* dest = s_samplebuffer + len1;
        memcpy( dest, p_rawpointer2, len2 );
    }

    // captured new samples? if so then encode and distribute them
    if ( ( len1 + len2 ) > 0 )
    {
        VOICE_DATA_FORMAT_TYPE* p_data = reinterpret_cast< VOICE_DATA_FORMAT_TYPE* >( s_samplebuffer );
        unsigned int encodedbytes = _p_codec->encode( p_data, ( len1 + len2 ) / sizeof ( VOICE_DATA_FORMAT_TYPE ), _p_encoderbuffer, _inputGain );

        // check for network paket overrun
        assert( encodedbytes <= VOICE_PAKET_MAX_BUF_SIZE );

        // call all registered functors with fresh data
        if ( encodedbytes > 0 )
        {
            std::vector< FCaptureInput* >::iterator p_beg = _sinks.begin(), p_end = _sinks.end();
            for ( ; p_beg != p_end; ++p_beg )
                ( *p_beg )->operator ()( _p_encoderbuffer, encodedbytes );
        }
    }
}

// implementation of microphone input
VoiceMicrophoneInput::VoiceMicrophoneInput( FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec ):
BaseVoiceInput( p_sndsystem, p_codec ),
_inputDeviceReady( false )
{
}

VoiceMicrophoneInput::~VoiceMicrophoneInput()
{
    if ( _p_sound )
    {
        _p_soundSystem->recordStop();
        _p_channel->stop();
        _p_sound->release();
        _p_sound       = NULL;
        _p_channel     = NULL;
    }
}

void VoiceMicrophoneInput::initialize() throw( NetworkSoundExpection )
{
    // first init the base object
    BaseVoiceInput::initialize();

    _inputDeviceReady = false;

    // create a sound object
    FMOD_RESULT result;
    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE | FMOD_LOOP_NORMAL;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    int                     channels = 1;

    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );
    createsoundexinfo.decodebuffersize  = 0;
    createsoundexinfo.length            = VOICE_SAMPLE_RATE; // buffer for one second
    createsoundexinfo.numchannels       = channels;
    createsoundexinfo.defaultfrequency  = VOICE_SAMPLE_RATE;
    createsoundexinfo.format            = VOICE_SOUND_FORMAT;

    result = _p_soundSystem->createSound( NULL, mode, &createsoundexinfo, &_p_sound );
    if ( result != FMOD_OK )
        throw NetworkSoundExpection( "Problem occured during creation of sound object: " + std::string( FMOD_ErrorString( result ) ) );

    // get input device
    int numdrivers = 0;
    result = _p_soundSystem->getRecordNumDrivers( &numdrivers );
    if ( result != FMOD_OK )
        throw NetworkSoundExpection( "Cannot get any microphone devices: " + std::string( FMOD_ErrorString( result ) ) );

    unsigned int inputdevice;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdevice );
    if ( inputdevice == VRC_GS_VOICECHAT_INPUT_DEVICE_NA )
    {
        _inputDeviceReady = false;
    }
    // default to first device if something with setting is wrong
    else if ( static_cast< int >( inputdevice ) > numdrivers )
    {
        log_error << "VoiceMicrophoneInput: cannot set requested input device, set to default first device" << std::endl;
        inputdevice = VRC_GS_VOICECHAT_INPUT_DEVICE_NA;
    }

    result = _p_soundSystem->setRecordDriver( inputdevice );
    if ( result != FMOD_OK )
    {
        _inputDeviceReady = false;
        throw NetworkSoundExpection( "Error setting input device: " + std::string( FMOD_ErrorString( result ) ) );
    }

    _inputDeviceReady = true;
}

void VoiceMicrophoneInput::update()
{
    if ( !_active || !_inputDeviceReady )
        return;

    assert( _p_sound && "sound has not been created, first call initialize!" );

    // update base object first
    BaseVoiceInput::update();

    // get access to sound's raw data, encode and distribute
    {
        unsigned int currentSoundTrackPos;
        _p_soundSystem->getRecordPosition( &currentSoundTrackPos );

        if ( _lastSoundTrackPos != currentSoundTrackPos )        
        {
            unsigned int len1, len2;
            void* p_rawpointer1 = NULL;
            void* p_rawpointer2 = NULL;
            int blocklength     = abs( static_cast< int >( currentSoundTrackPos ) - static_cast< int >( _lastSoundTrackPos ) );

            // get access to raw sound data
            FMOD_RESULT res = _p_sound->lock( _lastSoundTrackPos * sizeof( VOICE_DATA_FORMAT_TYPE ), blocklength * sizeof( VOICE_DATA_FORMAT_TYPE ), &p_rawpointer1, &p_rawpointer2, &len1, &len2 );
            if ( res != FMOD_OK )
            {
                log_error << "VoiceMicrophoneInput: cannot lock input sample buffer" << std::endl;
                return;
            }

            // encode and distribute samples
            distributeSamples( p_rawpointer1, p_rawpointer2, len1, len2 );

            _p_sound->unlock( p_rawpointer1, p_rawpointer2, len1, len2 );
        }
        _lastSoundTrackPos = currentSoundTrackPos;
    }
}

void VoiceMicrophoneInput::stop( bool st )
{
    assert( _p_sound && "input is not initialized!" );

    if ( st )
        _p_soundSystem->recordStop();
    else
        _p_soundSystem->recordStart( _p_sound, true );

    // set active flag
    _active = !st;
}

void VoiceMicrophoneInput::setInputDevice( unsigned int deviceid )
{
    _p_soundSystem->recordStop();
    _p_soundSystem->setRecordDriver( deviceid );
    _p_soundSystem->recordStart( _p_sound, true );
}

// implementation of file input
VoiceFileInput::VoiceFileInput( const std::string& file, FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec ):
BaseVoiceInput( p_sndsystem, p_codec ),
_file( file )
{
}

VoiceFileInput::~VoiceFileInput()
{
    FMOD_RESULT result;
    if ( _p_sound )
    {
        result = _p_sound->release();
        _p_sound       = NULL;
        _p_channel     = NULL;
    }
}

void VoiceFileInput::initialize() throw( NetworkSoundExpection )
{
    // first init the base object
    BaseVoiceInput::initialize();

    FMOD_RESULT result;
    // create a sound object
    FMOD_MODE   mode = FMOD_2D | FMOD_LOOP_NORMAL | FMOD_SOFTWARE;
    result = _p_soundSystem->createSound( std::string( yaf3d::Application::get()->getMediaPath() + _file ).c_str(), mode, NULL, &_p_sound );
    if ( result != FMOD_OK )
        throw NetworkSoundExpection( "Problem occured during creation of sound object: " + std::string( FMOD_ErrorString( result ) + std::string( " : " ) + _file ) );

    _p_soundSystem->playSound( FMOD_CHANNEL_FREE, _p_sound, false, &_p_channel );
    _p_channel->setMute( true );
}

void VoiceFileInput::update()
{
    if ( !_active )
        return;

    assert( _p_sound && "sound has not been created, first call initialize!" );

    // update base object first
    BaseVoiceInput::update();

    // get access to sound's raw data, encode and distribute
    {
        unsigned int currentSoundTrackPos;
        _p_channel->getPosition( &currentSoundTrackPos, FMOD_TIMEUNIT_PCM );

        if ( _lastSoundTrackPos != currentSoundTrackPos )        
        {
            unsigned int len1, len2;
            void* p_rawpointer1 = NULL;
            void* p_rawpointer2 = NULL;
            int   blocklength   = abs( static_cast< int >( currentSoundTrackPos ) - static_cast< int >( _lastSoundTrackPos ) );

            // get access to raw sound data
            FMOD_RESULT res = _p_sound->lock( _lastSoundTrackPos * sizeof( VOICE_DATA_FORMAT_TYPE ), blocklength * sizeof( VOICE_DATA_FORMAT_TYPE ), &p_rawpointer1, &p_rawpointer2, &len1, &len2 );
            assert( res == FMOD_OK );

            // encode and distribute samples
            distributeSamples( p_rawpointer1, p_rawpointer2, len1, len2 );

            _p_sound->unlock( p_rawpointer1, p_rawpointer2, len1, len2 );
        }

        _lastSoundTrackPos = currentSoundTrackPos;
    }
}

void VoiceFileInput::stop( bool st )
{
    assert( _p_channel && "input is not initialized!" );

    if ( st )
        _p_channel->setPaused( true );
    else
        _p_channel->setPaused( false );

    _active = !st;
}

} // namespace vrc
