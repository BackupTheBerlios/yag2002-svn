/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # neoengine, framework implementation class for server mode
 #
 #
 #   date of creation:  09/24/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  09/24/2004 boto       creation of FrameworkImplServer
 #
 ################################################################*/


#ifndef _CTD_FRAMEWORK_IMPL_SERVER_H_
#define _CTD_FRAMEWORK_IMPL_SERVER_H_

#include <ctd_frbase.h>
#include <ctd_framework_impl.h>
#include <ctd_framework_builder.h>

namespace CTD
{

//! Implementation of Framework for server mode
class FrameworkImplServer: public FrameworkImpl
{

    public: 

        /**
        * Initialize simulation.
        */
        bool                                    Initialize( int iArgc = 0, char** ppcArgv = NULL );

        /**
        * This function starts the game.
        * Call this function after initialization.
        */
        void                                    StartGame();


        /**
        * Shutdown simulation.
        */
        void                                    ExitGame();

        /**
        * Register an entity for pre-rendering callbacks. The server ignores this request.
        */
        void                                    RegisterPreRenderEntity( CTD::BaseEntity *pkEntity ) {}

        /**
        * Register an entity for 2D rendering callbacks. The server ignores this request.
        */
        void                                    Register2DRendering( BaseEntity *pkEntity ) {}

    protected:

        /**
        * Setup networking
        * \return                               Returns true if networking setup was successfull.
        */
        bool                                    SetupNetworking();

        /**
        * Setup render device
        */
        void                                    SetupRenderDevice();

        /**
        * Game's main loop
        */
        void                                    GameLoop();

    private:

        //! Only FrameworkBuilder is allowed to create and destroy an object of this class 
                                                FrameworkImplServer();  

                                                ~FrameworkImplServer();


                                                FrameworkImplServer( FrameworkImplServer& );

        FrameworkImplServer&                    operator =( FrameworkImplServer& );


    friend class FrameworkBuilder< FrameworkImplServer >;

};

} // namespace CTD

#endif // _CTD_FRAMEWORK_IMPL_SERVER_H_

