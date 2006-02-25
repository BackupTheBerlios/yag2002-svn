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

typedef std::queue< short, std::deque< short > > SoundSampleQueue;

//! Class for network sound related exceptions
class NetworkSoundExpection : public std::runtime_error
{
    public:
                                                    NetworkSoundExpection( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~NetworkSoundExpection() throw() {}

                                                    NetworkSoundExpection( const NetworkSoundExpection& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    NetworkSoundExpection();

        NetworkSoundExpection&                      operator = ( const NetworkSoundExpection& );
};

//! Base class of all network sound implementations
class BaseNetworkSoundImplementation
{
    public:


                                                    BaseNetworkSoundImplementation();

        virtual                                     ~BaseNetworkSoundImplementation();

        
        //! Initialize
        virtual void                                initialize() throw( NetworkSoundExpection ) = 0;

        //! Update
        virtual void                                update( float deltaTime ) = 0;

        //! Shutdown
        virtual void                                shutdown() = 0;
};

//! Voice server name
#define VOICE_SERVER_NAME                       "vrc-voiceserver"
//! Voice sample rate
#define VOICE_SAMPLE_RATE                       8000

//! Sound format, this must match to VOICE_DATA_FORMAT_TYPE
#define VOICE_SOUND_FORMAT                      FMOD_SOUND_FORMAT_PCM16
//! Type of raw voice data, currently we use 'short' for PCM16
#define VOICE_DATA_FORMAT_TYPE                  short

//! Paket structure and type ids for transmitting voice stream
#define NETWORKSOUND_PAKET_TYPE_CON_REQ         0x0010  /* Sender requests receiver for connection */
#define NETWORKSOUND_PAKET_TYPE_CON_GRANT       0x0020  /* Receiver grants connection from sender */
#define NETWORKSOUND_PAKET_TYPE_CON_DENY        0x0030  /* Receiver denies connection from sender */
#define NETWORKSOUND_PAKET_TYPE_CON_CLOSE       0x0040  /* Sender closes connection */
#define NETWORKSOUND_PAKET_TYPE_VOICE_DATA      0x0050  /* voice data buffer */
#define NETWORKSOUND_PAKET_TYPE_VOICE_ENABLE    0x0060  /* enable voice transmission */
#define NETWORKSOUND_PAKET_TYPE_VOICE_DISABLE   0x0070  /* disable voice transmission */


#define VOICE_PAKET_MAX_BUF_SIZE                508
struct VoicePaket
{
    unsigned short      _typeId;            // paket id
    unsigned short      _length;            // buffer length
    char                _p_buffer[ VOICE_PAKET_MAX_BUF_SIZE ];          // this buffer must not exceed 508 bytes
};

} // namespace vrc

#endif // _VRC_NETWORKSOUNDIMPL_H_
