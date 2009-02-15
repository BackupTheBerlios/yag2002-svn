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


#include <vrc_main.h>
#include "guibase.h"
#include "mainframe.h"
#include "gameinterface.h"
#include "panelentities.h"
#include "panelnavigation.h"
#include "navigation.h"
#include "editorutils.h"
#include "statswindow.h"
#include "logwindow.h"
#include "fileio.h"
#include "editor.h"


IMPLEMENT_CLASS( MainFrame, wxFrame )


BEGIN_EVENT_TABLE( MainFrame, wxFrame )

    EVT_CLOSE( MainFrame::onCloseWindow )

    EVT_MENU( ID_MENUITEM_FILE_NEW, MainFrame::onMenuitemFileNewClick )

    EVT_MENU( ID_MENUITEM_FILE_OPEN, MainFrame::onMenuitemFileOpenClick )

    EVT_MENU( ID_MENUITEM_FILE_SAVE, MainFrame::onMenuitemFileSaveClick )

    EVT_MENU( ID_MENUITEM_FILE_SAVE_AS, MainFrame::onMenuitemFileSaveAsClick )

    EVT_MENU( ID_MENUITEM_FILE_CLOSE, MainFrame::onMenuitemFileCloseClick )

    EVT_MENU( ID_MENUITEM_FILE_QUIT, MainFrame::onMenuitemFileQuitClick )

    EVT_MENU( ID_MENUITEM_VIEW_STATS, MainFrame::onMenuitemViewStatsClick )

    EVT_MENU( ID_MENUITEM_VIEW_LOG, MainFrame::onMenuitemViewLogClick )

    EVT_MENU( ID_MENUITEM_HELP_ABOUT, MainFrame::onMenuitemHelpAboutClick )

    EVT_MENU( ID_TOOL_SAVE, MainFrame::onMenuitemFileSaveClick )

    EVT_MENU( ID_TOOL_OPEN, MainFrame::onMenuitemFileOpenClick )

    EVT_MENU( ID_TOOL_SEL, MainFrame::onToolSelectClickClick )

    EVT_MENU( ID_TOOL_MOVE, MainFrame::onToolMoveClickClick )

    EVT_MENU( ID_TOOL_AUTOPLACE, MainFrame::onToolAutoPlaceClickClick )

    EVT_MENU( ID_TOOL_ROTATE, MainFrame::onToolRotateClickClick )

    EVT_NOTEBOOK_PAGE_CHANGED( ID_NOTEBOOK, MainFrame::onNotebookPageChanged )

END_EVENT_TABLE()


MainFrame::MainFrame( EditorApp* p_app ) :
 _p_editorApp( p_app ),
 _p_menuFile( NULL ),
 _p_menuView( NULL ),
 _p_notebook( NULL ),
 _p_panelEntities( NULL ),
 _p_panelNavigation( NULL )
{
    assert( _p_editorApp && "invalid editor app pointer" );

    wxFrame::Create( NULL, SYMBOL_MAINFRAME_IDNAME, SYMBOL_MAINFRAME_TITLE, SYMBOL_MAINFRAME_POSITION, SYMBOL_MAINFRAME_SIZE, SYMBOL_MAINFRAME_STYLE );

    SetIcon( BitmapResource::get()->getIcon( EDITOR_RESID_ICON_MAINFRAME ) );

    createControls();

    // notbook is enabled on level loading
    _p_notebook->Disable();
    setStatus( "Initializing game platform ..." );

    Move( 0, 0 );

    SetMinSize( wxSize( SYMBOL_MAINFRAME_SIZE.x, 400 ) );
}

MainFrame::~MainFrame()
{
}

void MainFrame::setStatus( const std::string& status )
{
    if ( GetStatusBar() )
        GetStatusBar()->SetLabel( status );
}

void MainFrame::selectEntity ( yaf3d::BaseEntity* p_entity )
{
    _p_panelEntities->selectEntity( p_entity );
}

void MainFrame::notify( unsigned int id )
{
    switch ( id )
    {
        case MainFrame::NOTIFY_GAME_STARTED:
        {
            _p_notebook->Enable( false );
            setStatus( "" );
        }
        break;

        case MainFrame::NOTIFY_LEVEL_LOADED:
        {
            _p_panelEntities->setupControls( true, _levelFileName );
            _p_panelNavigation->loadSettings();
            _p_notebook->Enable( true );
            setStatus( "" );

            if ( GetToolBar() )
                GetToolBar()->EnableTool( ID_TOOL_SAVE, true );

            assert( _p_menuFile );
            assert( _p_menuView );
            _p_menuFile->Enable( ID_MENUITEM_FILE_SAVE, true );
            _p_menuFile->Enable( ID_MENUITEM_FILE_SAVE_AS, true );
            _p_menuFile->Enable( ID_MENUITEM_FILE_CLOSE, true );

            // set proper mode
            GameNavigator::get()->setMode( GameNavigator::EntityPick );
        }
        break;

        case MainFrame::NOTIFY_LEVEL_UNLOADED:
        {
            _p_panelEntities->setupControls( false, "" );
            _p_panelNavigation->loadSettings();
            _p_notebook->Enable( false );
            setStatus( "" );

            if ( GetToolBar() )
                GetToolBar()->EnableTool( ID_TOOL_SAVE, false );

            assert( _p_menuFile );
            assert( _p_menuView );
            _p_menuFile->Enable( ID_MENUITEM_FILE_SAVE, false );
            _p_menuFile->Enable( ID_MENUITEM_FILE_SAVE_AS, false );
            _p_menuFile->Enable( ID_MENUITEM_FILE_CLOSE, false );
        }
        break;

        case MainFrame::NOTIFY_ARROW_CLICK:
        {
            // this is called when in arrow mode mouse button was clicked
            if ( GetToolBar() )
                GetToolBar()->ToggleTool( ID_TOOL_SEL, true );

            // restore picking mode
            GameNavigator::get()->setMode( GameNavigator::EntityPick );
        }
        break;

        default:
            assert( NULL && "invalid main frame notification!" );
    }

}

void MainFrame::updateStatsWindow( unsigned int fps, const osg::Vec3f& pos, const osg::Vec2f& rot )
{
    StatsWindow* p_stats = _p_editorApp->getStatsWindow();
    if ( !p_stats )
        return;

    p_stats->setFPS( fps );
    p_stats->setCameraSpeed( GameNavigator::get()->getSpeed() );
    p_stats->setPosition( pos );
    p_stats->setPitchYaw( rot._v[ 0 ], rot._v[ 1 ] );
}

void MainFrame::updateLogWindow()
{
    LogWindow* p_log = _p_editorApp->getLogWindow();
    if ( !p_log )
        return;

    p_log->update();
}

void MainFrame::createControls()
{
    wxMenuBar* menuBar = new wxMenuBar;
    _p_menuFile  = new wxMenu;
    _p_menuFile->Append(ID_MENUITEM_FILE_NEW, _("New ..."), _T(""), wxITEM_NORMAL);
    _p_menuFile->Append(ID_MENUITEM_FILE_OPEN, _("Open"), _T(""), wxITEM_NORMAL);
    _p_menuFile->Append(ID_MENUITEM_FILE_SAVE, _("Save"), _T(""), wxITEM_NORMAL);
    _p_menuFile->Enable(ID_MENUITEM_FILE_SAVE, false);
    _p_menuFile->Append(ID_MENUITEM_FILE_SAVE_AS, _("Save as ..."), _T(""), wxITEM_NORMAL);
    _p_menuFile->Enable(ID_MENUITEM_FILE_SAVE_AS, false);
    _p_menuFile->AppendSeparator();
    _p_menuFile->Append(ID_MENUITEM_FILE_CLOSE, _("Close"), _T(""), wxITEM_NORMAL);
    _p_menuFile->Enable(ID_MENUITEM_FILE_CLOSE, false);
    _p_menuFile->AppendSeparator();
    _p_menuFile->Append(ID_MENUITEM_FILE_QUIT, _("Quit"), _T(""), wxITEM_NORMAL);
    menuBar->Append(_p_menuFile, _("File"));
    _p_menuView = new wxMenu;
    _p_menuView->Append(ID_MENUITEM_VIEW_STATS, _("Statistics"), _T(""), wxITEM_CHECK);
    _p_menuView->Check(ID_MENUITEM_VIEW_STATS, true);
    _p_menuView->Append(ID_MENUITEM_VIEW_LOG, _("Log Console"), _T(""), wxITEM_CHECK);
    _p_menuView->Check(ID_MENUITEM_VIEW_LOG, true);
    menuBar->Append(_p_menuView, _("View"));
    wxMenu* p_menuabout = new wxMenu;
    p_menuabout->Append(ID_MENUITEM_HELP_ABOUT, _("About"), _T(""), wxITEM_NORMAL);
    menuBar->Append(p_menuabout, _("Help"));
    SetMenuBar(menuBar);

    wxToolBar* p_toolbar = CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_TOOLBAR );
    wxBitmap itemtoolBitmapDisabled;
    p_toolbar->AddTool(ID_TOOL_OPEN, _T(""), BitmapResource::get()->getBitmap( EDITOR_RESID_BMP_FILEOPEN ), itemtoolBitmapDisabled, wxITEM_NORMAL, _("Open a level file"), wxEmptyString);
    p_toolbar->AddTool(ID_TOOL_SAVE, _T(""), BitmapResource::get()->getBitmap( EDITOR_RESID_BMP_FILESAVE ), itemtoolBitmapDisabled, wxITEM_NORMAL, _("Save level file"), wxEmptyString);
    p_toolbar->AddSeparator();
    p_toolbar->AddTool(ID_TOOL_SEL, _T(""), BitmapResource::get()->getBitmap( EDITOR_RESID_BMP_ENT_SEL ), itemtoolBitmapDisabled, wxITEM_RADIO, _("Select Entity"), wxEmptyString);
    p_toolbar->AddTool(ID_TOOL_MOVE, _T(""), BitmapResource::get()->getBitmap( EDITOR_RESID_BMP_ENT_MOVE ), itemtoolBitmapDisabled, wxITEM_RADIO, _("Move Entity"), wxEmptyString);
    p_toolbar->AddTool(ID_TOOL_AUTOPLACE, _T(""), BitmapResource::get()->getBitmap( EDITOR_RESID_BMP_ENT_AUTOPLACE ), itemtoolBitmapDisabled, wxITEM_RADIO, _("Auto-place Entity"), wxEmptyString);
    p_toolbar->AddTool(ID_TOOL_ROTATE, _T(""), BitmapResource::get()->getBitmap( EDITOR_RESID_BMP_ENT_ROTATE ), itemtoolBitmapDisabled, wxITEM_RADIO, _("Rotate Entity"), wxEmptyString);
    p_toolbar->Realize();
    SetToolBar(p_toolbar);

    // diable some tools
    p_toolbar->EnableTool( ID_TOOL_SAVE, false );

    wxStatusBar* itemStatusBar20 = new wxStatusBar( this, ID_STATUSBAR, wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar20->SetFieldsCount(1);
    SetStatusBar(itemStatusBar20);

    _p_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    _p_panelEntities = new PanelEntities( _p_notebook, _p_editorApp->getGameInterface() );
    _p_notebook->AddPage(_p_panelEntities, _("Entities"));

    _p_panelNavigation = new PanelNavigation( _p_notebook );
    _p_notebook->AddPage(_p_panelNavigation, _("Navigation"));
}

void MainFrame::onCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( wxMessageBox( "You really want to quit the editor?", "Attention", wxYES_NO | wxICON_QUESTION ) == wxNO )
            return;

        // is any level loaded?
        if ( _levelFileName.length() )
        {
            if ( wxMessageBox( "Save before closing?", "Attention", wxYES_NO | wxICON_QUESTION  ) == wxYES )
            {
                wxCommandEvent ev;
                onMenuitemFileSaveClick( ev );
            }
        }
    }

    if ( _p_editorApp->getLogWindow() )
        _p_editorApp->getLogWindow()->Destroy();

    if ( _p_editorApp->getStatsWindow() )
        _p_editorApp->getStatsWindow()->Destroy();

    event.Skip();
}

void MainFrame::onMenuitemFileNewClick( wxCommandEvent& event )
{
    std::string dir( yaf3d::Application::get()->getMediaPath() );
    std::string levelfile;
    bool retry = false;

    do
    {
        wxFileDialog dlg( this, "Choose a file name", dir.c_str(), "NewLevel.lvl", "*.lvl" );
        if ( dlg.ShowModal() == wxID_CANCEL )
            return;

        std::string path = yaf3d::cleanPath( dlg.GetDirectory().c_str() );
        path += "/";
        std::string::size_type pos = 0;
        if ( ( path.length() > 2 ) && path[ 1 ] == ':' )
            pos = 2;

        if ( path.find( yaf3d::Application::get()->getMediaPath().substr( pos ), pos ) == std::string::npos )
        {
            wxMessageBox( std::string( "Please choose from following folder:\n\n" ) + yaf3d::Application::get()->getMediaPath(), "Attention", wxOK | wxICON_QUESTION );
            retry = true;
        }
        else
        {
            if ( wxFileExists( dlg.GetPath() ) )
            {
                if ( wxMessageBox( "This file already exists. Overwrite it?", "Attention", wxYES_NO | wxICON_QUESTION  ) == wxNO )
                {
                    retry = true;
                    continue;
                }
            }

            retry          = false;
            levelfile      += dlg.GetPath();
            _levelFileName = dlg.GetFilename();
            _levelFulPath  = dlg.GetPath();
        }
    }
    while ( retry );

    levelfile = yaf3d::cleanPath( levelfile );
    std::string levelname = levelfile.substr( yaf3d::Application::get()->getMediaPath().length() );
    if ( levelname.substr( levelname.length() - 4 ) == ".lvl" )
        levelname = levelname.substr( 0, levelname.length() - 4 );

    // write out the level file
    {
        ScopedGameUpdateLock lock;

        // get the entities
        std::vector< yaf3d::BaseEntity* > levelentities;

        // write a blank level file
        FileOutputLevel out;
        out.write( levelentities, levelfile, levelname, false );
    }

    // now open the new file
    levelfile = yaf3d::cleanPath( levelfile );
    _p_editorApp->loadLevel( levelfile.substr( yaf3d::Application::get()->getMediaPath().length() ) );
}

void MainFrame::onMenuitemFileOpenClick( wxCommandEvent& event )
{
    // is any level loaded?
    if ( _levelFileName.length() )
    {
        wxMessageBox( "First close the current level, please.", "Attention" );
        return;
    }

    std::string dir( yaf3d::Application::get()->getMediaPath() );
    std::string levelfile;
    bool retry = false;

    do
    {
        wxFileDialog dlg( this, "Choose a level file", dir.c_str(), "", "*.lvl" );
        if ( dlg.ShowModal() == wxID_CANCEL )
            return;

        std::string path = yaf3d::cleanPath( dlg.GetDirectory().c_str() );
        path += "/";
        std::string::size_type pos = 0;
        if ( ( path.length() > 2 ) && path[ 1 ] == ':' )
            pos = 2;

        if ( path.find( yaf3d::Application::get()->getMediaPath().substr( pos ), pos ) == std::string::npos )
        {
            wxMessageBox( std::string( "Please choose a file from following folder:\n\n" ) + yaf3d::Application::get()->getMediaPath(), "Attention", wxOK | wxICON_QUESTION );
            retry = true;
        }
        else
        {
            retry          = false;
            levelfile      += dlg.GetPath();
            _levelFileName = dlg.GetFilename();
            _levelFulPath  = dlg.GetPath();
        }
    }
    while ( retry );

    levelfile = yaf3d::cleanPath( levelfile );
    _p_editorApp->loadLevel( levelfile.substr( yaf3d::Application::get()->getMediaPath().length() ) );
}

void MainFrame::onMenuitemFileSaveClick( wxCommandEvent& event )
{
    // just to be on safe side
    if ( !_levelFulPath.length() )
        return;

    std::string levelfile = yaf3d::cleanPath( _levelFulPath );
    std::string levelname = levelfile.substr( yaf3d::Application::get()->getMediaPath().length() );
    if ( levelname.substr( levelname.length() - 4 ) == ".lvl" )
        levelname = levelname.substr( 0, levelname.length() - 4 );

    // write out the level file
    {
        ScopedGameUpdateLock lock;

        // get the entities
        std::vector< yaf3d::BaseEntity* > levelentities;
        yaf3d::EntityManager::get()->getAllEntities( levelentities );

        // write them out
        FileOutputLevel out;
        out.write( levelentities, levelfile, levelname, false );
    }
}

void MainFrame::onMenuitemFileSaveAsClick( wxCommandEvent& event )
{
    std::string dir( yaf3d::Application::get()->getMediaPath() );
    std::string levelfile;
    bool retry = false;

    do
    {
        wxFileDialog dlg( this, "Choose a file name", dir.c_str(), "NewLevel.lvl", "*.lvl" );
        if ( dlg.ShowModal() == wxID_CANCEL )
            return;

        std::string path = yaf3d::cleanPath( dlg.GetDirectory().c_str() );
        path += "/";
        std::string::size_type pos = 0;
        if ( ( path.length() > 2 ) && path[ 1 ] == ':' )
            pos = 2;

        if ( path.find( yaf3d::Application::get()->getMediaPath().substr( pos ), pos ) == std::string::npos )
        {
            wxMessageBox( std::string( "Please choose from following folder:\n\n" ) + yaf3d::Application::get()->getMediaPath(), "Attention", wxOK | wxICON_QUESTION );
            retry = true;
        }
        else
        {
            if ( wxFileExists( dlg.GetPath() ) )
            {
                if ( wxMessageBox( "This file already exists. Overwrite it?", "Attention", wxYES_NO | wxICON_QUESTION  ) == wxNO )
                {
                    retry = true;
                    continue;
                }
            }

            retry          = false;
            levelfile      += dlg.GetPath();
            _levelFileName = dlg.GetFilename();
            _levelFulPath  = dlg.GetPath();
        }
    }
    while ( retry );

    levelfile = yaf3d::cleanPath( levelfile );
    std::string levelname = levelfile.substr( yaf3d::Application::get()->getMediaPath().length() );
    if ( levelname.substr( levelname.length() - 4 ) == ".lvl" )
        levelname = levelname.substr( 0, levelname.length() - 4 );

    // write out the level file
    {
        ScopedGameUpdateLock lock;

        // get the entities
        std::vector< yaf3d::BaseEntity* > levelentities;
        yaf3d::EntityManager::get()->getAllEntities( levelentities );

        // write them out
        FileOutputLevel out;
        out.write( levelentities, levelfile, levelname, false );
    }
}

void MainFrame::onMenuitemFileCloseClick( wxCommandEvent& event )
{
    // is any level loaded?
    if ( !_levelFileName.length() )
        return;

    if ( wxMessageBox( "Save before closing?", "Attention", wxYES_NO | wxICON_QUESTION  ) == wxYES )
    {
        wxCommandEvent ev;
        onMenuitemFileSaveClick( ev );
    }

    GameNavigator::get()->selectEntity( NULL );
    GameNavigator::get()->setMode( GameNavigator::EntityPick );

    // unload the level
    _p_editorApp->unloadLevel();
    _p_notebook->Disable();
    _levelFileName = "";
    _levelFulPath  = "";
}

void MainFrame::onMenuitemFileQuitClick( wxCommandEvent& event )
{
    if ( wxMessageBox( "You really want to quit the editor?", "Attention", wxYES_NO | wxICON_QUESTION ) == wxNO )
        return;

    // is any level loaded?
    if ( _levelFileName.length() )
    {
        // is any level loaded?
        if ( _levelFileName.length() )
        {
            if ( wxMessageBox( "Save before closing?", "Attention", wxYES_NO | wxICON_QUESTION  ) == wxYES )
            {
                wxCommandEvent ev;
                onMenuitemFileSaveClick( ev );
            }
        }
    }

    wxCloseEvent ev;
    ev.SetCanVeto( false );
    onCloseWindow( ev );

    Destroy();
}

void MainFrame::onMenuitemViewStatsClick( wxCommandEvent& event )
{
    StatsWindow* p_stats = _p_editorApp->getStatsWindow();
    if ( !p_stats )
        return;

    p_stats->Show( _p_menuView->IsChecked( ID_MENUITEM_VIEW_STATS ) );
}

void MainFrame::onMenuitemViewLogClick( wxCommandEvent& event )
{
    LogWindow* p_stats = _p_editorApp->getLogWindow();
    if ( !p_stats )
        return;

    p_stats->Show( _p_menuView->IsChecked( ID_MENUITEM_VIEW_LOG ) );
}

void MainFrame::onMenuitemHelpAboutClick( wxCommandEvent& event )
{
    std::string text;
    text += "Yaf3D Content Editor " CONTENT_EDITOR_VERSION "\n";
    text += "\n";
    text += "Yaf3D Framework Version: " YAF3D_VERSION "\n";
    text += "Virtual Reality Chat Version: " VRC_VERSION "\n";
    text += "\n";
    text += "For more details about this work visit following sites.\n";
    text += "\n";
    text += "  * Yaf3D project site http://yag2002.sourceforge.net\n";
    text += "\n";
    text += "  * Virtual Reality Chat site http://www.vr-fun.net\n";

    wxMessageBox( text, "About", wxOK | wxICON_INFORMATION );
}

void MainFrame::onToolSelectClickClick( wxCommandEvent& event )
{
    GameNavigator::get()->setMode( GameNavigator::EntityPick );
}

void MainFrame::onToolMoveClickClick( wxCommandEvent& event )
{
    //! TODO ...
    GameNavigator::get()->selectEntity( NULL );
    GameNavigator::get()->setMode( GameNavigator::ShowPickArrow );
}

void MainFrame::onToolAutoPlaceClickClick( wxCommandEvent& event )
{
    GameNavigator::get()->setMode( GameNavigator::EntityPlace );
}

void MainFrame::onToolRotateClickClick( wxCommandEvent& event )
{
    //! TODO
}

void MainFrame::onNotebookPageChanged( wxNotebookEvent& event )
{
    event.Skip();
}
