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
 # main editor frame
 #
 #   date of creation:  01/18/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_


#include "guibase.h"


#define SYMBOL_MAINFRAME_STYLE      wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|/*wxMAXIMIZE_BOX|*/wxCLOSE_BOX
#define SYMBOL_MAINFRAME_TITLE      _( "VRC Level Editor " CONTENT_EDITOR_VERSION )
#define SYMBOL_MAINFRAME_IDNAME     ID_MAINFRAME
#define SYMBOL_MAINFRAME_SIZE       wxSize( 320, 800 )
#define SYMBOL_MAINFRAME_POSITION   wxDefaultPosition


class EditorApp;
class PanelPhysics;
class PanelEntities;
class PanelNavigation;

//! Main frame class
class MainFrame: public wxFrame
{
    DECLARE_CLASS( MainFrame )
    DECLARE_EVENT_TABLE()

    public:

        //! Main frame notifications
        enum Notify
        {
            NOTIFY_GAME_STARTED   = 0x01,
            NOTIFY_LEVEL_LOADED   = 0x02,
            NOTIFY_LEVEL_UNLOADED = 0x04,
            NOTIFY_INSPECT_CLICK  = 0x08
        };

        explicit                                    MainFrame( EditorApp* p_app );

                                                    ~MainFrame();

        //! Set given string in status bar
        void                                        setStatus( const std::string& status );

        //! Select given entity in tree.
        void                                        selectEntity ( yaf3d::BaseEntity* p_entity );

        //! Notify the main frame about relevant game events, one of Notify enums.
        void                                        notify( unsigned int id );

        //! Update statistics for camera
        void                                        updateStatsWindowCamera( unsigned int fps, const osg::Vec3f& pos, const osg::Vec2f& pitchyaw );

        //! Enable/disable inspector information in stats window.
        void                                        enableStatsWindowInspector( bool en );

        //! Update statistics for inspector
        void                                        updateStatsWindowInspector( const osg::Vec3f& pos, const osg::Vec3f& normal, const std::string& object );

        //! Update log window content
        void                                        updateLogWindow();

    protected:

        //! Create the controls and sizers
        void                                        createControls();

        //! Called when close icon clicked
        void                                        onCloseWindow( wxCloseEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FILE_NEW
        void                                        onMenuitemFileNewClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FILE_OPEN
        void                                        onMenuitemFileOpenClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FILE_SAVE
        void                                        onMenuitemFileSaveClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FILE_SAVE_AS
        void                                        onMenuitemFileSaveAsClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FILE_CLOSE
        void                                        onMenuitemFileCloseClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FILE_QUIT
        void                                        onMenuitemFileQuitClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VIEW_STATS
        void                                        onMenuitemViewStatsClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VIEW_LOG
        void                                        onMenuitemViewLogClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_ABOUT
        void                                        onMenuitemHelpAboutClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_SEL
        void                                        onToolSelectClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_MOVE
        void                                        onToolMoveClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_ROTATE
        void                                        onToolRotateClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_AUTOPLACE
        void                                        onToolAutoPlaceClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_INSPECT
        void                                        onToolInspectClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK1
        void                                        onNotebookPageChanged( wxNotebookEvent& event );

        EditorApp*                                  _p_editorApp;

        wxMenu*                                     _p_menuFile;

        wxMenu*                                     _p_menuView;

        wxNotebook*                                 _p_notebook;

        PanelEntities*                              _p_panelEntities;

        PanelNavigation*                            _p_panelNavigation;

        std::string                                 _levelFileName;

        std::string                                 _levelFulPath;

        // Control identifiers
        enum
        {
            ID_MAINFRAME             = 10000,
            ID_MENUITEM_FILE_NEW     = 10001,
            ID_MENUITEM_FILE_OPEN    = 10002,
            ID_MENUITEM_FILE_SAVE    = 10003,
            ID_MENUITEM_FILE_SAVE_AS = 10004,
            ID_MENUITEM_FILE_CLOSE   = 10005,
            ID_MENUITEM_FILE_QUIT    = 10006,
            ID_MENUITEM_VIEW_STATS   = 10007,
            ID_MENUITEM_VIEW_LOG     = 10008,
            ID_MENUITEM_HELP_ABOUT   = 10010,
            ID_TOOLBAR               = 10011,
            ID_TOOL_SAVE             = 10012,
            ID_TOOL_OPEN             = 10013,
            ID_TOOL_SEL              = 10014,
            ID_TOOL_MOVE             = 10015,
            ID_TOOL_ROTATE           = 10016,
            ID_TOOL_AUTOPLACE        = 10017,
            ID_TOOL_INSPECT          = 10018,
            ID_STATUSBAR             = 10020,
            ID_NOTEBOOK              = 10030
        };
};

#endif
    // _EDITORFRAME_H_
