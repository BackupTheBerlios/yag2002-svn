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
 # keyframe animation tool
 #
 #
 #   date of creation:  10/16/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_animutil.h"

using namespace NeoEngine;
using namespace NeoChunkIO;
using namespace std;

#define BUILD_VERSION       "10/19/2004"


//------------------------------------------------------------------------------------------------------------------------------//
// print help message
void PrintHelp();

// extract the value for given command token
bool GetCommandArgument( int iArgc, char **pcArgv, const string& strToken, string& strValue );

// search for an option in command line argument
bool GetOption( int iArgc, char **pcArgv, const string& strToken );
//------------------------------------------------------------------------------------------------------------------------------//


int main( int argc, char **argv )
{

    printf( "\n" );
    printf( "NeoEngine keyframe animation tool\n" );
    printf( "Build %s\n", BUILD_VERSION );
    printf( "A. Botorabi, botorabi@gmx.net\n" );

    if ( GetOption( argc, argv, "-h" ) || argc < 2 ) {
        PrintHelp();
        return 0;
    }

    string strValue;
    string strInputFile;    bool bInputFile  = false;
    string strOutputFile;   bool bOutputFile = false;

    NeoEngine::Core::Get()->Initialize( argc, argv );
    NeoChunkIO::Core::Get()->Initialize();    

    printf( "\n" );    

    // begin command line processing
    //--------------------------------------------------------------------------//

    if ( GetCommandArgument( argc, argv, "-in", strValue ) ) {
        strInputFile  = strValue;
        bInputFile    = true;
    }

    bool      bPrintIno         = false;
    bool      bBinaryOutput     = true;
    Vector3d  kSetRotation;      bool bSetRotation      = false;
    Vector3d  kRotate;           bool bRotate           = false;
    Vector3d  kSetTranslation;   bool bSetTranslation   = false;
    Vector3d  kTranslate;        bool bTranslate        = false;
    
    KeyFrameAnimUtil             kKfUtil;

    if ( !bInputFile ) {

        printf( "*** no input file defined, use -in < input file >\n\n" );
        PrintHelp();
        return -1;

    }

    printf( " reading animation file...\n" );

    if ( !kKfUtil.Load( strInputFile ) ) {

        printf( "*** could not open keyframe file!\n" );
        return -1;

    }

    unsigned int uiNumKeys        = kKfUtil.GetNumKeyframes();
    string       strAnimNodeName  = kKfUtil.GetAnimationName();

    size_t       uiBeginFrame     = 0;         bool bBeginFrame = false;
    size_t       uiEndFrame       = uiNumKeys; bool bEndFrame   = false;

    if ( GetOption( argc, argv, "-info" ) ) {
        printf( " file information\n" );
        printf( "  animated node name:          %s\n", kKfUtil.GetAnimationName().c_str() );
        printf( "  animation id:                %d\n", kKfUtil.GetAnimID() );
        printf( "  total number of keyframes:   %d\n", kKfUtil.GetNumKeyframes() );
        printf( "  animation length in seconds: %0.2f\n", kKfUtil.GetAnimLength() );
        printf( "\n" );
        bPrintIno = true;
    }

    if ( GetOption( argc, argv, "-ascii" ) ) {
        printf( " setting output format to ascii...\n" );
        bBinaryOutput = false;
    } else {
        if ( GetOption( argc, argv, "-binary" ) ) {
            printf( " setting output format to binary...\n" );
            bBinaryOutput = true;
        } 
    }

    if ( GetCommandArgument( argc, argv, "-out", strValue ) ) {
        strOutputFile = strValue;
        bOutputFile   = true;
    }

    if ( GetCommandArgument( argc, argv, "-beginFrame", strValue ) ) {
        uiBeginFrame = ( size_t )atoi( strValue.c_str() );
        printf( " set begin frame to %d\n", uiBeginFrame );
        kKfUtil.SetBeginFrame( uiBeginFrame );
        bBeginFrame  = true;
    }

    if ( GetCommandArgument( argc, argv, "-endFrame", strValue ) ) {
        uiEndFrame = ( size_t )atoi( strValue.c_str() );
        printf( " set end frame to %d\n", uiEndFrame );
        kKfUtil.SetEndFrame( uiEndFrame );
        bEndFrame  = true;
    }

    if ( ( uiEndFrame > uiNumKeys ) || ( uiEndFrame < uiBeginFrame ) ) {
        printf( "*** end frame must be a value between begin frame and total num of keyframes!\n" );
        NeoChunkIO::Core::Get()->Shutdown();
        NeoEngine::Core::Get()->Shutdown();
        return -1;
    }

    if ( GetCommandArgument( argc, argv, "-trackLength", strValue ) ) {
        printf( " set new track length %0.2f\n", ( float )atof( strValue.c_str() ) );
        kKfUtil.SetAnimLength( ( float )atof( strValue.c_str() ) );
    }

    if ( GetCommandArgument( argc, argv, "-translate", strValue ) ) {
        float fX = 0, fY = 0, fZ = 0;
        if ( sscanf( strValue.c_str(), "%f %f %f", &fX, &fY, &fZ ) != 3 ) {
            printf( "*** operation -translate needs following argument: \"x y z\"\n" );
            return -1;        
        }
        kTranslate = Vector3d( fX, fY, fZ );
        bTranslate = true;
    }

    if ( GetCommandArgument( argc, argv, "-setTranslation", strValue ) ) {
        float fX = 0, fY = 0, fZ = 0;
        if ( sscanf( strValue.c_str(), "%f %f %f", &fX, &fY, &fZ ) != 3 ) {
            printf( "*** operation -setTranslation needs following argument: \"x y z\"\n" );
            return -1;
        }
        kSetTranslation = Vector3d( fX, fY, fZ );
        bSetTranslation = true;
    }

    if ( GetCommandArgument( argc, argv, "-rotate", strValue ) ) {
        float fX = 0, fY = 0, fZ = 0;
        if ( sscanf( strValue.c_str(), "%f %f %f", &fX, &fY, &fZ ) != 3 ) {
            printf( "*** operation -rotate needs following argument: \"x y z\"\n" );
            return -1;        
        }
        kRotate = Vector3d( fX, fY, fZ );
        bRotate = true;
    }

    if ( GetCommandArgument( argc, argv, "-setRotation", strValue ) ) {
        float fX = 0, fY = 0, fZ = 0;
        if ( sscanf( strValue.c_str(), "%f %f %f", &fX, &fY, &fZ ) != 3 ) {
            printf( "*** operation -setRotation needs following argument: \"x y z\"\n" );
            return -1;        
        }
        kSetRotation = Vector3d( fX, fY, fZ );
        bSetRotation = true;
    }

    // semantic check of commands
    if ( uiBeginFrame > uiEndFrame ) {
        printf( "*** begin frame must be a value between 0 and end frame!\n" );
        NeoChunkIO::Core::Get()->Shutdown();
        NeoEngine::Core::Get()->Shutdown();
        return -1;
    }

    printf( "\n begin operations...\n\n" );

    // execute operations
    //--------------------------------------------------------------------------------------------------//

    // first of all execute the delete operation
    if ( GetOption( argc, argv, "-deleteFrames" ) ) {
        printf( " deleting frames between %d and %d ...\n", uiBeginFrame, uiEndFrame );
        if ( !bBeginFrame || !bEndFrame ) {
            printf( "*** for operation -deleteFrames you must define begin and end frame indices, use -begin_frame and -end_frame\nn" );
            NeoChunkIO::Core::Get()->Shutdown();
            NeoEngine::Core::Get()->Shutdown();            
            return -1;
        }
        // delete frames
        kKfUtil.CmdDeleteFrames( uiBeginFrame, uiEndFrame );
    }

    if ( bSetRotation ) {
        printf( " set rotation to %0.2f %0.2f %0.2f ...\n", kSetRotation.x, kSetRotation.y, kSetRotation.z );          
        kKfUtil.CmdSetRotation( kSetRotation );
    }

    if ( kRotate.x != 0 ) {
        printf( " rotating %0.2f degree about X axis ...\n", kRotate.x );
        kKfUtil.CmdRotate( Vector3d( 1, 0, 0 ), kRotate.x );
    }
    if ( kRotate.y != 0 ) {            
        printf( " rotating %0.2f degree about Y axis ...\n", kRotate.y );
        kKfUtil.CmdRotate( Vector3d( 0, 1, 0 ), kRotate.y );
    }
    if ( kRotate.z != 0 ) {
        printf( " rotating %0.2f degree about Z axis ...\n", kRotate.z );          
        kKfUtil.CmdRotate( Vector3d( 0, 0, 1 ), kRotate.z );
    }

    if ( bSetTranslation ) {
        printf( " set translation %0.2f %0.2f %0.2f ...\n", kSetTranslation.x, kSetTranslation.y, kSetTranslation.z );          
        kKfUtil.CmdSetTranslation( kSetTranslation );
    }
    if ( bTranslate ) {
        printf( " translate %0.2f %0.2f %0.2f ...\n", kTranslate.x, kTranslate.y, kTranslate.z );          
        kKfUtil.CmdTranslate( kTranslate );
    }

    //--------------------------------------------------------------------------------------------------//

    printf( "\n" );

    if ( bOutputFile ) {
        // write output file
        printf( " writing output file %s ...\n", strOutputFile.c_str() );
        if ( !kKfUtil.Store( strOutputFile, bBinaryOutput ) ) {            
            printf( "*** cannot create output file %s\n\n", strOutputFile );
            NeoChunkIO::Core::Get()->Shutdown();
            NeoEngine::Core::Get()->Shutdown();
            return -1;
        }
    } else {            
        if ( !bPrintIno ) {        
            printf( "*** no output file defined, the results of manipulations are lost!\n" );
        }

    }

    NeoChunkIO::Core::Get()->Shutdown();
    NeoEngine::Core::Get()->Shutdown();

    return 0;
}

void PrintHelp()
{

    printf(
            " use: animtool.exe -in < input file > -out < output file > [ options ]\n"
            " options:\n\n"
            "  -h                             print this message\n"
            "\n"
            "information\n"
            "  -info                          print information about animation file\n"
            "\n"
            "output formats\n"
            "  -ascii                         write ascii output file\n"
            "  -binary                        write binary output file\n"
            "\n"
            "frame manipulation\n"
            "  -beginFrame < frame index >    set begin of selection,\n"
            "                                  the default is begin of the animation\n"
            "  -endFrame < frame index >      set end of selection,\n"
            "                                  the default is end of the whole animation\n"
            "  -deleteFrames                  delete selected frames from track,\n"
            "                                  use -beginFrame and -endFrame\n"
            "  -trackLength < time >          set new track length in seconds\n"
            "\n"
            "translation of selected frames\n"
            "  -translate < \"x y z\" >         translate\n"
            "  -setTranslation < \"x y z\" >    set translation\n"
            "\n"
            "rotation of selected frames\n"
            "  -rotate < \"x y z\" >            rotate about X, Y, and Z axes in XYZ order\n"
            "  -setRotation < \"x y z\" >       set rotation\n"
            "\n"
          );


}



// extract the value for given option token and command arguments
bool GetCommandArgument( int iArgc, char **pcArgv, const string& strToken, string& strValue )
{

    for ( int iArgCnt = 1; iArgCnt < iArgc; iArgCnt++ ) {

        if ( string( pcArgv[ iArgCnt ] ) == strToken ) {
        
            iArgCnt++;
            if ( iArgCnt > iArgc ) {

                printf( " *** Program command '%s' needs an argument!\n\n", strToken );
                PrintHelp();

                return false;
            } 

            strValue = pcArgv[ iArgCnt ];
            return true;

        }
    }

    return false;

}

// search for an option in command line argument
bool GetOption( int iArgc, char **pcArgv, const string& strToken )
{

    for ( int iArgCnt = 1; iArgCnt < iArgc; iArgCnt++ ) {

        if ( string( pcArgv[ iArgCnt ] ) == strToken ) {

            return true;

        }
    }

    return false;

}
