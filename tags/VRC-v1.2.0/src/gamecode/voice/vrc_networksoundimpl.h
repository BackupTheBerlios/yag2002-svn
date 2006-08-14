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
 # base class for networked sound
 #
 #   date of creation:  01/28/2006
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_NETWORKSOUNDIMPL_H_
#define _VRC_NETWORKSOUNDIMPL_H_

#include <vrc_main.h>

namespace vrc
{

typedef std::queue< short > SoundSampleQueue;

//! Class for network sound related exceptions
class NetworkSoundExpection : public std::runtime_error
{
    public:
                                        NetworkSoundExpection( const std::string& reason ) :
                                         std::runtime_error( reason )
                                        {
                                        }

        virtual                         ~NetworkSoundExpection() throw() {}

                                        NetworkSoundExpection( const NetworkSoundExpection& e ) :
                                         std::runtime_error( e )
                                        {
                                        }

    protected:

                                        NetworkSoundExpection();

        NetworkSoundExpection&          operator = ( const NetworkSoundExpection& );
};

//! Base class of all network sound implementations
class BaseNetworkSoundImplementation
{
    public:


                                        BaseNetworkSoundImplementation();

        virtual                         ~BaseNetworkSoundImplementation();


        //! Initialize
        virtual void                    initialize() throw( NetworkSoundExpection ) = 0;

        //! Set output volume
        virtual void                    setOutputGain( float /* gain */ ) {}

        //! Set voice spot range
        virtual void                    setSpotRange( float /* range */ ) {}

        //! Update
        virtual void                    update( float deltaTime ) = 0;

        //! Shutdown
        virtual void                    shutdown() = 0;

        //! Set output gain, this is meant to be used in a receiver
};

//! Voice server name
#define VOICE_SERVER_NAME                       "vrc-voiceserver"
//! Voice sample rate
#define VOICE_SAMPLE_RATE                       16000

//! Sound format, this must match to VOICE_DATA_FORMAT_TYPE
#define VOICE_SOUND_FORMAT                      FMOD_SOUND_FORMAT_PCM16
//! Type of raw voice data, currently we use 'short' for PCM16
#define VOICE_DATA_FORMAT_TYPE                  short

//! Life-sign signaling period ( in seconds ) used in voice paket transmission protocol
#define VOICE_LIFESIGN_PERIOD                   5.0f

//! Paket structure and type ids for transmitting voice stream
#define NETWORKSOUND_PAKET_TYPE_CON_REQ         0x0010  /* Sender requests receiver for connection */
#define NETWORKSOUND_PAKET_TYPE_CON_GRANT       0x0020  /* Receiver grants connection from sender */
#define NETWORKSOUND_PAKET_TYPE_CON_DENY        0x0030  /* Receiver denies connection from sender */
#define NETWORKSOUND_PAKET_TYPE_CON_CLOSE       0x0040  /* Sender closes connection */
#define NETWORKSOUND_PAKET_TYPE_CON_PING        0x0050  /* Sender pings receiver */
#define NETWORKSOUND_PAKET_TYPE_CON_PONG        0x0060  /* Receiver pongs sender */
#define NETWORKSOUND_PAKET_TYPE_VOICE_DATA      0x0070  /* voice data buffer */

//! Maximal data in _p_buffer ( in bytes ) in a voice paket
#define VOICE_PAKET_MAX_BUF_SIZE                500
//! Voice paket header size ( in bytes )
#define VOICE_PAKET_HEADER_SIZE                 12
//! The total paket size must not exceed 512 bytes!
struct VoicePaket
{
    unsigned int        _paketStamp;        // stamp for detecting lost pakets
    unsigned int        _senderID;          // unique sender ID, 0 means unassigned
    unsigned short      _typeId;            // paket id
    unsigned short      _length;            // buffer length
    char                _p_buffer[ VOICE_PAKET_MAX_BUF_SIZE ];
};

} // namespace vrc

#endif // _VRC_NETWORKSOUNDIMPL_H_
