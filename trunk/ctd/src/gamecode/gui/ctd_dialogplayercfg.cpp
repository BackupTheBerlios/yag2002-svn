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
 # dialog control for configuring the player
 #
 #   date of creation:  06/07/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_dialogplayercfg.h"
#include "ctd_dialogsettings.h"
#include "../sound/ctd_ambientsound.h"

using namespace std;

namespace CTD
{

// some defines
#define ADLG_PREFIX             "as_"

#define PLAYER_CFG_POSTFIX      "cfg"

DialogPlayerConfig::DialogPlayerConfig( DialogGameSettings* p_menuEntity ) :
_p_clickSound( NULL ),
_p_playerConfigDialog( NULL ),
_p_listbox( NULL ),
_p_playerName( NULL ),
_p_image( NULL ),
_p_lastListSelection( NULL ),
_p_settingsDialog( p_menuEntity )
{
    // get the player config folder
    string playercfgdir;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_CONFIG_DIR, playercfgdir );

    // get player file names
    string searchdir = Application::get()->getMediaPath() + playercfgdir + "/";
    std::vector< std::string > filelisting;
    getDirectoryListing( filelisting, searchdir, PLAYER_CFG_POSTFIX );

    // setup the preview pics for StaticImage field
    if ( filelisting.size() > 0 )
    {
        for ( size_t cnt = 0; cnt < filelisting.size(); cnt++ )
        {
            // get the preview pic and player name out of player config file
            //! Note: all player types must have unique names!
            string profile( filelisting[ cnt ] );
            Settings* p_settings = SettingsManager::get()->createProfile( profile, searchdir + filelisting[ cnt ] );
            if ( !p_settings )
            {
                log << Log::LogLevel( Log::L_ERROR ) << "DialogPlayerConfig: cannot find player settings: " << searchdir + filelisting[ cnt ] << endl;
                continue;
            }
            string playertype;
            string previewPic;
            p_settings->registerSetting( "previewPic", previewPic );
            p_settings->registerSetting( "name", playertype );

            SettingsManager::get()->loadProfile( profile );
            p_settings->getValue( "previewPic", previewPic );
            p_settings->getValue( "name", playertype );
            SettingsManager::get()->destroyProfile( profile );
            //-----------------------------------

            // store the player type name and its associated config file name
            _cfgFiles[ playertype ] = filelisting[ cnt ];

            try
            {
                // create a new imageset
                CEGUI::Texture*  p_texture = GuiManager::get()->getGuiRenderer()->createTexture( previewPic, "MenuResources" );
                CEGUI::Imageset* p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( playertype, p_texture );
             
                if ( !p_imageSet->isImageDefined( previewPic ) )
                {
                    p_imageSet->defineImage( playertype, CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f, 0.0f ) );
                }

                CEGUI::Image* p_image = &const_cast< CEGUI::Image& >( p_imageSet->getImage( playertype ) );
                
                // add new preview to map
                _players.insert( make_pair( playertype, p_image ) );
            }
            catch ( const CEGUI::Exception& e )
            {
                e; // suppress warning for unused e
                CEGUI::Image* p_null = NULL;
                // empty image identifies missing preview pic
                _players.insert( make_pair( playertype, p_null ) );
            }
        }
    }
}

DialogPlayerConfig::~DialogPlayerConfig()
{
    // free up the imagesets
    std::map< std::string, CEGUI::Image* >::iterator p_beg = _players.begin(), p_end = _players.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        CEGUI::ImagesetManager::getSingleton().destroyImageset( p_beg->first );
    }

    if ( _p_playerConfigDialog )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_playerConfigDialog );
}

bool DialogPlayerConfig::initialize( const string& layoutfile )
{    
    _p_playerConfigDialog = GuiManager::get()->loadLayout( layoutfile, NULL, ADLG_PREFIX );
    if ( !_p_playerConfigDialog )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** DialogPlayerConfig: cannot find layout: " << layoutfile << endl;
        return false;
    }

    try
    {
        // setup ok button
        CEGUI::PushButton* p_btnok = static_cast< CEGUI::PushButton* >( _p_playerConfigDialog->getChild( ADLG_PREFIX "btn_ok" ) );
        p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( DialogPlayerConfig::onClickedOk, this ) );

        // setup cancel button
        CEGUI::PushButton* p_btncancel = static_cast< CEGUI::PushButton* >( _p_playerConfigDialog->getChild( ADLG_PREFIX "btn_cancel" ) );
        p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( DialogPlayerConfig::onClickedCancel, this ) );

        // setup editbox for player name
        _p_playerName = static_cast< CEGUI::Editbox* >( _p_playerConfigDialog->getChild( ADLG_PREFIX "eb_playername" ) );

        // get list box
        _p_listbox = static_cast< CEGUI::Listbox* >( _p_playerConfigDialog->getChild( ADLG_PREFIX "lst_players" ) );
        _p_listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( DialogPlayerConfig::onListItemSelChanged, this ) );

        _p_image = static_cast< CEGUI::StaticImage* >( _p_playerConfigDialog->getChild( ADLG_PREFIX "img_player" ) );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** DialogPlayerConfig: cannot setup dialog layout." << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
    }

    return true;
}

void DialogPlayerConfig::setClickSound( EnAmbientSound* p_sound )
{
    _p_clickSound = p_sound;
}

void DialogPlayerConfig::show( bool visible )
{
    if ( visible )
    {
        setupControls();
        _p_playerConfigDialog->show();
    }
    else
    {
        _p_playerConfigDialog->hide();
    }
}

void DialogPlayerConfig::setupControls()
{
    // setup level list
    //-----------------
    _p_listbox->setSortingEnabled( true );
    // set selection background color
    CEGUI::ColourRect col( 
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f ),
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f )
                          );    
    // fill up the list
    _p_listbox->resetList();
    std::map< std::string, CEGUI::Image* >::iterator p_beg = _players.begin(), p_end = _players.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( p_beg->first.c_str() );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        p_item->setUserData( ( void* )( &p_beg->first ) );  // set texture name as item data
        _p_listbox->insertItem( p_item, NULL );
    }

    // get settings
    {
        string playername;
        Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, playername );
        _p_playerName->setText( playername.c_str() );

        string playercfg;
        Configuration::get()->getSettingValue( CTD_GS_PLAYER_CONFIG, playercfg );

        // get the player type out of file list lookup table
        std::map< std::string, std::string >::iterator p_beg = _cfgFiles.begin(), p_end = _cfgFiles.end();
        string playertype;
        for ( ; p_beg != p_end; p_beg++ )
        {
            if ( p_beg->second == playercfg )
            {
                playertype = p_beg->first;
                break;
            }
        }

        if ( _players.size() > 0 )
        {
            CEGUI::ListboxItem* p_sel = _p_listbox->findItemWithText( playertype.c_str(), NULL );
            assert( p_sel && "cannot find the player type in list!" );
                
            p_sel->setSelected( true );
            _p_lastListSelection = p_sel;
            // set preview image
            setPreviewPic( p_sel );
        }
    }
}

void DialogPlayerConfig::setPreviewPic( CEGUI::ListboxItem* p_item )
{
    string* p_texname = static_cast< string* >( p_item->getUserData() );
    CEGUI::Image*  p_image = _players[ *p_texname ];
    if ( !p_image )
    {
        _p_image->setImage( NULL );
        return;
    }
    _p_image->setImage( p_image );
}

// dialog callbacks
//-----------------
bool DialogPlayerConfig::onClickedOk( const CEGUI::EventArgs& arg )
{
    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    if ( _currentSelection == "" )
        return true;

    // write back the settings to configuration
    {
        string playername = _p_playerName->getText().c_str();
        Configuration::get()->setSettingValue( CTD_GS_PLAYER_NAME, playername );
        string playercfg = _cfgFiles[ _currentSelection ];
        Configuration::get()->setSettingValue( CTD_GS_PLAYER_CONFIG, playercfg );
    }

    _currentSelection = "";

    // store the settings changes
    Configuration::get()->store();

    // let the parent dialog know that we are done configuring the player
    _p_settingsDialog->onPlayerConfigDialogClose();

    return true;
}

bool DialogPlayerConfig::onClickedCancel( const CEGUI::EventArgs& arg )
{
    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    _currentSelection = "";

    // let the parent dialog know that we are done configuring the player
    _p_settingsDialog->onPlayerConfigDialogClose();

    return true;
}

bool DialogPlayerConfig::onListItemSelChanged( const CEGUI::EventArgs& arg )
{
    // get selection
    CEGUI::ListboxItem* p_sel = _p_listbox->getFirstSelectedItem();
    if ( !p_sel )
    {
        if ( _p_lastListSelection )
        {
            _p_lastListSelection->setSelected( true );
            _currentSelection = _p_lastListSelection->getText().c_str();
            setPreviewPic( _p_lastListSelection );
            return true;
        }
        else
            return true;
    }

    _p_lastListSelection = p_sel;
    _currentSelection = p_sel->getText().c_str();
    setPreviewPic( p_sel );

    return true;
}

void DialogPlayerConfig::update( float deltaTime )
{
}

}
