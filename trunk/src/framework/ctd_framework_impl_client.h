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
 # neoengine, framework implementation class for client mode
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
 #  09/24/2004 boto       creation of FrameworkImplClient
 #
 ################################################################*/


#ifndef _CTD_FRAMEWORK_IMPL_CLIENT_H_
#define _CTD_FRAMEWORK_IMPL_CLIENT_H_


#include <ctd_frbase.h>
#include <ctd_framework_impl.h>
#include <ctd_framework_builder.h>

namespace CTD
{

class Widgets;

//! Implementation of Framework for client mode
class FrameworkImplClient: public FrameworkImpl
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
        * Register an entity for pre-rendering callbacks. The entity function PreRender will be called in this rendering phase.
        */
        void                                    RegisterPreRenderEntity( CTD::BaseEntity *pkEntity ) { m_vpkPreRenderEntities.push_back( pkEntity ); }

        /**
        * Register an entity for 2D rendering callbacks. The entity function Render2D will be called in 2D rendering phase.
        */
        void                                    Register2DRendering( BaseEntity *pkEntity ) { m_vpk2DRenderEntities.push_back( pkEntity ); }

    protected:

        /**
        * Setup networking
        * \param iClientPort                    Client port
        * \param iServerPort                    Server port
        * \param strServerIP                    Server IP
        * \param strNodeName                    Network node name
        * \return                               Returns true if networking setup was successfull.
        */
        bool                                    InitiateClientNetworking( int iClientPort, int iServerPort, const std::string& strServerIP, const std::string& strNodeName );

        /**
        * Start networking for client mode. Call this method after a successful initiation of client networking by calling 'InitiateClientNetworking'.
        * \return                               false if an error occures
        */
        bool                                    StartClientNetworking();

        /**
        * Shutdown networking for client mode.
        */
        void                                    ShutdownClientNetworking();

        /**
        * Game's main loop
        */
        void                                    GameLoop();

        /**
        * Widget system
        */
        Widgets                                 *m_pkWidgets;

        //! Client networking states
        enum { stateOff, stateInitiated, stateStarted } m_eClientNetworking;

    private:
    
        //! Only FrameworkBuilder is allowed to create and destroy an object of this class 
                                                FrameworkImplClient();  

                                                ~FrameworkImplClient();


                                                FrameworkImplClient( FrameworkImplClient& );

        FrameworkImplClient&                    operator =( FrameworkImplClient& );


    friend class FrameworkBuilder< FrameworkImplClient >;
};

} // namespace CTD

#endif // _CTD_FRAMEWORK_IMPL_CLIENT_H_

