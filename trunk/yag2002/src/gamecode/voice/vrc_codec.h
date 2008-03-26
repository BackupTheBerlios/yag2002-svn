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

#ifndef _VRC_CODEC_H_
#define _VRC_CODEC_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include <speex/speex_preprocess.h>


namespace vrc
{

// maximum smaple buffer size for encoding and decoding
#define CODEC_MAX_BUFFER_SIZE       16000


//! Codec class
//! Note: This codec class uses Speex in wide mode
class NetworkSoundCodec
{
    public:

                                                    NetworkSoundCodec();

        virtual                                     ~NetworkSoundCodec();

        //! Setup Encoder
        void                                        setupEncoder();

        //! Set encoder quality in range [ 1 ... 10 ], default is 8
        void                                        setEncoderQuality( unsigned int q );

        //! Set decoder complexity in range [ 1 ... 10 ], default is 5
        void                                        setEncoderComplexity( unsigned int c );

        //! Get the frame size of every encoded input voice packet in bytes.
        unsigned int                                getEncoderFrameSize() const { return _encoderFrameSize; }

        //! Get the bytes needed for encoding one input frame, it depends on quality setting.
        unsigned int                                getEncodedFrameBytes() const { return _encodedFrameBytes; }

        /**
            Encode 'length' number of raw sound data ( of type short ) to a compressed paket stored in p_bitbuffer, the samples are multiplied by 'gain'.
            This method returns the count of bytes needed for encoding. The caller must ensure that p_bitbuffer is big enough.
            Note: p_soundbuffer is modified by this method ( preprocessing ) 
            Every call consumes 320 samples. If fewer then 320 samples are passed in then they will be buffered and on considered on next calls.
            Returns the count of encoded bytes.
        */ 
        unsigned int                                encode( short* p_soundbuffer, unsigned int length, char* p_bitbuffer, float gain = 1.0f );

        //! Setup the decoder given quality and complexity, it must match to those settings in encoder.
        void                                        setupDecoder( int quality, int complexity );

        //! Enable / disable perceptual enhancement for decoding
        void                                        setDecoderENH( bool enh );

        //! Decode incoming compressed sound paket into given sample queue, the samples are multiplied by 'gain'.
        //! Returns false if a queue overflow occurs ( see setMaxDecodeQueueSize ).
        bool                                        decode( char* p_bitbuffer, unsigned int length, std::queue< short >& samplequeue, float gain = 1.0f );

    protected:

        //! Codec mode ( narrow, wide, ultrawide )
        const SpeexMode*                            _p_mode;

        //! Preprocessor state, used for encoding
        SpeexPreprocessState*                       _p_preprocessorState;

        //! Encoder State
        void*                                       _p_codecEncoderState;

        //! Encoder bit structure
        SpeexBits                                   _encoderBits;

        //! [ 1 ... 10 ], higher values cause bigger size of encoded frames
        int                                         _encoderQuality;

        //! [ 1 ... 10 ], note that values bigger than 4 cause high cpu usage
        int                                         _encoderComplexity;

        //! Input frame size for encoder
        unsigned int                                _encoderFrameSize;

        //! Count of bytes needed for encoding one frame, it depends on quality setting.
        unsigned int                                _encodedFrameBytes;

        //! Decoder State
        void*                                       _p_codecDecoderState;

        //! Decoder bit structure
        SpeexBits                                   _decoderBits;

        //! Enhanced decoding
        int                                         _enh;

        //! Sample rate used for bit-rate calculation
        int                                         _sampleRate;

        //! Frame size for decoder
        int                                         _decoderFrameSize;

        //! Internal buffer for encoding sound data
        float                                       _p_inputBuffer[ CODEC_MAX_BUFFER_SIZE ];

        //! Input queue for raw data to be encoded
        std::queue< short >                         _inputBufferQueue;

        //! Internal buffer for getting access to decoded sound data
        float                                       _p_outputBuffer[ CODEC_MAX_BUFFER_SIZE ];
};


} // namespace vrc

#endif // _VRC_CODEC_H_
