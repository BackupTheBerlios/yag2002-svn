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

#ifndef _VRC_CODEC_H_
# error "do not include this file directly, include vrc_codec.h instead!"
#endif

template< typename SoundDataTypeT >
unsigned int NetworkSoundCodec::encode( SoundDataTypeT* p_soundbuffer, unsigned int length, char* p_bitbuffer )
{
    assert( _p_codecEncoderState && "encoder has not been created!" );

    // limit the input to max buffer size
    if ( length > CODEC_MAX_BUFFER_SIZE )
    {
        length = CODEC_MAX_BUFFER_SIZE;
        log_debug << "*** sound codec: encoder had to limit requested buffer length to defined maximum: " << CODEC_MAX_BUFFER_SIZE << std::endl;
    }

    // convert buffer from sound data type to float
    SoundDataTypeT* p_buffer = p_soundbuffer;
    for ( unsigned int cnt = 0; cnt < length; ++cnt )
    {
        _p_inputBuffer[ cnt ] = static_cast< float >( *p_buffer++ );
    }

    speex_bits_reset( &_encoderBits );
    speex_encode( _p_codecEncoderState, _p_inputBuffer, &_encoderBits );
    int nb = speex_bits_nbytes( &_encoderBits );
    int numbytes = speex_bits_write( &_encoderBits, p_bitbuffer, nb );

    return static_cast< unsigned int >( numbytes );
}

template< typename SoundDataTypeT >
bool NetworkSoundCodec::decode( char* p_bitbuffer, unsigned int length, std::queue< SoundDataTypeT , std::deque< SoundDataTypeT > >& samplequeue )
{
    assert( _p_codecDecoderState && "decoder has not been created!" );

    // we check this only once per call, not for all iterations on pushing values into queue ( it saves cpu time )
    // so the actual max queue size may vary!
    if ( samplequeue.size() > _maxDecodeQueueSize )
        return false;

    // decode and enqueue sound data
    speex_bits_read_from( &_decoderBits, p_bitbuffer, length );
    int res = speex_decode( _p_codecDecoderState, &_decoderBits, _p_outputBuffer );
    if ( res == 0 )
    {
        for ( unsigned short cnt = 0; cnt < CODEC_SAMPLES; ++cnt )
            samplequeue.push( static_cast< SoundDataTypeT >( _p_outputBuffer[ cnt ] ) );
    }

    return true;
}
