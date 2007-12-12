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

#include <main.h>
#include "mainframe.h"
#include "drawpanel.h"

#include "pix/story.xpm"
#include "pix/event.xpm"
#include "pix/condition.xpm"

#define APP_ICON    "pix/story.xpm"

IMPLEMENT_CLASS( beditor::MainFrame, wxFrame )

BEGIN_EVENT_TABLE( beditor::MainFrame, wxFrame )

    EVT_MENU( ID_MENUITEM6, beditor::MainFrame::onMenuitemNewClick )

    EVT_MENU( ID_MENUITEM7, beditor::MainFrame::onMenuitemOpenClick )

    EVT_MENU( ID_MENUITEM8, beditor::MainFrame::onMenuitemSaveClick )

    EVT_MENU( ID_MENUITEM9, beditor::MainFrame::onMenuitemSaveAsClick )

    EVT_MENU( ID_MENUITEM10, beditor::MainFrame::onMenuitemQuitClick )

    EVT_MENU( ID_MENUITEM11, beditor::MainFrame::onMenuitemAboutClick )

    EVT_MENU( ID_TOOL_DRAW_EVENT, beditor::MainFrame::onToolDrawEventClick )

    EVT_MENU( ID_TOOL_DRAW_CONDITION, beditor::MainFrame::onToolDrawConditionClick )

END_EVENT_TABLE()

namespace beditor
{

MainFrame::MainFrame()
{
}

MainFrame::MainFrame( wxWindow* parent )
{
    wxFrame::Create( parent, SYMBOL_MAINFRAME_IDNAME, SYMBOL_MAINFRAME_TITLE, SYMBOL_MAINFRAME_POSITION, SYMBOL_MAINFRAME_SIZE, SYMBOL_MAINFRAME_STYLE );
    createControls();
    SetIcon( GetIconResource( wxT( APP_ICON ) ) );
    Centre();
}

MainFrame::~MainFrame()
{
}

void MainFrame::createControls()
{
    MainFrame* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append(ID_MENUITEM6, _("New"), _T(""), wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM7, _("Open"), _T(""), wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM8, _("Save"), _T(""), wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM9, _("Save As ..."), _T(""), wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append(ID_MENUITEM10, _("Quit"), _T(""), wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    wxMenu* itemMenu10 = new wxMenu;
    menuBar->Append(itemMenu10, _("Edit"));
    wxMenu* itemMenu11 = new wxMenu;
    itemMenu11->Append(ID_MENUITEM11, _("About"), _T(""), wxITEM_NORMAL);
    menuBar->Append(itemMenu11, _("Help"));
    itemFrame1->SetMenuBar(menuBar);

    wxPanel* itemPanel13 = new wxPanel( itemFrame1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel13->SetSizer(itemBoxSizer14);

    wxSplitterWindow* itemSplitterWindow15 = new wxSplitterWindow( itemPanel13, ID_SPLITTERWINDOW, wxDefaultPosition, wxSize(100, 100), 0 );
    itemSplitterWindow15->SetMinimumPaneSize(0);

    wxPanel* itemPanel16 = new wxPanel( itemSplitterWindow15, ID_PANEL6, wxDefaultPosition, wxDefaultSize, 0 );
    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemPanel16->SetSizer(itemBoxSizer17);

    wxTreeCtrl* itemTreeCtrl18 = new wxTreeCtrl( itemPanel16, ID_TREECTRL, wxDefaultPosition, wxSize(150, -1), wxTR_SINGLE|wxSUNKEN_BORDER );
    itemBoxSizer17->Add(itemTreeCtrl18, 1, wxALIGN_LEFT|wxALL, 5);

    wxPanel* itemPanel19 = new wxPanel( itemPanel16, ID_PANEL7, wxDefaultPosition, wxSize(150, 200), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer17->Add(itemPanel19, 0, wxALIGN_LEFT|wxALL, 5);

    wxPanel* itemPanel20 = new wxPanel( itemSplitterWindow15, ID_PANEL8, wxDefaultPosition, wxSize(600, -1), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxVERTICAL);
    itemPanel20->SetSizer(itemBoxSizer21);

    wxToolBar* itemToolBar22 = new wxToolBar( itemPanel20, ID_TOOLBAR, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL );
    wxBitmap itemtool23Bitmap(itemFrame1->GetBitmapResource(wxT("event.xpm")));
    wxBitmap itemtool23BitmapDisabled;
    itemToolBar22->AddTool(ID_TOOL_DRAW_EVENT, _T(""), itemtool23Bitmap, itemtool23BitmapDisabled, wxITEM_NORMAL, _("Add Event"), _("Add Event Block to diagram"));
    wxBitmap itemtool24Bitmap(itemFrame1->GetBitmapResource(wxT("condition.xpm")));
    wxBitmap itemtool24BitmapDisabled;
    itemToolBar22->AddTool(ID_TOOL_DRAW_CONDITION, _T(""), itemtool24Bitmap, itemtool24BitmapDisabled, wxITEM_NORMAL, _("Add Condition"), _("Add Condition Block to diagram"));
    itemToolBar22->Realize();
    itemBoxSizer21->Add(itemToolBar22, 0, wxGROW|wxALL, 5);
    wxPanel* itemPanel25 = new DrawPanel( itemPanel20 );
    itemBoxSizer21->Add(itemPanel25, 1, wxGROW|wxALL, 5);

    itemSplitterWindow15->SplitVertically(itemPanel16, itemPanel20, 170);
    itemBoxSizer14->Add(itemSplitterWindow15, 1, wxGROW|wxALL, 5);

    wxStatusBar* itemStatusBar26 = new wxStatusBar( itemFrame1, ID_STATUSBAR, wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar26->SetFieldsCount(1);
    itemFrame1->SetStatusBar(itemStatusBar26);
}

void MainFrame::onMenuitemNewClick( wxCommandEvent& event )
{
    event.Skip();
}

void MainFrame::onMenuitemOpenClick( wxCommandEvent& event )
{
    event.Skip();
}

void MainFrame::onMenuitemSaveClick( wxCommandEvent& event )
{
    event.Skip();
}

void MainFrame::onMenuitemSaveAsClick( wxCommandEvent& event )
{
    event.Skip();
}

void MainFrame::onMenuitemQuitClick( wxCommandEvent& event )
{
    wxWindow::Destroy();
}

void MainFrame::onMenuitemAboutClick( wxCommandEvent& event )
{
    event.Skip();
}

bool MainFrame::ShowToolTips()
{
    return true;
}

wxBitmap MainFrame::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    if ( name == _T( "event.xpm" ) )
    {
        wxBitmap bitmap( folder_xpm );
        return bitmap;
    }
    else if ( name == _T( "condition.xpm" ) )
    {
        wxBitmap bitmap( condition_xpm );
        return bitmap;
    }
    return wxNullBitmap;
}

wxIcon MainFrame::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    if ( name == _T( "story.xpm" ) )
    {
        wxIcon icon( htmfoldr_xpm );
        return icon;
    }
    return wxNullIcon;
}

void MainFrame::onToolDrawEventClick( wxCommandEvent& event )
{
    event.Skip();
}

void MainFrame::onToolDrawConditionClick( wxCommandEvent& event )
{
    event.Skip();
}

} // namespace beditor
