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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_codec.h"
#include "vrc_networksoundimpl.h"

namespace vrc
{

NetworkSoundCodec::NetworkSoundCodec() :
_p_codecEncoderState( NULL ),
_encoderQuality( 8 ),
_encoderComplexity( 5 ),
_p_codecDecoderState( NULL ),
_enh( 1 ),
_sampleRate( VOICE_SAMPLE_RATE ),
_maxDecodeQueueSize( CODEC_MAX_BUFFER_SIZE )
{
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
    assert( ( _p_codecEncoderState == NULL ) && "encoder already initialized" );

    int enabled  = 1;

    // setup preprocessor
    _p_preprocessorState = speex_preprocess_state_init( CODEC_FRAME_SIZE, _sampleRate );
    speex_preprocess_ctl( _p_preprocessorState, SPEEX_PREPROCESS_SET_DENOISE, &enabled );
    //! TODO: for some reason AGC does not work!? check why!
    //int agclevel = 16000;
    //speex_preprocess_ctl( _p_preprocessorState, SPEEX_PREPROCESS_SET_AGC, &enabled );
    //speex_preprocess_ctl( _p_preprocessorState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &agclevel );
    speex_preprocess_ctl( _p_preprocessorState, SPEEX_PREPROCESS_SET_VAD, &enabled );

    // setup encoder
    _p_codecEncoderState = speex_encoder_init( &speex_nb_mode );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_QUALITY, &_encoderQuality );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_COMPLEXITY, &_encoderComplexity );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_SAMPLING_RATE, &_sampleRate );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_VBR, &enabled );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_DTX, &enabled );
    speex_bits_init( &_encoderBits );
}

void NetworkSoundCodec::setupDecoder()
{
    assert( ( _p_codecDecoderState == NULL ) && "decoder already initialized" );

    int enabled = 1;
    _p_codecDecoderState = speex_decoder_init( &speex_nb_mode );
    speex_decoder_ctl( _p_codecDecoderState, SPEEX_SET_ENH, &enabled );
    speex_decoder_ctl( _p_codecDecoderState, SPEEX_SET_SAMPLING_RATE, &_sampleRate );
    speex_bits_init( &_decoderBits );
}

void NetworkSoundCodec::setMaxDecodeQueueSize( unsigned int size )
{
    // -100 as we do not exactly limit the queue size to this max size, see 'decode'
    _maxDecodeQueueSize = size - 100;
}

void NetworkSoundCodec::setEncoderQuality( unsigned int q )
{
    _encoderQuality = q;

    if ( _p_codecEncoderState )
        speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_QUALITY, &_encoderQuality );
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

unsigned int NetworkSoundCodec::encode( short* p_soundbuffer, unsigned int length, char* p_bitbuffer )
{
    assert( _p_codecEncoderState && "encoder has not been created!" );

    // limit the input to max buffer size
    if ( length > CODEC_MAX_BUFFER_SIZE )
    {
        length = CODEC_MAX_BUFFER_SIZE;
        log_debug << "*** sound codec: encoder had to limit requested buffer length to defined maximum: " << CODEC_MAX_BUFFER_SIZE << std::endl;
    }
 
    int encodedbytes = 0;
    // preprocess input before encoding, encode only when speech is detected
    int prec = speex_preprocess( _p_preprocessorState, p_soundbuffer, NULL );
    if ( prec > 0 )
    {
        speex_bits_reset( &_encoderBits );
        speex_encode_int( _p_codecEncoderState, p_soundbuffer, &_encoderBits );
        int nb = speex_bits_nbytes( &_encoderBits );
        encodedbytes = speex_bits_write( &_encoderBits, p_bitbuffer, nb );
    }

    return static_cast< unsigned int >( encodedbytes );
}

bool NetworkSoundCodec::decode( char* p_bitbuffer, unsigned int length, std::queue< short , std::deque< short > >& samplequeue )
{
    assert( _p_codecDecoderState && "decoder has not been created!" );

    // we check this only once per call, not for all iterations on pushing values into queue ( it saves cpu time )
    // so the actual max queue size may vary!
    if ( samplequeue.size() > _maxDecodeQueueSize )
        return false;

    // decode and enqueue sound data
    speex_bits_read_from( &_decoderBits, p_bitbuffer, length );
    int res = speex_decode_int( _p_codecDecoderState, &_decoderBits, _p_outputBuffer );
    if ( res == 0 )
    {
        for ( unsigned short cnt = 0; cnt < CODEC_FRAME_SIZE; ++cnt )
            samplequeue.push( _p_outputBuffer[ cnt ] );
    }

    return true;
}

} // namespace vrc
