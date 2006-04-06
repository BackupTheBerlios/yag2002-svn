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

#ifndef _VRC_VOICEINPUT_H_
#define _VRC_VOICEINPUT_H_

#include <vrc_main.h>
#include "vrc_networksoundimpl.h"

namespace vrc
{

class NetworkSoundCodec;

//! Base class for voice input
class BaseVoiceInput
{
    public:

        //! If no sound system / compression codec is given then they are created with default settings and destroyed on shutdown.
        //! Note: if a sound system / codec is given then it is not destroyed on shutdown!
                                                    BaseVoiceInput( FMOD::System* p_sndsystem = NULL, NetworkSoundCodec* p_codec = NULL );

        virtual                                     ~BaseVoiceInput();

        //! Initialize the voice input
        virtual void                                initialize() throw( NetworkSoundExpection );

        //! Update sound system, call this only once per simulation step
        virtual void                                update();

        //! Stop / continue grabbing
        virtual void                                stop( bool st ) = 0;

        //! Set input gain
        virtual void                                setInputGain( float gain );

        //! Set input device given its ID
        virtual void                                setInputDevice( unsigned int deviceid ) {}

        //! Functor for grabbing the encoded audio data.
        class FCaptureInput
        {
            public:

                                                        FCaptureInput() {}

                virtual                                 ~FCaptureInput() {}

                //! Implement the functor in your derived class
                //! p_encodedaudio contains the encoded audio data, length is the count of encoded bytes.
                virtual void                            operator ()( char* p_encodedaudio, unsigned short length ) = 0;
        };

        //! Register a sink for encoded audio stream. It will be called during update if something was encoded.
        //! Pass reg = true for registration and reg = false for deregistration.
        void                                        registerStreamSink( FCaptureInput* p_functor, bool reg = true );

    protected:

        //! Encode and distribute samples
        void                                        distributeSamples( void* const p_rawpointer1, void* const p_rawpointer2, unsigned int len1, unsigned int len2 );

        //! Sound system
        FMOD::System*                               _p_soundSystem;

        //! Sound compression codec
        NetworkSoundCodec*                          _p_codec;

        //! Sound object
        FMOD::Sound*                                _p_sound;
        
        //! Sound channel
        FMOD::Channel*                              _p_channel;

        //! Input gain
        float                                       _inputGain;

        //! Encoder buffer
        char                                        _p_encoderbuffer[ VOICE_PAKET_MAX_BUF_SIZE ];

        //! Indicated whether the sound system has been created by this instance
        bool                                        _createSoundSystem;

        //! Indicated whether the codec has been created by this instance
        bool                                        _createCodec;

        //! Identifies whether the input is grabbed ( reflects the state set by 'stop' )
        bool                                        _active;

        //! Used for getting access to raw sound data
        unsigned int                                _lastSoundTrackPos;

        //! Registered stream sinks
        std::vector< FCaptureInput* >               _sinks;
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
        void                                        initialize() throw( NetworkSoundExpection );

        //! Update the input and encoder, call this only once per simulation step.
        void                                        update();

        //! Set input device given its ID
        void                                        setInputDevice( unsigned int deviceid );

        //! Stop / continue grabbing
        void                                        stop( bool st );
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
        void                                        initialize() throw( NetworkSoundExpection );

        //! Update the input and encoder, call this only once per simulation step.
        void                                        update();

        //! Stop / continue grabbing
        void                                        stop( bool st );

        //! File to be captured as input
        std::string                                 _file;
};

} // namespace vrc

#endif // _VRC_VOICEINPUT_H_
