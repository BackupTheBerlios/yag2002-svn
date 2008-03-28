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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_NETWORKSOUNDIMPL_H_
#define _VRC_NETWORKSOUNDIMPL_H_

#include <vrc_main.h>

namespace vrc
{
//! Voice sample rate
#define VOICE_SAMPLE_RATE                       16000
//! Sound format, this must match to VOICE_DATA_FORMAT_TYPE
#define VOICE_SOUND_FORMAT                      FMOD_SOUND_FORMAT_PCM16
//! Type of raw voice data, currently we use 'short' for PCM16
#define VOICE_DATA_FORMAT_TYPE                  short
//! Maximal data in _p_buffer ( in bytes ) in a voice paket.
//! Note: VOICE_PAKET_HEADER_SIZE + VOICE_PAKET_MAX_BUF_SIZE ( see vrc_natcommon.h ) should be maximal 512 bytes for a better performance
#define VOICE_PAKET_MAX_BUF_SIZE                ( 512 - 14  )


typedef std::queue< short > SoundSampleQueue;

//! Class for network sound related exceptions
class NetworkSoundException : public std::runtime_error
{
    public:
                                        NetworkSoundException( const std::string& reason ) :
                                         std::runtime_error( reason )
                                        {
                                        }

        virtual                         ~NetworkSoundException() throw() {}

                                        NetworkSoundException( const NetworkSoundException& e ) :
                                         std::runtime_error( e )
                                        {
                                        }

    protected:

                                        NetworkSoundException();

        NetworkSoundException&          operator = ( const NetworkSoundException& );
};

//! Base class of all network sound implementations
class BaseNetworkSoundImplementation
{
    public:


                                        BaseNetworkSoundImplementation();

        virtual                         ~BaseNetworkSoundImplementation();


        //! Initialize
        virtual void                    initialize() throw( NetworkSoundException ) = 0;

        //! Set output volume
        virtual void                    setOutputGain( float /* gain */ ) {}

        //! Set voice spot range
        virtual void                    setSpotRange( float /* range */ ) {}

        //! Update
        virtual void                    update( float deltaTime ) = 0;
 
        //! Called when a voice chat player left the session
        virtual void                    removePlayer( yaf3d::BaseEntity* p_entity ) {}

        //! Shutdown
        virtual void                    shutdown() = 0;

        //! Get the sender ID
        unsigned int                    getSenderID() const { return _senderID; }

    protected:

        //! Unique sender ID created by receiver on connection
        unsigned int                    _senderID;
};

} // namespace vrc

#endif // _VRC_NETWORKSOUNDIMPL_H_
