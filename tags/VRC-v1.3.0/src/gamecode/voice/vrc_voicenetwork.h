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

#ifndef _VRC_VOICENETWORK_H_
#define _VRC_VOICENETWORK_H_

#include <vrc_main.h>
#include "vrc_networksoundimpl.h"

namespace vrc
{

//! Update period for hotspot determination
#define HOTSPOT_UPDATE_PERIOD       1.0f
//! Square of distance threshold for hotspot range checking on voice clients
#define HOTSPOT_RANGE_THRESHOLD2    4.0f

class EnNetworkVoice;

//! Class for managing voice connections in network
//! This class provides a hotspot list of all clients with enabled voice which are in a defined range.
class VoiceNetwork
{
    public:

        //! A map for remote client entities and their accociated session ID
        typedef std::map< yaf3d::BaseEntity*, int >   VoiceClientMap;


                                                    VoiceNetwork();

        virtual                                     ~VoiceNetwork();

        //! Get all clients in hotspot
        const VoiceClientMap&                       getHotspot();

        //! Update the internal client map having voice support.
        //! Call this whenever voice enabled clients have joined to network or left it.
        //! Set 'joining' to true if the player is beeing joined otherwise ( on leaving ) set it to false.
        void                                        updateVoiceClients( yaf3d::BaseEntity* p_playerentity, bool joining );

        //! Class for getting notified whenever the voice hotspot changed
        class CallbackHotspotChange
        {
            public:
                                                        CallbackHotspotChange() {}

                virtual                                 ~CallbackHotspotChange() {}

                //! Callback method called whenever the hotspot changed
                virtual void                            onHotspotChanged( bool joining, yaf3d::BaseEntity* p_entity ) = 0;
        };

        //! Callback registry for changed hotspot.
        //! Use reg = true for registration and reg = false for deregistration
        void                                        registerCallbackHotspotChanged( CallbackHotspotChange* p_func, bool reg = true );

    protected:

        //! Initialize
        void                                        initialize();

        //! Shutdown
        void                                        shutdown();

        //! Update
        void                                        update( float deltaTime );

        //! Set the hotspot range, default is 20.0
        void                                        setHotspotRange( float range );

        //! Check for voice clients in hotspot range
        void                                        updateHotspot();

        //! Square of hotspot range, default is 400
        float                                       _hotspotRange2;

        //! Map of clients supporting voice
        VoiceClientMap                              _voiceClientMap;

        //! Hotspot map
        VoiceClientMap                              _hotspotMap;

        //! Center of hotspot, it's the local player position
        osg::Vec3f                                  _hotspotCenter;

        //! Local player
        yaf3d::BaseEntity*                          _p_player;

        //! Hotspot update timer
        float                                       _hotspotUpdateTimer;

        //! Hotspot changed-callback registrations
        std::vector< CallbackHotspotChange* >       _cbsHotspot;

    friend class EnNetworkVoice;
};

} // namespace vrc

#endif // _VRC_VOICENETWORK_H_
