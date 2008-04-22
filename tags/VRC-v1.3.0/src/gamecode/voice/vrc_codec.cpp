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
 # codec for encoding / decoding sound using Speex
 #
 #   date of creation:  02/04/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_codec.h"
#include "vrc_networksoundimpl.h"
#include "vrc_voicetestutils.h"


namespace vrc
{

NetworkSoundCodec::NetworkSoundCodec() :
_p_mode( NULL ),
_p_preprocessorState( NULL ),
_p_codecEncoderState( NULL ),
_encoderQuality( 5 ),
_encoderComplexity( 3 ),
_encoderFrameSize( 0 ),
_encodedFrameBytes( 0 ),
_p_codecDecoderState( NULL ),
_enh( 1 ),
_sampleRate( VOICE_SAMPLE_RATE ),
_decoderFrameSize( 0 )
{
    // determine the mode
    int modeID = -1;

    if ( _sampleRate > 48000 )
    {
        log_error << "NetworkSoundCodec: sample rate " << _sampleRate << " is too high, valid sample rate range is [ 6000 ... 48000 ]" << std::endl;
        return;
    }

    if ( _sampleRate > 25000 )
    {
        modeID = SPEEX_MODEID_UWB;
    }
    else if ( _sampleRate > 12500 )
    {
        modeID = SPEEX_MODEID_WB;
    }
    else if ( _sampleRate >= 6000 )
    {
        modeID = SPEEX_MODEID_NB;
    }
    else
    {
        log_error << "NetworkSoundCodec: sample rate " << _sampleRate << " is too low, valid sample rate range is [ 6000 ... 48000 ]" << std::endl;
        return;
    }

    _p_mode = speex_lib_get_mode( modeID );
}

NetworkSoundCodec::~NetworkSoundCodec()
{
    if ( _p_codecEncoderState )
    {
        speex_preprocess_state_destroy( _p_preprocessorState );
        speex_encoder_destroy( _p_codecEncoderState );
        speex_bits_destroy( &_encoderBits );
        _p_codecEncoderState = NULL;
    }

    if ( _p_codecDecoderState )
    {
        speex_decoder_destroy( _p_codecDecoderState );
        speex_bits_destroy( &_decoderBits );
        _p_codecDecoderState = NULL;
    }
}

void NetworkSoundCodec::setupEncoder()
{
    // catch invalid sample rates resulting in invalid mode
    if ( !_p_mode )
        return;

    assert( ( _p_codecEncoderState == NULL ) && "encoder already initialized" );

    int enabled   = 1;
    int disabled  = 0;
    // setup encoder
    _p_codecEncoderState = speex_encoder_init( _p_mode );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_QUALITY, &_encoderQuality );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_COMPLEXITY, &_encoderComplexity );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_SAMPLING_RATE, &_sampleRate );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_VBR, &disabled );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_DTX, &disabled );
    speex_bits_init( &_encoderBits );

    // determine the frame bytes by encoding a dummy input sample set
    float dummyinput[ 512 ];
    memset( dummyinput, 0, sizeof( dummyinput ) );
    speex_bits_reset( &_encoderBits );
    speex_encode( _p_codecEncoderState, dummyinput, &_encoderBits );
    _encodedFrameBytes = static_cast< unsigned int >( speex_bits_nbytes( &_encoderBits ) );

    // setup preprocessor
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_GET_FRAME_SIZE, &_encoderFrameSize );
    _p_preprocessorState = speex_preprocess_state_init( _encoderFrameSize, _sampleRate );
    speex_preprocess_ctl( _p_preprocessorState, SPEEX_PREPROCESS_SET_DENOISE, &enabled );
    speex_preprocess_ctl( _p_preprocessorState, SPEEX_PREPROCESS_SET_VAD, &disabled );

    // erase the input buffer
    memset( _p_inputBuffer, 0, sizeof( _p_inputBuffer ) );
}

void NetworkSoundCodec::setupDecoder( int quality, int complexity )
{
    // catch invalid sample rates resulting in invalid mode
    if ( !_p_mode )
        return;

    assert( ( _p_codecDecoderState == NULL ) && "decoder already initialized" );

    int enabled = 1;
    _p_codecDecoderState = speex_decoder_init( _p_mode );
    speex_decoder_ctl( _p_codecDecoderState, SPEEX_SET_ENH, &enabled );
    speex_decoder_ctl( _p_codecDecoderState, SPEEX_SET_SAMPLING_RATE, &_sampleRate );
    speex_decoder_ctl( _p_codecDecoderState, SPEEX_GET_FRAME_SIZE, &_decoderFrameSize );
    speex_bits_init( &_decoderBits );

    // determine the encoded frame size which depends on quality
    //------
    int disabled  = 0;
    // setup a temporary encoder
    void* p_codecEncoderState = speex_encoder_init( _p_mode );
    speex_encoder_ctl( p_codecEncoderState, SPEEX_SET_QUALITY, &quality );
    speex_encoder_ctl( p_codecEncoderState, SPEEX_SET_COMPLEXITY, &complexity );
    speex_encoder_ctl( p_codecEncoderState, SPEEX_SET_SAMPLING_RATE, &_sampleRate );
    speex_encoder_ctl( p_codecEncoderState, SPEEX_SET_VBR, &disabled );
    speex_encoder_ctl( p_codecEncoderState, SPEEX_SET_DTX, &disabled );
    SpeexBits encoderBits;
    speex_bits_init( &encoderBits );
    speex_bits_reset( &encoderBits );
    float in[ 512 ];
    speex_encode( p_codecEncoderState, in, &encoderBits );

    // here we get the needed frame size
    _encodedFrameBytes = static_cast< unsigned int >( speex_bits_nbytes( &encoderBits ) );

    // now destroy the encoder stuff
    speex_encoder_destroy( p_codecEncoderState );
    speex_bits_destroy( &encoderBits );
    //------

    // erase the output buffer
    memset( _p_outputBuffer, 0, sizeof( _p_outputBuffer ) );
}

void NetworkSoundCodec::setEncoderQuality( unsigned int q )
{
    _encoderQuality = q;

    if ( _p_codecEncoderState )
    {
        speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_QUALITY, &_encoderQuality );

        // determine the encoded frame size
        speex_bits_reset( &_encoderBits );
        speex_encode( _p_codecEncoderState, _p_inputBuffer, &_encoderBits );
        _encodedFrameBytes = static_cast< unsigned int >( speex_bits_nbytes( &_encoderBits ) );
        // erase the input buffer
        memset( _p_inputBuffer, 0, sizeof( _p_inputBuffer ) );
    }
}

void NetworkSoundCodec::setEncoderComplexity( unsigned int c )
{
    _encoderComplexity = c;

    if ( _p_codecEncoderState )
        speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_COMPLEXITY, &_encoderComplexity );
}

void NetworkSoundCodec::setDecoderENH( bool enh )
{
    _enh = enh ? 1 : 0;

    if ( _p_codecDecoderState )
        speex_decoder_ctl( _p_codecDecoderState, SPEEX_SET_ENH, &_enh );
}

unsigned int NetworkSoundCodec::encode( short* p_soundbuffer, unsigned int length, char* p_bitbuffer, float gain )
{
    assert( _p_codecEncoderState && "encoder has not been created!" );

    // check internal buffer overrun
    if ( _inputBufferQueue.size() < CODEC_MAX_BUFFER_SIZE - length )
    {
        for ( unsigned int cnt = 0; cnt < length; ++cnt )
        {
            _inputBufferQueue.push( p_soundbuffer[ cnt ] );
        }
    }
    else
    {
        log_verbose << "Voice Codec: buffer overrun!" << std::endl;
    }

    unsigned int buffersize = _inputBufferQueue.size();

    // do we have enough data for encoding?
    if ( buffersize < _encoderFrameSize )
    {
        return 0;
    }

    int encodedbytes = 0;
    buffersize      -= _encoderFrameSize;
    for ( unsigned int pos = 0; pos < buffersize; pos += _encoderFrameSize )
    {
        for ( unsigned int cnt = 0; cnt < _encoderFrameSize; ++cnt )
        {
            _p_inputBuffer[ cnt ] = static_cast< float >( _inputBufferQueue.front() ) * gain;
            _inputBufferQueue.pop();
        }

        // preproces samples
        /*int vad = */ //speex_preprocess( _p_preprocessorState, p_soundbuffer, NULL );

        // encode
        speex_bits_reset( &_encoderBits );
        speex_encode( _p_codecEncoderState, _p_inputBuffer, &_encoderBits );
        int nb = speex_bits_nbytes( &_encoderBits );
        //! NOTE: we expect encoded packets of CODEC_CHUNK_SIZE bytes
        encodedbytes += speex_bits_write( &_encoderBits, &p_bitbuffer[ encodedbytes ], nb );
    }

    return encodedbytes;
}

bool NetworkSoundCodec::decode( char* p_bitbuffer, unsigned int length, std::queue< short >& samplequeue, float gain )
{
    assert( _p_codecDecoderState && "decoder has not been created!" );

    if ( length < _encodedFrameBytes )
    {
        log_error << "Codec: unexpected codec packet size received, " << length << ", expected minimal size: " << _encodedFrameBytes << std::endl;
        return false;
    }

    int buffersize = length;
    for ( int pos = 0; buffersize > 0; pos += _encodedFrameBytes, buffersize -= _encodedFrameBytes )
    {
        // decode and enqueue sound data
        speex_bits_read_from( &_decoderBits, &p_bitbuffer[ pos ], _encodedFrameBytes );
        int res = speex_decode( _p_codecDecoderState, &_decoderBits, _p_outputBuffer );
        if ( res == 0 )
        {
            for ( unsigned short cnt = 0; cnt < _decoderFrameSize; ++cnt )
                samplequeue.push( static_cast< short >( _p_outputBuffer[ cnt ] * gain ) );
        }
    }

    return true;
}

} // namespace vrc
