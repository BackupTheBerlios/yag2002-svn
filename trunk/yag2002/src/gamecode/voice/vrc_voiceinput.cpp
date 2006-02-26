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

namespace vrc
{

BaseVoiceInput::BaseVoiceInput( FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec ) :
_p_soundSystem( p_sndsystem ),
_p_codec( p_codec ),
_p_sound( NULL ),
_p_channel( NULL ),
_createSoundSystem( true ),
_createCodec( true )
{
    // remember if sound system was given for destruction phase later
    if ( p_sndsystem )
        _createSoundSystem = false;

    if ( p_codec )
        _createCodec = false;
}

BaseVoiceInput::~BaseVoiceInput()
{
    FMOD_RESULT result;
    if ( _p_soundSystem && _createSoundSystem )
    {
        result = _p_soundSystem->release();
        _p_soundSystem = NULL;
    }

    if ( _p_codec && _createCodec )
        delete _p_codec;
}

void BaseVoiceInput::initialize() throw( NetworkSoundExpection )
{
    FMOD_RESULT result;

    // need to create a soundsystem?
    if ( !_p_soundSystem )
    {
        result = FMOD::System_Create( &_p_soundSystem );
        if ( result != FMOD_OK )
            throw NetworkSoundExpection( "Cannot create sound system" );

        unsigned int version;
        result = _p_soundSystem->getVersion( &version );

        if ( version < FMOD_VERSION )
            throw NetworkSoundExpection( "FMOD version conflict." );
 
        result = _p_soundSystem->init( 32, FMOD_INIT_NORMAL, 0 );
    }

    // need for creating codec?
    if ( !_p_codec )
    {
        _p_codec = new NetworkSoundCodec;
        _p_codec->setupEncoder();
    }
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

// implementation of microphone input
VoiceMicrophoneInput::VoiceMicrophoneInput( FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec ):
BaseVoiceInput( p_sndsystem, p_codec ),
_lastSoundTrackPos( 0 )
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

    // create a sound object
    FMOD_RESULT result;
    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    int                     channels = 1;

    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );
    createsoundexinfo.decodebuffersize  = 0;
    createsoundexinfo.length            = VOICE_SAMPLE_RATE * sizeof( short ) * 2; // buffer for 2 seconds recording 
    createsoundexinfo.numchannels       = channels;
    createsoundexinfo.defaultfrequency  = VOICE_SAMPLE_RATE;
    createsoundexinfo.format            = VOICE_SOUND_FORMAT;

    result = _p_soundSystem->createSound( NULL, mode, &createsoundexinfo, &_p_sound );
    if ( result != FMOD_OK )
    {
        std::stringstream exptext;
        exptext << "Problem occured during creation of sound object, error code: " << result;
        throw NetworkSoundExpection( exptext.str() );
    }

    // get input device
    int numdrivers = 0;
    result = _p_soundSystem->getRecordNumDrivers( &numdrivers );
    if ( result != FMOD_OK )
    {
        std::stringstream exptext;
        exptext << "Cannot get any microphone devices, error code: " << result;
        throw NetworkSoundExpection( exptext.str() );
    }
    int inputdevice = -1;
    for ( int count = 0; count < numdrivers; ++count )
    {
        char name[ 256 ];
        result = _p_soundSystem->getRecordDriverName( count, name, 256 );
        if ( result != FMOD_OK )
            log_error << "error getting input device name" << std::endl;
        else
        {
            log_debug << "input device detected: " << name << std::endl;
            // currently we take the first valid input device
            //! TODO: the input device should be obtained by menu system!
            if ( inputdevice == -1 )
                inputdevice = count;
        }
    }
    result = _p_soundSystem->setRecordDriver( inputdevice );
    if ( result != FMOD_OK )
    {
        std::stringstream exptext;
        exptext << "Error setting input device, error code: " << result;
        throw NetworkSoundExpection( exptext.str() );
    }

    // start grabbing from microphone
    result = _p_soundSystem->recordStart( _p_sound, true );
}

void VoiceMicrophoneInput::update()
{
    assert( _p_sound && "sound has not been created, first call initialize!" );

    // update base object first
    BaseVoiceInput::update();

    unsigned int encodedbytes = 0;
    // get access to sound's raw data and encode
    {
        unsigned int currentSoundTrackPos;
        _p_soundSystem->getRecordPosition( &currentSoundTrackPos );

        if ( _lastSoundTrackPos != currentSoundTrackPos )        
        {
            int blocklength = static_cast< int >( currentSoundTrackPos ) - static_cast< int >( _lastSoundTrackPos );

            if ( blocklength > 0 )
            {
                unsigned int len1, len2;
                void* p_rawpointer1 = NULL;
                void* p_rawpointer2 = NULL;

                // get access to raw sound data
                FMOD_RESULT res = _p_sound->lock( _lastSoundTrackPos * sizeof( VOICE_DATA_FORMAT_TYPE ), blocklength * sizeof( VOICE_DATA_FORMAT_TYPE ), &p_rawpointer1, &p_rawpointer2, &len1, &len2 );
                assert( res == FMOD_OK );

                if ( p_rawpointer1 && len1 )
                {
                    VOICE_DATA_FORMAT_TYPE* p_data = reinterpret_cast< VOICE_DATA_FORMAT_TYPE* >( p_rawpointer1 );

                    // encode grabbed raw data
                    encodedbytes = _p_codec->encode( p_data, blocklength, _p_encoderbuffer );
                    // check for network paket overrun
                    assert( encodedbytes <= VOICE_PAKET_MAX_BUF_SIZE );
                }
 
                _p_sound->unlock( p_rawpointer1, p_rawpointer2, len1, len2 );
            }

            _lastSoundTrackPos = currentSoundTrackPos;
        }
    }

    // call all registered functors with fresh data
    if ( encodedbytes > 0 )
    {
        std::vector< FCaptureInput* >::iterator p_beg = _sinks.begin(), p_end = _sinks.end();
        for ( ; p_beg != p_end; ++p_beg )
            ( *p_beg )->operator ()( _p_encoderbuffer, encodedbytes );
    }
}

// implementation of file input
VoiceFileInput::VoiceFileInput( const std::string& file, FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec ):
BaseVoiceInput( p_sndsystem, p_codec ),
_lastSoundTrackPos( 0 ),
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

    // create a sound object
    FMOD_RESULT result;
    FMOD_MODE               mode = FMOD_2D | FMOD_LOOP_NORMAL | FMOD_SOFTWARE;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    int                     channels = 1;

    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );
    createsoundexinfo.decodebuffersize  = 0;
    createsoundexinfo.length            = -1;
    createsoundexinfo.numchannels       = channels;
    createsoundexinfo.defaultfrequency  = VOICE_SAMPLE_RATE;
    createsoundexinfo.format            = VOICE_SOUND_FORMAT;

    result = _p_soundSystem->createSound( std::string( yaf3d::Application::get()->getMediaPath() + _file ).c_str(), mode, &createsoundexinfo, &_p_sound );
    if ( result != FMOD_OK )
    {
        std::stringstream exptext;
        exptext << "Problem occured during creation of sound object, error code: " << result;
        throw NetworkSoundExpection( exptext.str() );
    }

    _p_soundSystem->playSound( FMOD_CHANNEL_FREE, _p_sound, false, &_p_channel );
    _p_channel->setMute( true );
}

void VoiceFileInput::update()
{
    assert( _p_sound && "sound has not been created, first call initialize!" );

    // update base object first
    BaseVoiceInput::update();

    unsigned int encodedbytes = 0;
    // get access to sound's raw data and encode
    {
        unsigned int currentSoundTrackPos;
        _p_channel->getPosition( &currentSoundTrackPos, FMOD_TIMEUNIT_PCM );

        if ( _lastSoundTrackPos != currentSoundTrackPos )        
        {
            int blocklength = static_cast< int >( currentSoundTrackPos ) - static_cast< int >( _lastSoundTrackPos );

            if ( blocklength > 0 )
            {
                unsigned int len1, len2;
                void* p_rawpointer1 = NULL;
                void* p_rawpointer2 = NULL;

                // get access to raw sound data
                FMOD_RESULT res = _p_sound->lock( _lastSoundTrackPos * sizeof( VOICE_DATA_FORMAT_TYPE ), blocklength * sizeof( VOICE_DATA_FORMAT_TYPE ), &p_rawpointer1, &p_rawpointer2, &len1, &len2 );
                assert( res == FMOD_OK );

                if ( p_rawpointer1 && len1 )
                {
                    VOICE_DATA_FORMAT_TYPE* p_data = reinterpret_cast< VOICE_DATA_FORMAT_TYPE* >( p_rawpointer1 );

                    // encode grabbed raw data
                    encodedbytes = _p_codec->encode( p_data, blocklength, _p_encoderbuffer );
                    // check for network paket overrun
                    assert( encodedbytes <= VOICE_PAKET_MAX_BUF_SIZE );
                }
 
                _p_sound->unlock( p_rawpointer1, p_rawpointer2, len1, len2 );
            }

            _lastSoundTrackPos = currentSoundTrackPos;
        }
    }

    // call all registered functors with fresh data
    if ( encodedbytes > 0 )
    {
        std::vector< FCaptureInput* >::iterator p_beg = _sinks.begin(), p_end = _sinks.end();
        for ( ; p_beg != p_end; ++p_beg )
            ( *p_beg )->operator ()( _p_encoderbuffer, encodedbytes );
    }
}

} // namespace vrc
