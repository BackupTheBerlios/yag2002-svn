/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # dialog constrol for selecting a level
 #
 #   date of creation:  04/26/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_dialoglevelselect.h"
#include "vrc_menu.h"
#include "../sound/vrc_2dsound.h"

namespace vrc
{

// some defines
#define LDLG_PREFIX             "ls_"

DialogLevelSelect::DialogLevelSelect( EnMenu* p_menuEntity ) :
_p_levelSelectDialog( NULL ),
_p_listbox( NULL ),
_p_image( NULL ),
_p_lastListSelection( NULL ),
_p_levelFiles( NULL ),
_p_menuEntity( p_menuEntity ),
_enable( false )
{
}

DialogLevelSelect::~DialogLevelSelect()
{
    // free up allocated gui elements
    try
    {
        if ( _p_levelSelectDialog )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_levelSelectDialog );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "DialogLevelSelect: problem destroying level select dialog." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    destroyResources();
}

void DialogLevelSelect::destroyResources()
{
    // free up allocated gui elements
    try
    {
        if ( _p_levelFiles )
            delete _p_levelFiles;

        _p_levelFiles = NULL;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "DialogLevelSelect: problem cleaning up entity." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

bool DialogLevelSelect::initialize( const std::string& layoutfile )
{
    _p_levelSelectDialog = yaf3d::GuiManager::get()->loadLayout( layoutfile, NULL, LDLG_PREFIX );
    if ( !_p_levelSelectDialog )
    {
        log_error << "*** DialogLevelSelect: cannot find layout: " << layoutfile << std::endl;
        return false;
    }

    _p_levelSelectDialog->hide();

    try
    {
        // setup start button
        CEGUI::PushButton* p_btnstart = static_cast< CEGUI::PushButton* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "btn_start" ) );
        p_btnstart->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLevelSelect::onClickedStart, this ) );

        // setup return button
        CEGUI::PushButton* p_btnreturn = static_cast< CEGUI::PushButton* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "btn_return" ) );
        p_btnreturn->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLevelSelect::onClickedReturn, this ) );

        // get list box
        _p_listbox = static_cast< CEGUI::Listbox* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "lst_levels" ) );
        _p_listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::DialogLevelSelect::onListItemSelChanged, this ) );

        _p_image = static_cast< CEGUI::StaticImage* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "img_pic" ) );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** DialogLevelSelect: cannot setup dialog layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    return true;
}

void DialogLevelSelect::setSearchDirectory( const std::string& dir )
{
    _searchDirectory = dir;
}

void DialogLevelSelect::changeToSearchDirectory()
{
    if ( !_searchDirectory.length() )
    {
        log_error << "*** DialogLevelSelect: search directory is unspecified." << std::endl;
        return;
    }

    if( _p_levelFiles )
        delete _p_levelFiles;

    _p_levelFiles = new gameutils::LevelFiles( _searchDirectory );
}

void DialogLevelSelect::setupControls()
{
    assert( _p_levelFiles && "level files not created" );

    // setup level list
    //-----------------
    _p_listbox->setSortingEnabled( true );
    // set selection background color
    CEGUI::ColourRect col(
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f ),
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f ),
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f ),
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f )
                          );
    // fill up the list
    _p_listbox->resetList();
    std::map< std::string, CEGUI::Image* >::iterator p_beg = _p_levelFiles->getAllFiles().begin(), p_end = _p_levelFiles->getAllFiles().end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( p_beg->first.c_str() );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        p_item->setUserData( ( void* )( &p_beg->first ) );  // set texture name as item data
        _p_listbox->insertItem( p_item, NULL );
    }

    if ( _p_levelFiles->count() > 0 )
    {
        _p_listbox->getListboxItemFromIndex( 0 )->setSelected( true );
        // set preview image
        setPreviewPic( _p_listbox->getListboxItemFromIndex( 0 ) );
        _currentSelection = _p_listbox->getListboxItemFromIndex( 0 )->getText().c_str();
        _p_lastListSelection = _p_listbox->getListboxItemFromIndex( 0 );
    }
}

void DialogLevelSelect::setPreviewPic( CEGUI::ListboxItem* p_item )
{
    assert( _p_levelFiles && "level files not created" );

    std::string* p_texname = static_cast< std::string* >( p_item->getUserData() );
    CEGUI::Image*  p_image = _p_levelFiles->getImage( *p_texname );
    if ( !p_image )
    {
        _p_image->setImage( NULL );
        return;
    }
    _p_image->setImage( p_image );
}

// dialog callbacks
//-----------------
bool DialogLevelSelect::onClickedStart( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    if ( _currentSelection == "" )
        return true;

    _p_levelSelectDialog->hide();

    // let the menu system know that we load a new level
    assert( _p_levelFiles && "level files not created" );
    _p_menuEntity->onLevelSelected( _currentSelection, _p_levelFiles->getImage( _currentSelection ) );

    _currentSelection = "";

    return true;
}

bool DialogLevelSelect::onClickedReturn( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_levelSelectDialog->hide();

    _p_menuEntity->onLevelSelectCanceled();

    _currentSelection = "";

    return true;
}

bool DialogLevelSelect::onListItemSelChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // get selection
    CEGUI::ListboxItem* p_sel = _p_listbox->getFirstSelectedItem();
    if ( !p_sel )
    {
        _p_lastListSelection->setSelected( true );
        p_sel = _p_lastListSelection;
    }
    else
    {
        _p_lastListSelection = p_sel;
    }

    setPreviewPic( p_sel );
    _currentSelection = p_sel->getText().c_str();

    return true;
}

void DialogLevelSelect::update( float /*deltaTime*/ )
{
    // nothing to do
}

void DialogLevelSelect::enable( bool en )
{
    // just be safe
    if ( en == _enable )
        return;

    if ( en )
    {
        changeToSearchDirectory();
        setupControls();
        _p_levelSelectDialog->show();
    }
    else
    {
        _p_listbox->resetList();
        _p_levelSelectDialog->hide();
        destroyResources();
    }

    _enable = en;
}

} // namespace vrc
