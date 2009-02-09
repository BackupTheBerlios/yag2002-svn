/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # main editor application entry class
 #
 #   date of creation:  01/18/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "guibase.h"
#include "editor.h"
#include "mainframe.h"
#include "gameinterface.h"
#include "appinterface.h"
#include "logwindow.h"
#include "statswindow.h"
#include "editorutils.h"

#include "entityproperties.h"

IMPLEMENT_APP( EditorApp )

IMPLEMENT_CLASS( EditorApp, wxApp )


BEGIN_EVENT_TABLE( EditorApp, wxApp )


END_EVENT_TABLE()


EditorApp::EditorApp() :
 _p_gameInterface( NULL ),
 _p_appInterface( NULL ),
 _p_mainFrame( NULL ),
 _p_logWindow( NULL ),
 _p_statsWindow( NULL )
{
}

bool EditorApp::OnInit()
{
    //! NOTE: on multi-core systems running win32, sometimes a noticable performance drop has been observed
    //        when the application uses more than one cpu for its threads. we take the same cpu as the game.
#ifdef WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo( &sysInfo );
    if ( sysInfo.dwNumberOfProcessors > 1 )
    {
        // take the first cpu for our application
        DWORD_PTR  processAffinityMask = 0x1;
        SetProcessAffinityMask( GetCurrentProcess(), processAffinityMask );
    }
#endif

    // initialize the bitmap resource manager
    BitmapResource::get()->initialize();

    // setup the editor settings
    {
        yaf3d::SettingsPtr settings = yaf3d::SettingsManager::get()->createProfile( EDITOR_SETTINGS_PROFILE, EDITOR_SETTINGS_PROFILE ".cfg" );
        bool            navEnable = true;
        std::string     navMode( "Fly" );
        unsigned int    navSpeed  = 100;
        osg::Vec3f      navPos( 0.0f, 0.0f, 50.0f );
        osg::Vec2f      navRot( 0.0f, -90.0f );
        float           navFOV      = 45.0f;
        float           navNearClip = 0.5f;
        float           navFarClip  = 1000.0f;
        osg::Vec3f      navBkgColor( 0.0f, 0.0f, 0.0f );

        settings->registerSetting( ES_NAV_ENABLE,   navEnable );
        settings->registerSetting( ES_NAV_MODE,     navMode );
        settings->registerSetting( ES_NAV_SPEED,    navSpeed );
        settings->registerSetting( ES_NAV_POSITION, navPos );
        settings->registerSetting( ES_NAV_ROTATION, navRot );
        settings->registerSetting( ES_NAV_FOV,      navFOV );
        settings->registerSetting( ES_NAV_NEARCLIP, navNearClip );
        settings->registerSetting( ES_NAV_FARCLIP,  navFarClip );
        settings->registerSetting( ES_NAV_BKGCOLOR, navBkgColor );

        if ( !settings->load() )
        {
            if ( !settings->store() )
            {
                log_error << "[Editor] cannot create configuration file " << EDITOR_SETTINGS_PROFILE ".cfg"  << std::endl;
            }
            else
            {
                settings->load();
            }
        }
    }

    // first create the game interface, main frame needs it on creation!
    _p_gameInterface = new GameInterface( argv[ 0 ] );

    // create the main window
    _p_mainFrame = new MainFrame( this );
    
    // create the application interface
    _p_appInterface = new AppInterface( _p_mainFrame );

    // now show the app window
    _p_mainFrame->Show( true );

    // create the log window
    _p_logWindow = new LogWindow();
    // add it as sink to yaf3d log system
    yaf3d::yaf3dlog.addSink( "editor", *_p_logWindow );
    _p_logWindow->Show( true );

    // create the stats window
    _p_statsWindow = new StatsWindow();
    _p_statsWindow->Show( true );

    // initialize the application interface
    _p_appInterface->initialize( _p_gameInterface );

    // initialize the game interface
    _p_gameInterface->initialize( _p_appInterface );

    return true;
}

int EditorApp::OnExit()
{
    // stop the game loop thread
    _p_gameInterface->terminate();
    delete _p_gameInterface;

    _p_appInterface->terminate();
    delete _p_appInterface;

    // shutdown the bitmap resource
    BitmapResource::get()->shutdown();

    return wxApp::OnExit();
}

bool EditorApp::loadLevel( const std::string& levelfile )
{
    assert( _p_gameInterface && "no game running!" );

    char* p_filename = new char[ levelfile.length() + 1 ];
    strncpy( p_filename, levelfile.c_str(), levelfile.length() + 1 );

    _p_gameInterface->sendCmd( GameInterface::CMD_LOAD_LEVEL, p_filename );

    _p_mainFrame->setStatus( std::string( "Loading level: " ) + levelfile );

    return true;
}

void EditorApp::unloadLevel()
{
    assert( _p_gameInterface && "no game running!" );

    _p_mainFrame->setStatus( std::string( "Unloading level ..." ) );
    _p_gameInterface->sendCmd( GameInterface::CMD_UNLOAD_LEVEL, NULL );
}

void EditorApp::pauseGame()
{
    assert( _p_gameInterface && "no level file loaded!" );

    //! TODO
}

void EditorApp::continueGame()
{
    assert( _p_gameInterface && "no level file loaded!" );

    //! TODO
}

LogWindow* EditorApp::getLogWindow()
{
    return _p_logWindow;
}

StatsWindow* EditorApp::getStatsWindow()
{
    return _p_statsWindow;
}

GameInterface* EditorApp::getGameInterface()
{
    return _p_gameInterface;
}

AppInterface* EditorApp::getAppInterface()
{
    return _p_appInterface;
}
