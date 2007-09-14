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
 # utilities for testing the voice chat related stuff
 #
 #   date of creation:  03/18/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_VOICETESTUTILS_H_
#define _VRC_VOICETESTUTILS_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include "vrc_voiceinput.h"

namespace vrc
{

class  NetworkSoundCodec;
class  BaseVoiceInput;
struct VoicePaket;

//! Class for testing the codec
class CodecTest : public BaseVoiceInput::FCaptureInput
{
    public:

                                                    CodecTest( const std::string& outputfile, const std::string& testfile = "" );

        virtual                                     ~CodecTest();

        //! Initialize the test unit
        void                                        initialize();

        //! Update
        void                                        update();

    protected:

        //! Shutdown the voice server
        void                                        shutdown();

        //! Functor for grabbing the sound input, see class BaseVoiceInput
        void                                        operator ()( char* p_encodedaudio, unsigned short length );

        //! Test wav file, must be a PCM16, 16 khz
        std::string                                 _testFile;

        //! File name for the ripped stream
        std::string                                 _outputFile;

        //! Paket for voice transmission
        VoicePaket*                                 _p_voicePaket;

        //! Sound input
        BaseVoiceInput*                             _p_soundInput;

        //! Sound compression codec
        NetworkSoundCodec*                          _p_codec;

        //! Sound sample queue
        std::queue< short >                         _sampleQueue;

    friend class BaseVoiceInput::FCaptureInput;
};

//! Class for writing a wav file given a collection of samples
//! For PCM16 set DataTypeT to 'short'
template< typename DataTypeT >
class WaveWriter
{
    public:

        //! Channel modes
        enum
        {
            MONO,
            STEREO
        } ChannelMode;

        //! Sample rate can be 8000 or 16000, etc.
        //! Use MONO or STEREO for channelmode
                                                    WaveWriter( unsigned samplerate, unsigned int channelmode );

        virtual                                     ~WaveWriter();

        //! Write out the content of vector into given file
        bool                                        write( const std::string& filename, const std::vector< DataTypeT >& data );

        //! Write out the content of queue into given file
        bool                                        write( const std::string& filename, const std::queue< DataTypeT >& data );

    protected:

        void                                        writeHeader( std::fstream& stream, unsigned int size );

        void                                        writeSample( std::fstream& stream, DataTypeT sample );

        unsigned int                                _rate;

        unsigned int                                _bits;

        unsigned int                                _channels;
};

//! Define common PCM types for WaveWriter
typedef class WaveWriter< short > WaveWriterPCM16;
typedef class WaveWriter< char  > WaveWriterPCM8;

//! Include the inline file
#include "vrc_voicetestutils.inl"

} // namespace vrc

#endif // _VRC_VOICETESTUTILS_H_
