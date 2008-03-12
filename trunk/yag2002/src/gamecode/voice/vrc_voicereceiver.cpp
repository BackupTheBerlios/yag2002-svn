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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicereceiver.h"
#include "vrc_voicetestutils.h"
#include "vrc_netvoice.h"
#include "vrc_codec.h"
#include "../player/vrc_player.h"
#include "RNXPURL/Inc/XPURL.h"
#include "RNXPURL/Inc/Transport.h"
#include <RNLobby/Inc/NATDetectServer.h>


namespace vrc
{

// codec samples per frame
#define CODEC_FRAME_SIZE            320

// maximal allowed senders connected to the receiver
#define MAX_SENDERS_CONNECTED       5

//! TODO remove this test ripper
//**************************************************************
//#define VOICE_RIP

#ifdef VOICE_RIP
    std::queue< short > s_wavqueue;
#endif
//**************************************************************

//! Threading class for updating the receiver
class ReceiverUpdateThread: public OpenThreads::Thread
{
    public:

                                              ReceiverUpdateThread( BaseNetworkSoundImplementation* p_receiver ) :
                                               _terminate( false ),
                                               _p_receiver( p_receiver )
                                              {
                                              }

         virtual ~                            ReceiverUpdateThread() {}

        //! Run the update loop
        void                                  run()
                                              {
                                                  osg::Timer   timer;
                                                  osg::Timer_t curTick = timer.tick();
                                                  osg::Timer_t lastTick = curTick;
                                                  float        deltaTime = 0.015f;
                                                  while ( !_terminate )
                                                  {
                                                      // sleep for 50 miliseconds, the sound system needs at least 8 updates per second! we consider also some processing time
                                                      OpenThreads::Thread::microSleep( 50000 );

                                                      curTick = timer.tick();
                                                      deltaTime = timer.delta_s( lastTick, curTick );
                                                      lastTick  = curTick;

                                                      // update the receiver
                                                      _p_receiver->update( deltaTime );
                                                  }
                                              }

        void                                  terminate()
                                              {
                                                  _terminate = true;
                                              }

        bool                                  _terminate;

        BaseNetworkSoundImplementation*       _p_receiver;
};


OpenThreads::Mutex VoiceReceiver::_s_sndDataMutex;

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
                                                }

                                                ~SoundNode()
                                                {
                                                    if ( _p_sound )
                                                        _p_sound->release();

                                                    delete _p_sampleQueue;
                                                    delete _p_codec;
                                                }

        //! Queue for raw sound data
        SoundSampleQueue*                       _p_sampleQueue;

        //! Encoder / decoder, use only the decoder!
        NetworkSoundCodec*                      _p_codec;

        //! Sound object
        FMOD::Sound*                            _p_sound;
        
        //! Sound channel
        FMOD::Channel*                          _p_channel;

        //! Unique sender ID
        unsigned int                            _senderID;

        //! Timer for alive-signaling
        float                                   _pingTimer;

        //! Last received valid voice paket stamp
        unsigned int                           _lastPaketStamp;

        //! Ghost of sending sound data
        EnPlayer*                               _p_senderPlayer;
};

VoiceReceiver::VoiceReceiver( EnNetworkVoice* p_network, VoiceTransport* p_transport ) :
_p_transport( p_transport ),
_p_soundSystem( NULL ),
_p_network( p_network ),
_p_receiverUpdateThread( NULL ),
_lifesignCheck( 0.0f ),
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
    log_debug << "Voice receiver: shutting down ..." << std::endl;

    // first shutdown the updater thread
    if ( _p_receiverUpdateThread )
    {
         _p_receiverUpdateThread->terminate();
         while ( _p_receiverUpdateThread->isRunning() )
             OpenThreads::Thread::microSleep( 1000 );

         delete _p_receiverUpdateThread;
         _p_receiverUpdateThread = NULL;
    }

    if ( _p_transport )
    {
        _p_transport->registerReceiver( this, false );
        _p_transport = NULL;
    }

    // clean up sender / sound node map
    SenderMap::iterator p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        delete p_beg->second;
    }
    _soundNodeMap.clear();

    if ( _p_soundSystem )
    {
        _p_soundSystem->close();
        _p_soundSystem->release();
        _p_soundSystem = NULL;
    }

//! TODO: remove this rip code later
//*******************************
#ifdef VOICE_RIP
    // write out the ripped wav file
    {
#if 0
        // create a pure sine
        {
            std::queue< short > wavqueue;
            for ( int i = 0; i < 16000 * 10; i++)
            {
                wavqueue.push( ( short ) 50000.0f * sinf( float( i ) / float( 8 ) * 2.0f * 3.14159265f ) );
            }
            WaveWriterPCM16 wavwriter( 16000, WaveWriterPCM16::MONO );
            wavwriter.write( yaf3d::Application::get()->getMediaPath() + std::string( "sine-2khz-mono.wav" ), wavqueue );
        }
#endif


        if ( !s_wavqueue.empty() )
        {
            // write out the rip
            WaveWriterPCM16 wavwriter( VOICE_SAMPLE_RATE, WaveWriterPCM16::MONO );
            wavwriter.write( yaf3d::Application::get()->getMediaPath() + std::string( "sender0-rip-receiver.wav" ), s_wavqueue );
        }
    }
#endif
//*******************************
}

void VoiceReceiver::initialize() throw( NetworkSoundException )
{
    try
    {
        setupSound();
        setupNetwork();
    }
    catch ( const NetworkSoundException& e )
    {
        log_error << "Error initializing voice receiver " << std::endl;
        log_error << "   reason: " << e.what() << std::endl;
        return;
    }

     if ( !_p_receiverUpdateThread )
     {
        // start the receiver update thread
        _p_receiverUpdateThread = new ReceiverUpdateThread( this );
        _p_receiverUpdateThread->start();
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

void VoiceReceiver::setupSound() throw( NetworkSoundException )
{
    FMOD_RESULT result;
    result = FMOD::System_Create( &_p_soundSystem );
    if ( result != FMOD_OK )
        throw NetworkSoundException( "Cannot create sound system" );

    unsigned int version;
    result = _p_soundSystem->getVersion( &version );

    if ( version < FMOD_VERSION )
        throw NetworkSoundException( "FMOD version conflict." );

    result = _p_soundSystem->init( MAX_SENDERS_CONNECTED, /*FMOD_INIT_STREAM_FROM_UPDATE*/ FMOD_INIT_NORMAL, 0 );
}

static FMOD_RESULT F_CALLBACK voiceReceiverReadPCM( FMOD_SOUND* p_sound, void* p_data, unsigned int datalen )
{
    FMOD::Sound*    p_fmodsound = reinterpret_cast< FMOD::Sound* >( p_sound );
    void*           p_userdata;
    p_fmodsound->getUserData( &p_userdata );
    SoundNode*      p_soundnode = reinterpret_cast< SoundNode* >( p_userdata );

    if ( !p_soundnode )
        return FMOD_OK;

//! TODO: remove this
static int sequence = 0;
sequence++;

    VOICE_DATA_FORMAT_TYPE*   p_sndbuffer = reinterpret_cast< VOICE_DATA_FORMAT_TYPE* >( p_data );
    unsigned int              cnt         = datalen / sizeof( VOICE_DATA_FORMAT_TYPE );

    VoiceReceiver::getSoundDataMutex().lock();

    // get the proper sample queue and put the samples into raw sound buffer
    SoundSampleQueue& samplequeue = *p_soundnode->_p_sampleQueue;
    if ( !samplequeue.empty() )
    {
        // handle buffer underrun
        if ( samplequeue.size() < cnt )
        {
            log_verbose << "playback buffer underrun: " << samplequeue.size() << ", " << cnt << ", frame " << sequence << std::endl;
            cnt = samplequeue.size();
            // erase the remaining buffer
            memset( p_sndbuffer + cnt, 0, datalen - ( cnt * sizeof( VOICE_DATA_FORMAT_TYPE ) ) );
            log_verbose << "playback buffer corrected" << std::endl;
        }

        for ( ; cnt > 0; --cnt )
        {
            *p_sndbuffer++ = samplequeue.front();

//! TODO: remove this later, it's only for ripping to a wave file
#ifdef VOICE_RIP
            s_wavqueue.push( samplequeue.front() );
#endif

            samplequeue.pop();
        }

    }
    else
    {
        memset( p_sndbuffer, 0, datalen );
    }

    VoiceReceiver::getSoundDataMutex().unlock();

    return FMOD_OK;
}

SoundNode* VoiceReceiver::createSoundNode()
{
    // create a sound
    FMOD_RESULT result;

    FMOD_MODE               mode = FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL;
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    memset( &createsoundexinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
    createsoundexinfo.decodebuffersize  = VOICE_SAMPLE_RATE / 8;           // this means we need at least 8 playback updates per second
    createsoundexinfo.cbsize            = sizeof( FMOD_CREATESOUNDEXINFO );
    createsoundexinfo.length            = VOICE_SAMPLE_RATE * 2;
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

void VoiceReceiver::setupNetwork() throw( NetworkSoundException )
{
    // register us as voice packet receiver
    _p_transport->registerReceiver( this, true );
}

static int pakets = 0;
static float t = 0.0f;

void VoiceReceiver::update( float deltaTime )
{
//! TODO: remove this
#if 0
t += deltaTime;
if ( t > 1.0f )
{
    log_verbose << "time: " << t << ", packets received: " << pakets << std::endl;
    t -= 1.0f;
    pakets = 0;
}
#endif
    // update the sound system
    _p_soundSystem->update();

    // update the transport layer in this context
    _p_transport->update( deltaTime );

    _lifesignCheck += deltaTime;
    if ( _lifesignCheck < ( VOICE_LIFESIGN_PERIOD * 0.5f ) )
        return;

    // check senders' lifesign
    SenderMap::iterator p_beg = _soundNodeMap.begin(), p_end = _soundNodeMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        SoundNode* p_sendernode = p_beg->second;

        // first check for dead senders
        if ( ( p_sendernode->_pingTimer += _lifesignCheck ) > VOICE_LIFESIGN_PERIOD )
        {
            // removing sender 
            log_verbose << "  -> remove sender from receiver list!" << std::endl;

            // remove senders which do not respond anymore; this call modifies _soundNodeMap!
            destroyConnection( p_beg->first );
            break;
        }
    }

    _lifesignCheck = 0.0f;
}

void VoiceReceiver::destroyConnection( unsigned int senderID )
{
    SenderMap::iterator p_conmap = _soundNodeMap.find( senderID );
    if ( p_conmap == _soundNodeMap.end() )
        return;

    SoundNode* p_node = p_conmap->second;

    log_verbose << "  -> destroying sender with ID " << p_node->_senderID << std::endl;

    _p_network->destroySender( senderID );

    delete p_node;

    _soundNodeMap.erase( p_conmap );
}

bool VoiceReceiver::recvPacket( VoicePaket* p_packet, const RNReplicaNet::XPAddress& senderaddr )
{
    // check for new connecting senders, sender ID 0 means initial connection
    SenderMap::iterator p_node = _soundNodeMap.find( p_packet->_senderID ), p_end = _soundNodeMap.end();
    if ( p_node == p_end )
    {
        // if it is not a connection request then the packet is not meant for us
        if ( p_packet->_typeId != VOICE_PAKET_TYPE_CON_REQ )
        {
            return false;
        }

        log_debug << "  -> connection accepted from sender: " << senderaddr.Export() << std::endl;

        // get the network id of connecting sender
        int sid = *( reinterpret_cast< int* >( p_packet->_p_buffer ) );

        // check for local loopbacks
        if ( sid == yaf3d::NetworkDevice::get()->getSessionID() )
        {
            log_warning << "  -> local loopback detected!" << std::endl;
            return true;
        }

        // find the ghost of the sender in voice enabled player list using its network id
        EnPlayer* p_player = NULL;
        EnPlayer* p_senderplayer = NULL;
        const std::vector< yaf3d::BaseEntity* >& players = gameutils::PlayerUtils::get()->getRemotePlayersVoiceChat();
        std::vector< yaf3d::BaseEntity* >::const_iterator p_beg = players.begin(), p_end = players.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            p_player = dynamic_cast< EnPlayer* >( *p_beg );
            log_verbose << "Voice Receiver: searching for sending player with SID: " << sid << ", cur player SID: " << p_player->getNetworkID() << std::endl;
            if ( p_player->getNetworkID() == sid )
            {
                p_senderplayer = p_player;
            }
        }

        if ( !p_senderplayer )
        {
            log_error << "Voice Receiver cannot find sending player!" << std::endl;
            return true;
        }

        // create a new sound node
        SoundNode* p_soundnode = createSoundNode();
        p_soundnode->_p_senderPlayer = p_senderplayer;

        p_packet->_length = 0;

        // send back a GRANT if less senders exist then the max allowed
        if ( _soundNodeMap.size() > MAX_SENDERS_CONNECTED )
        {
            p_packet->_senderID = 0;
            p_packet->_typeId   = VOICE_PAKET_TYPE_CON_DENY;

            log_verbose << "  -> joining request denied ( " << p_packet->_senderID << " )" << std::endl;
        } 
        else
        {
            // assign a unique ID to new sender
            ++_senderID;

            // check the id overflow
            if ( _senderID > 0xFFFFFFFE )
                _senderID = 1;

            // store the sender ID into node
            p_soundnode->_senderID = _senderID;

            // add the new sender to internal lookup
            _soundNodeMap[ _senderID ] = p_soundnode;

            p_packet->_typeId   = VOICE_PAKET_TYPE_CON_GRANT;
            p_packet->_senderID = _senderID;

            log_verbose << "  -> joining request granted ( " << p_packet->_senderID << " )" << std::endl;
        }

        p_packet->_length = 0;

        _p_transport->getSocket()->Send( reinterpret_cast< char* >( p_packet ), VOICE_PAKET_HEADER_SIZE  + p_packet->_length, senderaddr );

        log_verbose << "  -> count of members in voice group ( " << _soundNodeMap.size() << " )" << std::endl;

        return true;
    }

    SoundNode* p_soundnode = p_node->second;

    bool handled = true;

    switch( p_packet->_typeId )
    {
        case VOICE_PAKET_TYPE_CON_CLOSE:
        {
            // remove senders which do not respond anymore; this call modifies _soundNodeMap!
            log_debug << "  -> voice sender leaves ( " << _soundNodeMap.size() << " )" << std::endl;

            destroyConnection( p_soundnode->_senderID );
        }
        break;

        case VOICE_PAKET_TYPE_VOICE_DATA:
        {
            // drop obsolete packets
            if ( p_packet->_paketStamp != p_soundnode->_lastPaketStamp + 1 )
            {
                std::stringstream msg;
                msg << "  -> *** paket loss detected (stamps received/current)" << p_packet->_paketStamp << "/" << p_soundnode->_lastPaketStamp;
                log_debug << msg.str() << std::endl;
                // update our stamp also when a loss has been detected
                // currently we cannot know if there was an actual loss or an addition overflow occured in stamp variable
                // one possible way to handle this issue would be that the protocol would support a sort of "reset stamp number"
                p_soundnode->_lastPaketStamp = p_packet->_paketStamp;
                break;
            }
            // udpate sound node's stamp
            p_soundnode->_lastPaketStamp = p_packet->_paketStamp;

            // attenuate the voice volume considering a cutoff area where the volume is at maximum
            float attenuation = 1.0;
            if ( p_soundnode->_p_senderPlayer )
            {
                yaf3d::BaseEntity* p_localplayer = gameutils::PlayerUtils::get()->getLocalPlayer();
                assert( p_localplayer && "local player is not set in PlayerUtils!" );
                osg::Vec3f diff = p_soundnode->_p_senderPlayer->getPosition() - p_localplayer->getPosition();
                float distance = diff.length();
                attenuation = ( distance < _cutoffRange ) ? 1.0f : std::max( 1.0f - ( ( distance - _cutoffRange ) / ( _spotRange - _cutoffRange ) ), 0.0f );
            }

            _s_sndDataMutex.lock();
            {
//! TODO: remove this
#if 0
log_verbose << "recv: " << p_packet->_length << std::endl;
pakets++;
#endif
                // decode and enqueue the samples
                if ( !p_soundnode->_p_codec->decode( p_packet->_p_buffer, p_packet->_length, *p_soundnode->_p_sampleQueue, _outputGain * attenuation ) )
                {
                    log_debug << "  -> decoder queue overrun, flushing queue!" << std::endl;
                    delete p_soundnode->_p_sampleQueue;
                    p_soundnode->_p_sampleQueue = new SoundSampleQueue;
                }
            }
            _s_sndDataMutex.unlock();
        }
        break;

        case VOICE_PAKET_TYPE_CON_PING:
        {
            // send back a pong
            p_packet->_length = 0;
            p_packet->_typeId = VOICE_PAKET_TYPE_CON_PONG;
            _p_transport->getSocket()->Send( reinterpret_cast< char* >( p_packet ), VOICE_PAKET_HEADER_SIZE  + p_packet->_length, senderaddr );
            // reset the ping timer
            p_soundnode->_pingTimer = 0.0f;
        }
        break;

        default:
            handled = false;
    }

    return handled;
}

} // namespace vrc
