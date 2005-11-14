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
 # common game utilities
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>


YAF3D_SINGLETON_IMPL( vrc::gameutils::PlayerUtils );
YAF3D_SINGLETON_IMPL( vrc::gameutils::GuiUtils );

namespace vrc
{
namespace gameutils
{

// Implementation of player utils
GuiUtils::GuiUtils() :
_p_mainWindow( NULL ),
_p_rootWindow( NULL )
{

}

GuiUtils::~GuiUtils()
{
    // clean up gui resources
    destroyMainWindow();
}

CEGUI::Window* GuiUtils::getMainGuiWindow()
{
    if ( _p_mainWindow )
        return _p_mainWindow;

    // create main window
    try
    {
        _p_rootWindow = yaf3d::GuiManager::get()->getRootWindow();
        _p_mainWindow = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", "_guiutils_game_main_window_" ) );
        _p_rootWindow->addChildWindow( _p_mainWindow );
    }
    catch ( const CEGUI::Exception& e )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** error setting up main gui window" << std::endl;
        yaf3d::log << "   reason: " << e.getMessage().c_str() << std::endl;
        return NULL;
    }

    return _p_mainWindow;
}

void GuiUtils::showMainWindow( bool show )
{
    if ( !_p_mainWindow )
        return;
 
    if ( show )
        _p_rootWindow->addChildWindow( _p_mainWindow );
    else
        _p_rootWindow->removeChildWindow( _p_mainWindow );
}

void GuiUtils::destroyMainWindow()
{
    if ( !_p_mainWindow )
        return;

    try
    {
        _p_rootWindow->removeChildWindow( _p_mainWindow );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_mainWindow );
    }
    catch ( const CEGUI::Exception& e )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "guiutils: problem cleaning up gui resources" << std::endl;
        yaf3d::log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
    _p_mainWindow = NULL;
    _p_rootWindow = NULL;
}

void GuiUtils::showMousePointer( bool show )
{
    if ( show )
        yaf3d::GuiManager::get()->showMousePointer( true );
    else
        yaf3d::GuiManager::get()->showMousePointer( false );
}

// Implementation of player utils
PlayerUtils::PlayerUtils() :
_p_localPlayer( NULL )
{
}

bool PlayerUtils::getPlayerConfig( unsigned int mode, bool remote, std::string& cfgfile )
{
    std::string playercfgdir;
    std::string playercfgfile;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_PLAYER_CONFIG_DIR, playercfgdir );
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_PLAYER_CONFIG, playercfgfile );
    // assemble full path of player cfg file
    std::string cfg = yaf3d::Application::get()->getMediaPath() + playercfgdir + "/" + playercfgfile;
    // load player config
    std::string profile( cfg );
    yaf3d::Settings* p_settings = yaf3d::SettingsManager::get()->createProfile( profile, cfg );
    if ( !p_settings )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "Menu: cannot find player settings: " << cfg << std::endl;
        return false;
    }
    std::string key, value;
    switch ( mode )
    {
        case yaf3d::GameState::Standalone:
            key = "standaloneConfig";
            break;

        case yaf3d::GameState::Client:
            if ( remote )
                key = "remoteClientConfig";
            else
                key = "clientConfig";
            break;

        case yaf3d::GameState::Server:
                key = "serverConfig";
            break;

        default:
            return false;
    }
    p_settings->registerSetting( key, value );
    yaf3d::SettingsManager::get()->loadProfile( profile );
    p_settings->getValue( key, cfgfile );
    yaf3d::SettingsManager::get()->destroyProfile( profile );

    return true;
}

yaf3d::BaseEntity* PlayerUtils::getLocalPlayer()
{
    return _p_localPlayer;
}

void PlayerUtils::setLocalPlayer( yaf3d::BaseEntity* p_entity )
{
    _p_localPlayer = p_entity;
}

void PlayerUtils::changeLocalPlayerName( const std::string& name )
{
    if ( !_p_localPlayer )
        return;

    // change the player name in configuration
    yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_PLAYER_NAME, name );

    // send a notification that the player name has been changed
    yaf3d::EntityNotification ennotify( PLAYER_NOTIFY_NAME_CHANGED );
    yaf3d::EntityManager::get()->sendNotification( ennotify );
}

void PlayerUtils::addRemotePlayer( yaf3d::BaseEntity* p_entity )
{
    // first check whether the entity is already in list
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _remotePlayers.begin(), p_end = _remotePlayers.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( *p_beg == p_entity )
            break;
   
    assert( ( p_beg == p_end ) && "remote player already exists in list!" );
    
    _remotePlayers.push_back( p_entity );
}

void PlayerUtils::removeRemotePlayer( yaf3d::BaseEntity* p_entity )
{
    // first check whether the entity is in list
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _remotePlayers.begin(), p_end = _remotePlayers.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( *p_beg == p_entity )
            break;

    assert( ( p_beg != p_end ) && "remote player does not exist in list!" );

    _remotePlayers.erase( p_beg );
}

// level file class
LevelFiles::LevelFiles( const std::string& dir )
{
    // get level file names
    std::string searchdir = yaf3d::Application::get()->getMediaPath() + dir;
    std::vector< std::string > files;
    yaf3d::getDirectoryListing( files, searchdir, "lvl" );

    static unsigned int s_postfix = 0;
    std::stringstream   postfix;
    postfix << s_postfix;
    s_postfix++;
    // setup the preview pics for StaticImage field
    if ( files.size() > 0 )
    {
        for ( size_t cnt = 0; cnt < files.size(); cnt++ )
        {
            std::string textureFile  = dir + files[ cnt ] + ".tga";
            std::string materialName = files[ cnt ];
            try
            {
                // create a new imageset
                CEGUI::Texture*  p_texture = yaf3d::GuiManager::get()->getGuiRenderer()->createTexture( textureFile, std::string( "_levelPics_" ) + postfix.str() );
                CEGUI::Imageset* p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( materialName + postfix.str(), p_texture );
             
                if ( !p_imageSet->isImageDefined( textureFile ) )
                {
                    p_imageSet->defineImage( materialName + postfix.str(), CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f,0.0f ) );
                }

                CEGUI::Image* p_image = &const_cast< CEGUI::Image& >( p_imageSet->getImage( materialName + postfix.str() ) );
                
                // add new preview to map
                _files.insert( make_pair( materialName, p_image ) );
            }
            catch ( const CEGUI::Exception& )
            {
                CEGUI::Image* p_null = NULL;
                // empty image identifies missing preview pic
                _files.insert( make_pair( materialName, p_null ) );
            }
        }
    }
}

LevelFiles::~LevelFiles()
{
    // free up the imagesets
    try
    {
        std::map< std::string, CEGUI::Image* >::iterator p_beg = _files.begin(), p_end = _files.end();
        for ( ; p_beg != p_end; p_beg++ )
        {
            CEGUI::ImagesetManager::getSingleton().destroyImageset( p_beg->first );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "DialogLevelSelect: problem cleaning up entity." << std::endl;
        yaf3d::log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

CEGUI::Image* LevelFiles::getImage( const std::string& file )
{
    std::map< std::string, CEGUI::Image* >::iterator found;
    found = _files.find( file );
    if ( found == _files.end() )
        return NULL;

    return found->second;
}
//----------

} // namespace gameutils
} // namespace vrc
