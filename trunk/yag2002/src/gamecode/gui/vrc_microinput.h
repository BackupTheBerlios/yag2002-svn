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
 # microphone input, contains also test methods for input / output gain
 #
 #   date of creation:  03/14/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_MICROINPUT_H_
#define _VRC_MICROINPUT_H_

#include <vrc_main.h>

namespace vrc
{

//! Class for setting up microphone input device
class MicrophoneInput
{
    public:

        //! Type for enumerating input devices < device ID / device name >
        typedef std::map< unsigned int, std::string > InputDeviceMap;

                                                    MicrophoneInput();

        virtual                                     ~MicrophoneInput();

        //! Get all detected input devices
        bool                                        getInputDevices( InputDeviceMap& devices );

        //! Set input device given its ID, retrieve the ID using 'getInputDevices'
        //! This method can be used during micro test. Returns false if something went wrong.
        bool                                        setInputDevice( unsigned int deviceid );

        //! Begin microphone input test, call this method after setting a valid input device using 'setInputDevice'
        void                                        beginMicroTest();

        //! Set input gain [ 0 ... 2 ]
        //! This method can be used during micro test
        void                                        setInputGain( float gain );

        //! Get input gain [ 0 ... 2 ]
        inline float                                getInputGain();

        //! Set output gain [ 0 ... 2 ]
        //! This method can be used during micro test
        void                                        setOutputGain( float gain );

        //! Get output gain [ 0 ... 2 ]
        inline float                                getOutputGain();

        //! End microphone input test
        void                                        endMicroTest();

    protected:

        //! Start capturing input from given device
        bool                                        setupInputCapturing( unsigned int deviceid );

        //! Sound system
        FMOD::System*                               _p_soundSystem;

        //! Sound object
        FMOD::Sound*                                _p_sound;
        
        //! Sound channel
        FMOD::Channel*                              _p_channel;

        //! Used for internal housekeeping
        bool                                        _microTestRunning;

        //! Input gain [ 0 ... 2 ]
        float                                       _inputGain;

        //! Output gain [ 0 ... 2 ]
        float                                       _outputGain;
};


inline float MicrophoneInput::getInputGain()
{ 
    return _inputGain; 
}

inline float MicrophoneInput::getOutputGain()
{ 
    return _outputGain; 
}

} // namespace vrc

#endif // _VRC_MICROINPUT_H_
