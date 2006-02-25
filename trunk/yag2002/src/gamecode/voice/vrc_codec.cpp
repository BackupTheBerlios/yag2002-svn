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

    _p_codecEncoderState = speex_encoder_init( &speex_nb_mode );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_QUALITY, &_encoderQuality );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_COMPLEXITY, &_encoderComplexity );
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_SAMPLING_RATE, &_sampleRate );
    int enabled  = 1;
    speex_encoder_ctl( _p_codecEncoderState, SPEEX_SET_VAD, &enabled );
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

} // namespace vrc
