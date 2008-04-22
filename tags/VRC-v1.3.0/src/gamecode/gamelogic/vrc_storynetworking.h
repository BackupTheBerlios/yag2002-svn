/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 # networking for story system; this is a server side object.
 #
 #   date of creation:  01/14/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_STORYNETWORKING_H_
#define _VRC_STORYNETWORKING_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_StoryNetworking.h"

namespace vrc
{

class StoryEvent;

class StoryNetworking : _RO_DO_PUBLIC_RO( StoryNetworking )
{
    public:

        //! Create network object.
        explicit                                   StoryNetworking();

        virtual                                    ~StoryNetworking();

        //! Send an event over network It is used by server and clients.
        bool                                        sendEvent( const StoryEvent& event );

    protected:

        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------------------------------------------//

        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        //! Client requests the server to perform an action
        void                                        RPC_ReceiveEvent( tEventData event );

        //-----------------------------------------------------------------------------------//

    friend class _MAKE_RO( StoryNetworking );
};

} // namespace vrc

#endif //_VRC_STORYNETWORKING_H_
