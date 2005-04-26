/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
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
 # dialog constrol for selecting a level
 #
 #   date of creation:  04/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_guimsgbox.h>
#include <ctd_guimanager.h>
#include <ctd_configuration.h>
#include <ctd_log.h>
#include <ctd_keymap.h>
#include <ctd_utils.h>
#include "ctd_dialoglevelselect.h"
#include "../sound/ctd_ambientsound.h"

using namespace std;

namespace CTD
{

// some defines
#define LDLG_PREFIX             "ls_"
// relative path to scene directory
#define SCENE_DIR               "scenes/"

DialogLevelSelect::DialogLevelSelect() :
_p_clickSound( NULL ),
_p_levelSelectDialog( NULL ),
_p_listbox( NULL ),
_p_image( NULL )
{
    // get level file names
    string searchdir = Application::get()->getMediaPath() + SCENE_DIR;
    std::vector< string > files;
    getDirectoryListing( files, searchdir, "lvl" );

    // setup the preview pics for StaticImage field
    if ( files.size() > 0 )
    {
        for ( size_t cnt = 0; cnt < files.size(); cnt++ )
        {
            std::string textureName  = string( SCENE_DIR ) + files[ cnt ] + ".tga";
            std::string materialName = files[ cnt ];
            try
            {
                // create a new imageset
                CEGUI::Texture*  p_texture = GuiManager::get()->getGuiRenderer()->createTexture( textureName, "MenuResources" );
                CEGUI::Imageset* p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( materialName, p_texture );
             
                if ( !p_imageSet->isImageDefined( textureName ) )
                {
                    p_imageSet->defineImage( materialName, CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f,0.0f ) );
                }

                CEGUI::Image* p_image = &const_cast< CEGUI::Image& >( p_imageSet->getImage( materialName ) );
                
                // add new preview to map
                _levelFiles.insert( make_pair( materialName, p_image ) );
            }
            catch ( CEGUI::Exception e )
            {
                CEGUI::Image* p_null = NULL;
                // empty image identifies missing preview pic
                _levelFiles.insert( make_pair( materialName, p_null ) );
            }
        }
    }
}

DialogLevelSelect::~DialogLevelSelect()
{
    // free up the imagesets
    std::map< std::string, CEGUI::Image* >::iterator p_beg = _levelFiles.begin(), p_end = _levelFiles.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        CEGUI::ImagesetManager::getSingleton().destroyImageset( p_beg->first + ".tga" );
    }

    if ( _p_levelSelectDialog )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_levelSelectDialog );
}

bool DialogLevelSelect::initialize( const string& layoutfile, CEGUI::Window* p_parent )
{    
    _p_parent = p_parent;
    _p_levelSelectDialog = GuiManager::get()->loadLayout( layoutfile, NULL, LDLG_PREFIX );
    if ( !_p_levelSelectDialog )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** DialogLevelSelect: cannot find layout: " << layoutfile << endl;
        return false;
    }

    _p_levelSelectDialog->hide();

    try
    {
        // setup start button
        CEGUI::PushButton* p_btnstart = static_cast< CEGUI::PushButton* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "btn_start" ) );
        p_btnstart->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( DialogLevelSelect::onClickedStart, this ) );

        // setup return button
        CEGUI::PushButton* p_btnreturn = static_cast< CEGUI::PushButton* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "btn_return" ) );
        p_btnreturn->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( DialogLevelSelect::onClickedReturn, this ) );

        // get list box
        _p_listbox = static_cast< CEGUI::Listbox* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "lst_levels" ) );
        _p_listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( DialogLevelSelect::onListItemSelChanged, this ) );

        _p_image = static_cast< CEGUI::StaticImage* >( _p_levelSelectDialog->getChild( LDLG_PREFIX "img_pic" ) );
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** DialogLevelSelect: cannot setup dialog layout." << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
    }

    return true;
}

void DialogLevelSelect::setClickSound( EnAmbientSound* p_sound )
{
    _p_clickSound = p_sound;
}

void DialogLevelSelect::setupControls()
{
    // setup level list
    //-----------------
    _p_listbox->setSortingEnabled( true );
    // set selection background color
    CEGUI::ColourRect col( 
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0            , 0.8f ),
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0            , 0.8f )
                          );    
    // fill up the list
    _p_listbox->resetList();
    std::map< std::string, CEGUI::Image* >::iterator p_beg = _levelFiles.begin(), p_end = _levelFiles.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( p_beg->first.c_str() );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        p_item->setUserData( ( void* )( &p_beg->first ) );  // set texture name as item data
        _p_listbox->insertItem( p_item, NULL );
    }

    if ( _levelFiles.size() > 0 )
    {
        _p_listbox->getListboxItemFromIndex( 0 )->setSelected( true );
        // set preview image
        setPreviewPic( _p_listbox->getListboxItemFromIndex( 0 ) );
    }
}

void DialogLevelSelect::setPreviewPic( CEGUI::ListboxItem* p_item )
{
    // NULL means deselection of all, no list elements selected anymore
    if ( !p_item )
        _p_image->setImage( NULL );

    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    string* p_texname = static_cast< string* >( p_item->getUserData() );
    CEGUI::Image*  p_image = _levelFiles[ *p_texname ];
    if ( !p_image )
    {
        _p_image->setImage( NULL );
        return;
    }
    _p_image->setImage( p_image );
}

// dialog callbacks
//-----------------
bool DialogLevelSelect::onClickedStart( const CEGUI::EventArgs& arg )
{
    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    return true;
}

bool DialogLevelSelect::onClickedReturn( const CEGUI::EventArgs& arg )
{
    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    _p_levelSelectDialog->hide();

    if ( _p_parent )
        _p_parent->enable();

    return true;
}

bool DialogLevelSelect::onListItemSelChanged( const CEGUI::EventArgs& arg )
{
    // get selection
    CEGUI::ListboxItem* p_sel = _p_listbox->getFirstSelectedItem();
    setPreviewPic( p_sel );

    //! TODO:
    return true;
}

void DialogLevelSelect::update( float deltaTime )
{
}

void DialogLevelSelect::show( bool visible )
{
    if ( visible )
    {
        setupControls();
        _p_levelSelectDialog->show();
        if ( _p_parent )
        {
            _p_parent->disable();
            _p_levelSelectDialog->enable();
        }
    }
    else
    {
        _p_listbox->resetList();
        _p_levelSelectDialog->hide();
        _p_parent->enable();
    }
}

}
