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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_VOICEINPUT_H_
#define _VRC_VOICEINPUT_H_

#include <vrc_main.h>
#include "vrc_codec.h"
#include "vrc_networksoundimpl.h"

namespace vrc
{

class NetworkSoundCodec;
class InputGrabber;

//! Base class for voice input
class BaseVoiceInput
{
    public:

        //! If no sound system / compression codec is given then they are created with default settings and destroyed on shutdown.
        //! Note: if a sound system / codec is given then it is not destroyed on shutdown!
                                                    BaseVoiceInput( FMOD::System* p_sndsystem = NULL, NetworkSoundCodec* p_codec = NULL );

        virtual                                     ~BaseVoiceInput();

        //! Initialize the voice input
        virtual void                                initialize() throw( NetworkSoundException );

        //! Update sound system, call this only once per simulation step
        virtual void                                update() = 0;

        //! Stop / continue grabbing
        virtual void                                stop( bool st ) = 0;

        //! Set input gain
        virtual void                                setInputGain( float gain );

        //! Set input device given its ID
        virtual void                                setInputDevice( unsigned int /*deviceid*/ ) {}

        //! Callback class for grabbing the encoded audio data.
        class CallbackInputStream
        {
            public:

                                                        CallbackInputStream() {}

                virtual                                 ~CallbackInputStream() {}

                //! Implement the callback method in your derived class
                //! p_encodedaudio contains the encoded audio data, length is the count of encoded bytes.
                virtual void                            recvEncodedAudio( char* p_encodedaudio, unsigned short length ) = 0;
        };

        //! Register a sink for encoded audio stream. It will be called during update if something was encoded.
        //! Pass reg = true for registration and reg = false for deregistration.
        void                                        registerStreamSink( CallbackInputStream* p_functor, bool reg = true );

    protected:

        //! Encode and distribute samples
        void                                        distributeSamples( VOICE_DATA_FORMAT_TYPE* p_data, unsigned int length );

        //! Sound system
        FMOD::System*                               _p_soundSystem;

        //! Sound compression codec
        NetworkSoundCodec*                          _p_codec;

        //! Sound object
        FMOD::Sound*                                _p_sound;

        //! Sound channel
        FMOD::Channel*                              _p_channel;

        //! Input grabber thread
        InputGrabber*                               _p_grabber;

        //! Input gain
        float                                       _inputGain;

        //! Encoder buffer
        char                                        _p_encoderBuffer[ CODEC_MAX_BUFFER_SIZE ];

        //! Indicated whether the sound system has been created by this instance
        bool                                        _createSoundSystem;

        //! Indicated whether the codec has been created by this instance
        bool                                        _createCodec;

        //! Identifies whether the input is grabbed ( reflects the state set by 'stop' )
        bool                                        _active;

        //! Used for getting access to raw sound data
        unsigned int                                _lastSoundTrackPos;

        //! Length of a sampled input chunk
        unsigned int                                _chunkLength;

        //! Registered stream sinks
        std::vector< CallbackInputStream* >         _sinks;

        //! Mutex used for sink feeding and registration / deregistration
        OpenThreads::Mutex                          _sinkMutex;
};

//! Class for capturing microphone input
class VoiceMicrophoneInput: public BaseVoiceInput
{
    public:

        //! Pass NULL for p_sndsystem or p_codec for getting them created with default settings
                                                    VoiceMicrophoneInput( FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec );

        virtual                                     ~VoiceMicrophoneInput();

    protected:

        //! Initialize the microphone input
        void                                        initialize() throw( NetworkSoundException );

        //! Update the input and encoder, call this only once per simulation step.
        void                                        update();

        //! Set input device given its ID
        void                                        setInputDevice( unsigned int deviceid );

        //! Stop / continue grabbing
        void                                        stop( bool st );

        //! Identifies whether the micro device has been initialized properly
        bool                                        _inputDeviceReady;
};

//! Class for capturing file as input
class VoiceFileInput: public BaseVoiceInput
{
    public:

        //! Pass NULL for p_sndsystem or p_codec for getting them created with default settings
                                                    VoiceFileInput( const std::string& file, FMOD::System* p_sndsystem, NetworkSoundCodec* p_codec );

        virtual                                     ~VoiceFileInput();

    protected:

        //! Initialize the file input
        void                                        initialize() throw( NetworkSoundException );

        //! Update the input and encoder, call this only once per simulation step.
        void                                        update();

        //! Stop / continue grabbing
        void                                        stop( bool st );

        //! File to be captured as input
        std::string                                 _file;
};

} // namespace vrc

#endif // _VRC_VOICEINPUT_H_
