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
 # neoengine, framework builder for all game modes
 #
 #
 #   date of creation:  09/25/2004
 #
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  09/24/2005 boto       creation of FrameworkBuilder
 #
 ################################################################*/


#ifndef _CTD_FRAMEWORK_BUILDER_H_
#define _CTD_FRAMEWORK_BUILDER_H_

#include <ctd_framework.h>

#include <string>

namespace CTD
{

class FrameworkImplClient;
class FrameworkImplServer;
class FrameworkImplStandalone;


//! Framework builder class
/**
* Use this class with an appropriate framework implementation class as template argument in order to build a framework.
* Current implementations are: FrameworkImplServer, FrameworkImplClient, and FrameworkImplStandalone.
* Alternatively you can use the following types:
*
* FrameworkBuilderClient         is the same as FrameworkBuilder< FrameworkImplClient > \n
* FrameworkBuilderServer         is the same as FrameworkBuilder< FrameworkImplServer > \n
* FrameworkBuilderStandalone     is the same as FrameworkBuilder< FrameworkImplStandalone > \n
* \n
* Then call the methods Create, Initialize, and Finalize in mentioned order. When exiting the application call Shutdown.
*/

template< class FrameworkImplementationT >
class FrameworkBuilder
{
    
    public:

        /**
        * Create framework
        */
        void                                    Create();


        /**
        * Initialize framework
        */
        void                                    Initialize( int iArgc, char **ppArgv );

        /**
        * Finalize framework building
        */
        Framework*                              Finalize();

        /**
        * Shutdown game
        */
        void                                   Shutdown();

        /**
        * Get the singlton instance of framework core
        */
        static FrameworkBuilder*                Get()
                                                {
                                                    if ( s_pkSingletonFrameworkBuilder == NULL ) s_pkSingletonFrameworkBuilder = new FrameworkBuilder;
                                                    return s_pkSingletonFrameworkBuilder;
                                                }


    private:
        

                                                FrameworkBuilder();
                                                
                                                ~FrameworkBuilder();

        /**
        * Loads engine configuration
        */
        bool                                    LoadConfig();

        // singleton instance
        static FrameworkBuilder*                s_pkSingletonFrameworkBuilder;

        //! Framework implementation singleton instance which is created by this builder class
        FrameworkImplementationT                *m_pkFrameworkImpl;

        //! Framework bridge singleton instance which is created by this builder class
        Framework                               *m_pkFramework;

        enum 
        { 
            eNone,
            eCreated,
            stateInitialized, 
            eBuilt 
        }                                       m_stateBuild;

};

//! Declaration of FrameworkBuilders
typedef FrameworkBuilder< FrameworkImplClient >     FrameworkBuilderClient;
typedef FrameworkBuilder< FrameworkImplServer >     FrameworkBuilderServer;
typedef FrameworkBuilder< FrameworkImplStandalone > FrameworkBuilderStandalone;

#include "ctd_framework_builder_inl.h"

} // namespace CTD

#endif // _CTD_FRAMEWORK_BUILDER_H_
