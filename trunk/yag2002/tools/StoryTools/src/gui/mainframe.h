/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 # application's main frame
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <main.h>

#define SYMBOL_MAINFRAME_STYLE      wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_MAINFRAME_TITLE _    ( "BEditor" )
#define SYMBOL_MAINFRAME_IDNAME     10000
#define SYMBOL_MAINFRAME_SIZE       wxSize( 800, 600 )
#define SYMBOL_MAINFRAME_POSITION   wxDefaultPosition

namespace beditor
{

class DrawPanel;

//! Application's main frame
class MainFrame: public wxFrame
{
        DECLARE_CLASS( MainFrame )
        DECLARE_EVENT_TABLE()

    public:

        //! Constructor

        explicit                                MainFrame( wxWindow* parent );

        virtual                                 ~MainFrame();

    protected:

        //! Make wxWidgets happy with this default constructor

                                                MainFrame();

        //! Create the controls and sizers
        void                                    createControls();

        //! Setup the story tree
        void                                    setupStoryTree();

        //! wxEVT_COMMAND_MENU_SELECTED event handler
        void                                    onMenuitemNewClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler
        void                                    onMenuitemOpenClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler
        void                                    onMenuitemSaveClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler
        void                                    onMenuitemSaveAsClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler
        void                                    onMenuitemQuitClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler
        void                                    onMenuitemAboutClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_TREE_SEL_CHANGING event handler
        void                                    onTreectrlSelChanged( wxTreeEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_DRAW_EVENT
        void                                    onToolDrawEventClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_DRAW_CONDITION
        void                                    onToolDrawConditionClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_DRAW_LINK
        void                                    onToolDrawLinkClick( wxCommandEvent& event );

        //! Retrieves bitmap resources
        wxBitmap                                GetBitmapResource( const wxString& name );

        //! Retrieves icon resources
        wxIcon                                  GetIconResource( const wxString& name );

        //! Should we show tooltips?
        static bool                             ShowToolTips();

        //! Control identifiers
        enum {
            ID_MENUITEM_NEW         = 10002,
            ID_MENUITEM_OPEN        = 10003,
            ID_MENUITEM_SAVE        = 10004,
            ID_MENUITEM_SAVE_AS     = 10005,
            ID_MENUITEM_QUIT        = 10006,
            ID_MENUITEM_ABOUT       = 10007,
            ID_PANEL5               = 10008,
            ID_SPLITTERWINDOW       = 10009,
            ID_PANEL6               = 10010,
            ID_TREECTRL             = 10011,
            ID_PANEL7               = 10012,
            ID_PANEL8               = 10013,
            ID_TOOLBAR              = 10014,
            ID_TOOL_DRAW_EVENT      = 10015,
            ID_TOOL_DRAW_CONDITION  = 10016,
            ID_TOOL_DRAW_LINK       = 10017,
            ID_STATUSBAR            = 10018
        };

        //! Draw panel object
        DrawPanel*                              _p_drawPanel;

        //! Story tree
        wxTreeCtrl*                             _p_storyTreeCtrl;
};

} // namespace beditor

#endif // _MAINFRAME_H_
