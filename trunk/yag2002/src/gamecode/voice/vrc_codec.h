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
#define _VRC_CODEC_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include <speex/speex_preprocess.h>

namespace vrc
{

// codec samples per frame ( for narrow mode it's always 160 )
#define CODEC_FRAME_SIZE            160
// maximum buffer size for encoding and decoding
#define CODEC_MAX_BUFFER_SIZE       8000


//! Codec class
//! Note: This codec class uses Speex in narrow mode
class NetworkSoundCodec
{
    public:

                                                    NetworkSoundCodec();

        virtual                                     ~NetworkSoundCodec();
        
        //! Setup Encoder
        void                                        setupEncoder();

        //! Setup Decoder
        void                                        setupDecoder();

        //! All incomming decoded samples are dropped when the sample queue is bigger than a maximum size set by this method.
        //! The default is CODEC_MAX_BUFFER_SIZE
        void                                        setMaxDecodeQueueSize( unsigned int size );

        //! Set encoder quality in range [1..10], default is 8
        void                                        setEncoderQuality( unsigned int q );

        //! Set decoder complexity in range [1..10], default is 5
        void                                        setEncoderComplexity( unsigned int c );

        //! Enable / disable perceptual enhancement for decoding
        void                                        setDecoderENH( bool enh );

        //! Encode 'length' number of raw sound data ( of type short ) to a compressed paket stored in p_bitbuffer.
        //! This method returns the count of bytes needed for encoding. The caller must ensure that p_bitbuffer is big enough.
        unsigned int                                encode( short* p_soundbuffer, unsigned int length, char* p_bitbuffer );

        //! Decode incoming compressed sound paket into given sample queue.
        //! Returns false if a queue overflow occurs ( see setMaxDecodeQueueSize ).
        bool                                        decode( char* p_bitbuffer, unsigned int length, std::queue< short , std::deque< short > >& samplequeue );

    protected:

        //! Preprocessor state, used for encoding
        SpeexPreprocessState*                       _p_preprocessorState;

        //! Encoder State
        void*                                       _p_codecEncoderState;
                
        //! Encoder bit structure
        SpeexBits                                   _encoderBits;

        //! [1..10]
        int                                         _encoderQuality;

        //! [1..10]
        int                                         _encoderComplexity;

        //! Decoder State
        void*                                       _p_codecDecoderState;
        
        //! Decoder bit structure
        SpeexBits                                   _decoderBits;

        //! Enhanced decoding
        int                                         _enh;

        //! Sample rate used for bit-rate calculation
        int                                         _sampleRate;

        //! Max size for decode queue
        unsigned int                                _maxDecodeQueueSize;

        //! Internal buffer for getting access to decoded sound data
        short                                       _p_outputBuffer[ CODEC_MAX_BUFFER_SIZE ];
};

} // namespace vrc

#endif // _VRC_CODEC_H_