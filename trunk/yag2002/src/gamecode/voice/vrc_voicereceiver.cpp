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
 # voice receiver is responsible for collecting incoming voice 
 #  over netweork
 #
 #   date of creation:  01/19/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicereceiver.h"
#include "vrc_codec.h"
#include "RNXPURL/Inc/XPURL.h"

namespace vrc
{

//! Class for fast access of stuff needed for decoding incoming voice pakets from different voice clients
class SoundNode
{
    public:

                                                    SoundNode()
                                                    {
                                                        _p_sampleQueue = new SoundSampleQueue;
                                                        _p_codec       = new NetworkSoundCodec;
                                                        _p_codec->setupDecoder();
                                                        _p_codec->setDecoderENH( true );
                                                        _p_codec->setMaxDecodeQueueSize( 8000 );
                                                    }

                                                    ~SoundNode()
                                                    {
                                                        delete _p_sampleQueue;
                                                        delete _p_codec;
                                                    }

        //! Queue for raw sound data
        SoundSampleQueue*                           _p_sampleQueue;

        //! Encoder / decoder, use only the decoder!
        NetworkSoundCodec*                          _p_codec;
};


VoiceReceiver::VoiceReceiver() :
_p_udpTransport( NULL ),
_p_soundSystem( NULL ),
_p_sound( NULL ),
_p_channel( NULL )
{
}

VoiceReceiver::~VoiceReceiver()
{
    shutdown();
}

void VoiceReceiver::shutdown()
{
    // clean up sender / sound node map
    SenderMap::iterator p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        delete p_beg->first;
        delete p_beg->second;
    }
    _soundNodeMap.clear();

    if ( _p_soundSystem )
    {
        FMOD_RESULT result;
        result = _p_sound->release();
        result = _p_soundSystem->release();
        _p_soundSystem = NULL;
        _p_sound       = NULL;
        _p_channel     = NULL;
    }

    if ( _p_udpTransport )
    {
        delete _p_udpTransport;
        _p_udpTransport = NULL;
    }
}

void VoiceReceiver::initialize() throw( NetworkSoundExpection )
{
    try
    {
        setupSound();
        setupNetwork();
    }
    catch ( const NetworkSoundExpection& e )
    {
        log_error << "Error initializing voice receiver " << std::endl;
        log_error << "   reason: " << e.what() << std::endl;
        return;
    }
}

void VoiceReceiver::update( float deltaTime )
{
    updateNetwork();
}

FMOD_RESULT F_CALLBACK voiceServerReadPCM( FMOD_SOUND* p_sound, void* p_data, unsigned int datalen )
{
    FMOD::Sound*    p_fmodsound = reinterpret_cast< FMOD::Sound* >( p_sound );
    void*           p_userdata;
    p_fmodsound->getUserData( &p_userdata );
    SoundNode*      p_soundnode = reinterpret_cast< SoundNode* >( p_userdata );

    if ( !p_soundnode )
        return FMOD_OK;
  
    VOICE_DATA_FORMAT_TYPE*   p_sndbuffer = reinterpret_cast< VOICE_DATA_FORMAT_TYPE* >( p_data );
    unsigned int    cnt = datalen / sizeof( VOICE_DATA_FORMAT_TYPE );

    // get the proper sample queue and put the sampes into raw sound buffer
    SoundSampleQueue& samplequeue = *p_soundnode->_p_sampleQueue;
    if ( !samplequeue.empty() )
    {
        log_debug << "recv bytes: " << samplequeue.size() << std::endl;

        // handle buffer underrun
        if ( samplequeue.size() < cnt )
            cnt = samplequeue.size();

        for ( ; cnt > 0; --cnt )
        {
            *p_sndbuffer++ = samplequeue.front();
            samplequeue.pop();
        }
    }
    //! TODO: better turn off the sound when buffer is empty
    else
    {
        memset( p_sndbuffer, 0, datalen );
    }

    return FMOD_OK;
}

void VoiceReceiver::setupSound() throw( NetworkSoundExpection )
{
    FMOD_RESULT result;
    result = FMOD::System_Create( &_p_soundSystem );
    if ( result != FMOD_OK )
        throw NetworkSoundExpection( "Cannot create sound system" );

    unsigned int version;
    result = _p_soundSystem->getVersion( &version );

    if ( version < FMOD_VERSION )
        throw NetworkSoundExpection( "FMOD version conflict." );

    //result = _p_soundSystem->setSoftwareFormat( 200, VOICE_SOUND_FORMAT, 1, 0, FMOD_DSP_RESAMPLER_LINEAR );

    result = _p_soundSystem->init( 32, FMOD_INIT_NORMAL, 0 );
}

SoundNode* VoiceReceiver::createSoundNode()
{
    // create a sound
    FMOD_RESULT result;

    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_SOFTWARE | FMOD_CREATESTREAM;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    int                     channels = 1;
    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );    
    createsoundexinfo.decodebuffersize  = 200;      // FIXME: determine a good buffer size, too big sizes result in noise and echos
    createsoundexinfo.length            = -1;
    createsoundexinfo.numchannels       = channels;
    createsoundexinfo.defaultfrequency  = VOICE_SAMPLE_RATE;
    createsoundexinfo.format            = VOICE_SOUND_FORMAT;
    createsoundexinfo.pcmreadcallback   = voiceServerReadPCM;

    result = _p_soundSystem->createSound( 0, mode, &createsoundexinfo, &_p_sound );
    FMOD::Channel* p_channel = NULL;
    _p_soundSystem->playSound( FMOD_CHANNEL_FREE, _p_sound, false, &p_channel );
    
    // create a new node
    SoundNode* p_soundnode = new SoundNode();
    _p_sound->setUserData( static_cast< void* >( p_soundnode ) );

    return p_soundnode;
}

void VoiceReceiver::setupNetwork() throw( NetworkSoundExpection )
{
    int channel;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_CHAN_SND, channel );

	RNReplicaNet::XPURL::RegisterDefaultTransports();
	RNReplicaNet::XPURL xpurl;

	assert( xpurl.FindTransport( "UDP@" ) );

	_p_udpTransport = xpurl.FindTransport( "UDP@" )->Allocate();
    _p_udpTransport->Listen( channel );
	std::string url = _p_udpTransport->ExportURL();
    log_debug << "  voice receiver started: " << url << std::endl;
}

void VoiceReceiver::updateNetwork()
{
    assert( _p_udpTransport && "network transport is not available!" );

    // check for new connecting senders
	RNReplicaNet::Transport* p_sender = _p_udpTransport->Accept();
    if ( p_sender )
    {
        log_debug << "  <- sender connection accepted " << std::endl;

        SoundNode* p_soundnode = createSoundNode();
        _soundNodeMap[ p_sender ] = p_soundnode;
    }

    // poll incoming data from senders
    static char s_buffer[ 512 ];
    SenderMap::iterator p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        p_sender = p_beg->first;
        if ( !p_sender->Recv( s_buffer, 512 ) )
            continue;

        VoicePaket* p_data = reinterpret_cast< VoicePaket* >( s_buffer );
        switch( p_data->_typeId )
        {
            case NETWORKSOUND_PAKET_TYPE_CON_REQ:
            {
                //! TODO:  send back a GRANT, currently we always grant!
                // we may change the grant to deny e.g. when too much senders are already connected
                p_data->_length = 0;
                p_data->_typeId = NETWORKSOUND_PAKET_TYPE_CON_GRANT;
                p_sender->SendReliable( reinterpret_cast< char* >( p_data ), 4  + p_data->_length );

                log_debug << "  <- voice sender connection request granted ( " << _soundNodeMap.size() << " )" << std::endl;
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_CON_CLOSE:
            {
                delete p_beg->first;
                delete p_beg->second;
                _soundNodeMap.erase( p_beg );
                p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
                log_debug << "  <- voice sender leaves hotspot ( " << _soundNodeMap.size() << " )" << std::endl;
                continue;
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_VOICE_DATA:
            {
                // decode and enqueue the samples
                SoundNode* p_soundnode = p_beg->second;
                if ( !p_soundnode->_p_codec->decode( p_data->_p_buffer, p_data->_length, *p_soundnode->_p_sampleQueue ) )
                {
                    log_debug << "decoder queue overrun, flushing queue!" << std::endl;
                    p_soundnode->_p_sampleQueue->c.clear();
                }
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_VOICE_ENABLE:
            {
                //! TODO
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_VOICE_DISABLE:
            {
                //! TODO
            }
            break;

            default:
                ;
        }
    }
}

} // namespace vrc
