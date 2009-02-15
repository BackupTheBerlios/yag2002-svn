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

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <vrc_main.h>
#include "guibase.h"


//! Editor's settings profile name
#define EDITOR_SETTINGS_PROFILE     "editor"
//! Setting tokens
#define ES_NAV_ENABLE               "navEnable"
#define ES_NAV_SPEED                "navSpeed"
#define ES_NAV_POSITION             "navPosition"
#define ES_NAV_ROTATION             "navRotation"
#define ES_NAV_FOV                  "navFOV"
#define ES_NAV_NEARCLIP             "navNearClip"
#define ES_NAV_FARCLIP              "navFarClip"
#define ES_NAV_BKGCOLOR             "navBkgColor"
#define ES_DEFAULT_MESH             "defaultMesh"

class MainFrame;
class LogWindow;
class StatsWindow;
class AppInterface;
class GameInterface;

//! wx app class
class EditorApp: public wxApp
{
    DECLARE_CLASS( EditorApp )
    DECLARE_EVENT_TABLE()

    public:

                                            EditorApp();

        //! Load a level.
        bool                                loadLevel( const std::string& levelfile );

        //! Unload a level.
        void                                unloadLevel();

        //! Pause the game.
        void                                pauseGame();

        //! Continue continue a paused game.
        void                                continueGame();

        //! Get the log window.
        LogWindow*                          getLogWindow();

        //! Get the stats window.
        StatsWindow*                        getStatsWindow();

        //! Get game interface
        GameInterface*                      getGameInterface();

        //! Get application interface
        AppInterface*                       getAppInterface();

    protected:

        //! Initialises the application.
        virtual bool                        OnInit();

        //! Called on exit.
        virtual int                         OnExit();

        //! Game interface
        GameInterface*                      _p_gameInterface;

        //! Application interface
        AppInterface*                       _p_appInterface;

        //! Main application frame
        MainFrame*                          _p_mainFrame;

        LogWindow*                          _p_logWindow;

        StatsWindow*                        _p_statsWindow;
};


DECLARE_APP(EditorApp)

#endif // _EDITOR_H_
