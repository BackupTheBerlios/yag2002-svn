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
#include "../player/vrc_player.h"
#include "RNXPURL/Inc/XPURL.h"

#include "vrc_voicetestutils.h"

namespace vrc
{

// codec samples per frame
#define CODEC_FRAME_SIZE            320

// maximal allowed senders connected to the receiver
#define MAX_SENDERS_CONNECTED       5

//**************************************************************
//! TODO remove this test ripper
std::queue< short > s_wavqueue;
//*******************************************************************************



//! Class for fast access of stuff needed for decoding incoming voice pakets from different voice clients
class SoundNode
{
    public:

                                     SoundNode()
                                     {
                                         _p_sound        = NULL;
                                         _p_channel      = NULL;
                                         _p_senderPlayer = NULL;
                                         _pingTimer      = 0.0f;
                                         _senderID       = 0;
                                         _lastPaketStamp = 0;
                                         _p_sampleQueue  = new SoundSampleQueue;
                                         _p_codec        = new NetworkSoundCodec;
                                         _p_codec->setupDecoder();
                                         _p_codec->setDecoderENH( true );
                                         _p_codec->setMaxDecodeQueueSize( CODEC_MAX_BUFFER_SIZE );
                                     }

                                     ~SoundNode()
                                     {
                                         if ( _p_sound )                                                            
                                             _p_sound->release();

                                         delete _p_sampleQueue;
                                         delete _p_codec;
                                     }

        //! Queue for raw sound data
        SoundSampleQueue*            _p_sampleQueue;

        //! Encoder / decoder, use only the decoder!
        NetworkSoundCodec*           _p_codec;

        //! Sound object
        FMOD::Sound*                 _p_sound;
        
        //! Sound channel
        FMOD::Channel*               _p_channel;

        //! Unique sender ID
        unsigned int                 _senderID;

        //! Timer for alive-signaling
        float                        _pingTimer;

        //! Last received valid voice paket stamp
        unsigned int                 _lastPaketStamp;

        //! Ghost of sending sound data
        EnPlayer*                   _p_senderPlayer;
};

VoiceReceiver::VoiceReceiver() :
_senderID( 0 ),
_p_udpTransport( NULL ),
_p_soundSystem( NULL ),
_outputGain( 1.0f ),
_spotRange( 20.0f ),
_cutoffRange( 0.0f )
{
    // setup sender ID enumeration
    srand( static_cast< unsigned int >( time( NULL ) ) );
    _senderID = static_cast< unsigned int >( rand() );
}

VoiceReceiver::~VoiceReceiver()
{
    shutdown();
}

void VoiceReceiver::shutdown()
{
//! TODO: remove this rip code later
//*******************************
    //// write out the ripped wav file
    //{
    //    SenderMap::iterator p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
    //    // we take the first sender and write it out
    //    if ( p_beg != p_end )
    //    {
    //        // write out the rip
    //        WaveWriterPCM16 wavwriter( VOICE_SAMPLE_RATE, WaveWriterPCM16::MONO );
    //        wavwriter.write( yaf3d::Application::get()->getMediaPath() + std::string( "sender0-rip.wav" ), s_wavqueue );
    //    }
    //}
//*******************************


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
        _p_soundSystem->close();
        _p_soundSystem->release();
        _p_soundSystem = NULL;
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

void VoiceReceiver::setOutputGain( float gain )
{
    _outputGain = gain;
}

void VoiceReceiver::setSpotRange( float range )
{
    _spotRange = range;
    // we take a cut--off range of 1/3 of the spot range
    _cutoffRange = _spotRange * 0.333f;
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

    // use FMOD_INIT_STREAM_FROM_UPDATE in order to synchronize the play callback with networking in update method
    result = _p_soundSystem->init( 5, FMOD_INIT_STREAM_FROM_UPDATE, 0 );
}

FMOD_RESULT F_CALLBACK voiceReceiverReadPCM( FMOD_SOUND* p_sound, void* p_data, unsigned int datalen )
{
    FMOD::Sound*    p_fmodsound = reinterpret_cast< FMOD::Sound* >( p_sound );
    void*           p_userdata;
    p_fmodsound->getUserData( &p_userdata );
    SoundNode*      p_soundnode = reinterpret_cast< SoundNode* >( p_userdata );

    if ( !p_soundnode )
        return FMOD_OK;

    VOICE_DATA_FORMAT_TYPE*   p_sndbuffer = reinterpret_cast< VOICE_DATA_FORMAT_TYPE* >( p_data );
    unsigned int    cnt = datalen / sizeof( VOICE_DATA_FORMAT_TYPE );

    // get the proper sample queue and put the samples into raw sound buffer
    SoundSampleQueue& samplequeue = *p_soundnode->_p_sampleQueue;
    if ( !samplequeue.empty() )
    {
        // handle buffer underrun
        if ( samplequeue.size() < cnt )
        {
            log_verbose << "playback buffer underrun: " << samplequeue.size() << ", " << cnt << std::endl;
            cnt = samplequeue.size();
            // erase the remaining buffer
            memset( p_sndbuffer + cnt, 0, datalen - ( cnt * sizeof( VOICE_DATA_FORMAT_TYPE ) ) );
            log_verbose << "playback buffer corrected" << std::endl;
        }
        for ( ; cnt > 0; --cnt )
        {
            *p_sndbuffer++ = samplequeue.front();

//! TODO: remove this later, it's only for ripping to a test wave file
//            s_wavqueue.push( samplequeue.front() );

            samplequeue.pop();
        }

    }
    else
    {
        memset( p_sndbuffer, 0, datalen );
    }

    return FMOD_OK;
}

SoundNode* VoiceReceiver::createSoundNode()
{
    // create a sound
    FMOD_RESULT result;

    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );    
    createsoundexinfo.decodebuffersize  = CODEC_FRAME_SIZE;
    createsoundexinfo.length            = VOICE_SAMPLE_RATE;
    createsoundexinfo.numchannels       = 1;
    createsoundexinfo.defaultfrequency  = VOICE_SAMPLE_RATE;
    createsoundexinfo.format            = VOICE_SOUND_FORMAT;
    createsoundexinfo.pcmreadcallback   = voiceReceiverReadPCM;

    // create a new node
    SoundNode* p_soundnode = new SoundNode();
    result = _p_soundSystem->createStream( 0, mode, &createsoundexinfo, &p_soundnode->_p_sound );
    _p_soundSystem->playSound( FMOD_CHANNEL_FREE, p_soundnode->_p_sound, false, &p_soundnode->_p_channel );    
    p_soundnode->_p_sound->setUserData( static_cast< void* >( p_soundnode ) );
    
    return p_soundnode;
}

void VoiceReceiver::setupNetwork() throw( NetworkSoundExpection )
{
    int channel;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_CHANNEL, channel );

    RNReplicaNet::XPURL xpurl;

    _p_udpTransport = xpurl.FindTransport( "UDP@" )->Allocate();
    _p_udpTransport->Listen( channel );
    std::string url = _p_udpTransport->ExportURL();
    log_debug << "  <- voice receiver started: " << url << std::endl;
}

void VoiceReceiver::update( float deltaTime )
{
    assert( _p_udpTransport && "network transport is not available!" );

    // update sound system
    _p_soundSystem->update();

    static char  s_buffer[ 512 ];
    VoicePaket*  p_data   = NULL;
    unsigned int senderID = 0xFFFFFFFF;

    // check for new connecting senders
	RNReplicaNet::Transport* p_sender = _p_udpTransport->Accept();
    if ( p_sender )
    {
        log_debug << "  <- connection accepted from sender: " << p_sender->GetPeerURL() << std::endl;
        
        SoundNode* p_soundnode = createSoundNode();
        _soundNodeMap[ p_sender ] = p_soundnode;
    }

    SenderMap::iterator p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        SoundNode* p_sendernode = p_beg->second;
        p_sender = p_beg->first;

        // first check for dead senders
        if ( ( p_sendernode->_pingTimer += deltaTime ) > VOICE_LIFESIGN_PERIOD )
        {
            // removing sender 
            log_verbose << "  <- remove sender from receiver list!" << std::endl;

            // remove senders which do not respond anymore
            delete p_sendernode;
            delete p_sender;
            _soundNodeMap.erase( p_beg );
            break;
        }

        // poll incoming data from senders
        if ( p_sender->Recv( s_buffer, 512 ) )
        {
            p_data = reinterpret_cast< VoicePaket* >( s_buffer );
            senderID = p_data->_senderID;
        }
        else
        {
            continue;
        }

        // check if the incoming voice paket belogs to current sender node
        if ( senderID != p_sendernode->_senderID )
        {
            log_error << "internal error on voice paket receiver! sender id mismatch" << std::endl;
            continue;
        }
        switch( p_data->_typeId )
        {
            case NETWORKSOUND_PAKET_TYPE_CON_REQ:
            {
                // get the network id of connecting sender
                int sid = *( reinterpret_cast< int* >( p_data->_p_buffer ) );

                // find the ghost of the sender in voice enabled player list using its network id
                EnPlayer* p_player = NULL;
                EnPlayer* p_senderplayer = NULL;
                const std::vector< yaf3d::BaseEntity* >& players = gameutils::PlayerUtils::get()->getRemotePlayersVoiceChat();
                std::vector< yaf3d::BaseEntity* >::const_iterator p_beg = players.begin(), p_end = players.end();
                for ( ; p_beg != p_end; ++p_beg )
                {
                    p_player = dynamic_cast< EnPlayer* >( *p_beg );
                    if ( p_player->getNetworkID() == sid )
                    {
                        p_senderplayer = p_player;
                    }
                }

                if ( !p_senderplayer )
                    log_error << "Voice Receiver cannot find sending player!" << std::endl;

                p_sendernode->_p_senderPlayer = p_senderplayer;

                p_data->_length   = 0;
                // send back a GRANT if less senders exit then the max allowed
                if ( _soundNodeMap.size() > MAX_SENDERS_CONNECTED )
                {
                    p_data->_typeId   = NETWORKSOUND_PAKET_TYPE_CON_DENY;
                } 
                else
                {
                    p_data->_typeId   = NETWORKSOUND_PAKET_TYPE_CON_GRANT;
                }

                // assign a unique ID to new sender
                ++_senderID;

                // check the id overflow ( this is very uncommon, but we want to be on safe side )
                if ( _senderID > 0xFFFFFFFE )
                    _senderID = 1;

                p_data->_senderID = _senderID;

                // store the sender ID into node
                p_sendernode->_senderID = _senderID;

                p_sender->SendReliable( reinterpret_cast< char* >( p_data ), VOICE_PAKET_HEADER_SIZE  + p_data->_length );

                log_debug << "  <- joining request granted ( " << _soundNodeMap.size() << " )" << std::endl;
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_CON_CLOSE:
            {
                delete p_beg->first;
                delete p_beg->second;
                _soundNodeMap.erase( p_beg );
                p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
                log_debug << "  <- voice sender leaves ( " << _soundNodeMap.size() << " )" << std::endl;
                break;
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_VOICE_DATA:
            {
                // drop obsolete packets
                if ( p_data->_paketStamp < p_sendernode->_lastPaketStamp )
                {
                    std::stringstream msg;
                    msg << "  <- *** paket loss detected (stamps received/current)" << p_data->_paketStamp << "/" << p_sendernode->_lastPaketStamp;
                    log_debug << msg.str() << std::endl;
                    // update our stamp also when a loss has been detected
                    // currently we cannot know if there was an actual loss or an addition overflow occured in stamp variable
                    // one possible way to handle this issue would be that the protocol would support a sort of "reset stamp number"
                    p_sendernode->_lastPaketStamp = p_data->_paketStamp;
                    continue;
                }
                // udpate sound node's stamp
                p_sendernode->_lastPaketStamp = p_data->_paketStamp;

                // attenuate the voice volume considering a cutoff area where the volume is at maximum
                float attenuation = 1.0;
                if ( p_sendernode->_p_senderPlayer )
                {
                    yaf3d::BaseEntity* p_localplayer = gameutils::PlayerUtils::get()->getLocalPlayer();
                    assert( p_localplayer && "local player is not set in PlayerUtils!" );
                    osg::Vec3f diff = p_sendernode->_p_senderPlayer->getPosition() - p_localplayer->getPosition();
                    float distance = diff.length();
                    attenuation = ( distance < _cutoffRange ) ? 1.0f : std::max( 1.0f - ( ( distance - _cutoffRange ) / ( _spotRange - _cutoffRange ) ), 0.0f );
                }

                // decode and enqueue the samples
                if ( !p_sendernode->_p_codec->decode( p_data->_p_buffer, p_data->_length, *p_sendernode->_p_sampleQueue, _outputGain * attenuation ) )
                {
                    log_debug << "decoder queue overrun, flushing queue!" << std::endl;
                    delete p_sendernode->_p_sampleQueue;
                    p_sendernode->_p_sampleQueue = new SoundSampleQueue;
                }
            }
            break;

            case NETWORKSOUND_PAKET_TYPE_CON_PING:
            {
                // send back a pong
                p_data->_length = 0;
                p_data->_typeId = NETWORKSOUND_PAKET_TYPE_CON_PONG;
                p_sender->SendReliable( reinterpret_cast< char* >( p_data ), VOICE_PAKET_HEADER_SIZE  + p_data->_length );
                // reset the ping timer
                p_sendernode->_pingTimer = 0.0f;
            }
            break;

            default:
                ;
        }
    }
}

} // namespace vrc
