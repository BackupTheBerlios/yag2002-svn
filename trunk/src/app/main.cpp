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
 # main entry
 #
 #   date of creation:  11/15/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include <ctd_frbase.h>
#include <ctd_framework.h>
#include <ctd_framework_impl_server.h>
#include <ctd_framework_impl_client.h>
#include <ctd_framework_impl_standalone.h>
#include <ctd_framework_builder.h>

// staticaly link plugins
#include "ctd_pluginlinks.h"

using namespace CTD;

int StartServer( int iArgc, char **ppcArgv );
int StartClient( int iArgc, char **ppcArgv );
int StartStandalone( int iArgc, char **ppcArgv );

int main( int iArgc, char **ppcArgv )
{

    enum { eSERVER, eCLIENT, eSTANDALONE } eGameMode;
    eGameMode = eCLIENT;

    //parse arguments for -server or -client
    //-----------------------------------------------------//
    for( int iArg = 0; iArg < iArgc; ++iArg )
    {
        if( !strcmp( ppcArgv[ iArg ], "-server" ) )
        {
            eGameMode = eSERVER;
        }
        else
        if( !strcmp( ppcArgv[ iArg ], "-client" ) )
        {
            eGameMode = eCLIENT;
        }
        else
        if( !strcmp( ppcArgv[ iArg ], "-standalone" ) )
        {
            eGameMode = eSTANDALONE;
        }

    }

    int iResult = 0;

//	try {

    switch( eGameMode ) {

        case eSERVER:

            iResult = StartServer( iArgc, ppcArgv );
            break;

        case eCLIENT:

            iResult = StartClient( iArgc, ppcArgv );
            break;

        case eSTANDALONE:

            iResult = StartStandalone( iArgc, ppcArgv );
            break;

        default:
            ;

    }

    return iResult;

    //}
    //catch( ...) {

    //    printf( "*** unexpected exception occured, exit..." );

    //    // if we use networking then do an emergency shutdown
    //    if ( Framework::Get() ) {

    //        Framework::Get()->EmergencyShutdownNetworkDevice();
    //    
    //    }
    //}

}

int StartServer( int iArgc, char **ppcArgv )
{

    CTD::FrameworkBuilderServer *pkFrameworkBuilder = CTD::FrameworkBuilderServer::Get();

    try {

    // create the framework
    pkFrameworkBuilder->Create();

    // initialize the framework
    pkFrameworkBuilder->Initialize( iArgc, ppcArgv );

    Framework*  pkFramework = pkFrameworkBuilder->Finalize();

    // start the game
    pkFramework->StartGame();

    pkFrameworkBuilder->Shutdown();


    } catch ( FrameworkException pkException ) {

        printf( "\n program exception while starting the server, reason: %s\n", pkException.GetMsg().c_str() );
        pkFrameworkBuilder->Shutdown();
        return -1;
    }

    return 0;

}

int StartClient( int iArgc, char **ppcArgv )
{

    CTD::FrameworkBuilderClient *pkFrameworkBuilder = CTD::FrameworkBuilderClient::Get();

    try {

    // create the framework
    pkFrameworkBuilder->Create();

    // initialize the framework
    pkFrameworkBuilder->Initialize( iArgc, ppcArgv );

    Framework*  pkFramework = pkFrameworkBuilder->Finalize();

    // start the game
    pkFramework->StartGame();

    pkFrameworkBuilder->Shutdown();


    } catch ( FrameworkException pkException ) {

        printf( "\n program exception while starting the client, reason: %s\n", pkException.GetMsg().c_str() );
        pkFrameworkBuilder->Shutdown();
        return -1;
    }

    return 0;

}

int StartStandalone( int iArgc, char **ppcArgv )
{

    CTD::FrameworkBuilderStandalone *pkFrameworkBuilder = CTD::FrameworkBuilderStandalone::Get();

    try {

    // create the framework
    pkFrameworkBuilder->Create();

    // initialize the framework
    pkFrameworkBuilder->Initialize( iArgc, ppcArgv );

    Framework*  pkFramework = pkFrameworkBuilder->Finalize();

    // start the game
    pkFramework->StartGame();

    pkFrameworkBuilder->Shutdown();


    } catch ( FrameworkException pkException ) {

        printf( "\n program exception while starting the game in standalone mode, reason: %s\n", pkException.GetMsg().c_str() );
        pkFrameworkBuilder->Shutdown();
        return -1;
    }

    return 0;

}

