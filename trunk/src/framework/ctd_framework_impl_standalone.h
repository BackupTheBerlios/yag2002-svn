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
 # neoengine, framework implementation class for standalone mode
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
 #  09/24/2004 boto       creation of FrameworkImpl
 #
 ################################################################*/


#ifndef _CTD_FRAMEWORK_IMPL_STANDALONE_H_
#define _CTD_FRAMEWORK_IMPL_STANDALONE_H_

#include <ctd_frbase.h>
#include <ctd_framework_impl.h>
#include <ctd_framework_builder.h>

namespace CTD
{

class Widgets;

//! Implementation of Framework for server mode
class FrameworkImplStandalone: public FrameworkImpl
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
        * Game's main loop
        */
        void                                    GameLoop();

        /**
        * Widget system
        */
        Widgets                                 *m_pkWidgets;

    private:

        //! Only FrameworkBuilder is allowed to create and destroy an object of this class 
                                                FrameworkImplStandalone();  

                                                ~FrameworkImplStandalone();

                                                FrameworkImplStandalone( FrameworkImplStandalone& );

        FrameworkImplStandalone&                operator =( FrameworkImplStandalone& );

    friend class FrameworkBuilder< FrameworkImplStandalone >;

};

} // namespace CTD

#endif // _CTD_FRAMEWORK_IMPL_STANDALONE_H_

