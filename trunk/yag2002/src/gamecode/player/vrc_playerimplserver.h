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
 # player implementation for server mode
 #
 #   date of creation:  05/31/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/31/2005 boto       creation of PlayerImplServer
 #
 ################################################################*/

#ifndef _VRC_PLAYERIMPLSERVER_H_
#define _VRC_PLAYERIMPLSERVER_H_

#include <vrc_main.h>
#include "vrc_playerimpl.h"

namespace vrc
{

//! Player implementation for game mode Server ( see framework class yaf3d::GameState )
class PlayerImplServer : public BasePlayerImplementation
{
    public:

        explicit                                    PlayerImplServer( EnPlayer* player );

        virtual                                     ~PlayerImplServer();


        //! Initialize
        void                                        initialize();

        //! Post-initialize
        void                                        postInitialize();

        //! Update
        void                                        update( float deltaTime );

        //! Implementation's notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );
};

} // namespace vrc

#endif // _VRC_PLAYERIMPLSERVER_H_
