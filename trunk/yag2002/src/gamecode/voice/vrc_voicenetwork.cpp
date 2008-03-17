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
 # manager for voice connections in network 
 #
 #   date of creation:  02/12/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicenetwork.h"
#include "../player/vrc_player.h"

namespace vrc
{

VoiceNetwork::VoiceNetwork() :
_hotspotRange2( 400.0f ),
_p_player( NULL ),
_hotspotUpdateTimer( 0.0f )
{
}

VoiceNetwork::~VoiceNetwork()
{
    shutdown();
}

void VoiceNetwork::shutdown()
{
    // currently nothing to do
}

void VoiceNetwork::initialize()
{
    log_debug << " initializing voice network manager ..." << std::endl;
}

void VoiceNetwork::update( float deltaTime )
{
    // update the internat hotspot map every HOTSPOT_UPDATE_PERIOD
    //! TODO: implement a cpu usage friendly hotspot determination considering temporal position changes
    // for now we implement the simplest mechanism: checking the distance every some frames       
    _hotspotUpdateTimer += deltaTime;
    if ( _hotspotUpdateTimer > HOTSPOT_UPDATE_PERIOD )
    {
        _hotspotUpdateTimer = 0.0f;
        updateHotspot();
    }
}

void VoiceNetwork::setHotspotRange( float range )
{
    _hotspotRange2 = range * range;
}

void VoiceNetwork::updateHotspot()
{
    // we assume the local player, once set, will remain during the lifetime of this instance!
    if ( _p_player == NULL )
    {
        _p_player = gameutils::PlayerUtils::get()->getLocalPlayer();
        return;
    }

    _hotspotCenter = _p_player->getPosition();
    VoiceClientMap::const_iterator p_beg = _voiceClientMap.begin(), p_end = _voiceClientMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        float distance = ( _hotspotCenter - p_beg->first->getPosition() ).length2();
        if ( distance < _hotspotRange2 )
        {
            // check for addition to hotspot list
            if ( _hotspotMap.find( p_beg->first ) == _hotspotMap.end() )
            {
                // first add entity to hotspot map
                _hotspotMap[ p_beg->first ] = p_beg->second;

                // now notify registered callbacks for changed hotspot ( addition )
                {
                    std::vector< CallbackHotspotChange* >::iterator p_cbbeg = _cbsHotspot.begin(), p_cbend = _cbsHotspot.end();
                    for ( ; p_cbbeg != p_cbend; ++p_cbbeg )
                        ( *p_cbbeg )->onHotspotChanged( true, p_beg->first );
                }
            }
        }
        else if ( distance > ( _hotspotRange2 + HOTSPOT_RANGE_THRESHOLD2 ) )
        {
            // check for removal from hotspot list
            VoiceClientMap::iterator p_rem = _hotspotMap.find( p_beg->first );
            if ( p_rem != _hotspotMap.end() )
            {
                // first notify registered callbacks for changed hotspot ( removal )
                {
                    std::vector< CallbackHotspotChange* >::iterator p_cbbeg = _cbsHotspot.begin(), p_cbend = _cbsHotspot.end();
                    for ( ; p_cbbeg != p_cbend; ++p_cbbeg )
                        ( *p_cbbeg )->onHotspotChanged( false, p_beg->first );
                }

                // now remove entity from hotspot map
                _hotspotMap.erase( p_rem );
            }
        }
    }
}

const VoiceNetwork::VoiceClientMap& VoiceNetwork::getHotspot()
{
    return _hotspotMap;
}

void VoiceNetwork::updateVoiceClients( yaf3d::BaseEntity* p_playerentity, bool joining )
{
    VoiceClientMap::iterator p_maphit = _voiceClientMap.find( p_playerentity );
    // is player joining or leaving?
    if ( joining )
    {
        if ( p_maphit == _voiceClientMap.end() )
        {
            EnPlayer* p_player = static_cast< EnPlayer* >( p_playerentity );
            assert( ( p_player->isVoiceChatEnabled() ) &&
                    "internal error, player has no voice chat support!" );

            int sid = p_player->getNetworkID();
            _voiceClientMap[ p_playerentity ] = sid ;
        }
    }
    else
    {
        // remove the player from map
        if ( p_maphit != _voiceClientMap.end() )
        {
            _voiceClientMap.erase( p_maphit );
        }
    }
}

void VoiceNetwork::registerCallbackHotspotChanged( CallbackHotspotChange* p_func, bool reg )
{
    bool funcinlist = false;
    std::vector< CallbackHotspotChange* >::iterator p_beg = _cbsHotspot.begin(), p_end = _cbsHotspot.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_func )
        {
            funcinlist = true;
            break;
        }
    }

    // check the registration / deregistration
    assert( !( funcinlist && reg ) && "callback is already registered for getting hotspot changes" );
    assert( !( !funcinlist && !reg ) && "callback has not been previousely registered for getting hotspot changes" );

    if ( reg )
        _cbsHotspot.push_back( p_func );
    else
        _cbsHotspot.erase( p_beg );
}

} // namespace vrc
