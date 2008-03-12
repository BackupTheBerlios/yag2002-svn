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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_voicetestutils.h"
#include "vrc_voiceinput.h"
#include "vrc_codec.h"
#include "vrc_networksoundimpl.h"
#include "vrc_natcommon.h"

namespace vrc
{

CodecTest::CodecTest( const std::string& outputfile, const std::string& testfile ) :
_testFile( testfile ),
_outputFile( outputfile ),
_p_soundInput( NULL ),
_p_codec( NULL )
{
}

CodecTest::~CodecTest()
{
    WaveWriterPCM16 wavwriter( VOICE_SAMPLE_RATE, WaveWriterPCM16::MONO );
    wavwriter.write( _outputFile, _sampleQueue );

    // shutdown
    shutdown();
}

void CodecTest::shutdown()
{
    if ( _p_voicePaket )
    {
        delete _p_voicePaket;
        _p_voicePaket = NULL;
    }

    // deregister from input stream
    if ( _p_soundInput )
    {
        _p_soundInput->registerStreamSink( this, false );
        delete _p_soundInput;
        _p_soundInput = NULL;
    }

    if ( _p_codec )
    {
        delete _p_codec;
        _p_codec = NULL;
    }
}

void CodecTest::initialize()
{
    _p_voicePaket = new VoicePaket;
    _p_voicePaket->_paketStamp = 0;
    _p_voicePaket->_senderID   = 0;

    // create sound codec
    _p_codec = new NetworkSoundCodec;
    _p_codec->setEncoderComplexity( 9 );
    _p_codec->setEncoderQuality( 10 );
    _p_codec->setupEncoder();
    _p_codec->setDecoderENH( true );
    _p_codec->setupDecoder();

    // if a test file is given then grab the input from wav file
    if ( _testFile.length() )
        _p_soundInput = new VoiceFileInput( _testFile, NULL, _p_codec );
    else
        _p_soundInput = new VoiceMicrophoneInput( NULL, _p_codec );

    _p_soundInput->initialize();

    // register for getting audio input stream
    _p_soundInput->registerStreamSink( this, true );

    // start the input grabbing
    _p_soundInput->stop( false );
}

void CodecTest::update()
{
    _p_soundInput->update();
}

void CodecTest::operator ()( char* p_encodedaudio, unsigned short length )
{ // this operator is called by voice input instance ( encoded audio is passed in p_encodedaudio )

    // simulate a transmission of encoded input
    _p_voicePaket->_length     = length;
    memcpy( _p_voicePaket->_p_buffer, p_encodedaudio, length );

    // simulate the reception of voice paket
    std::queue< short > samples;
    _p_codec->decode( _p_voicePaket->_p_buffer, length, samples );
    while( !samples.empty() )
    {
        _sampleQueue.push( samples.front() );
        samples.pop();
    }
}

} // namespace vrc
