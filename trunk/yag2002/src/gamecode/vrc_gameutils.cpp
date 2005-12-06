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

#define GUI_SND_FILE_CLICK                  "gui/sound/click.wav"
#define GUI_SND_VOL_CLICK                   0.2f
#define GUI_SND_FILE_HOVER                  "gui/sound/hover.wav"
#define GUI_SND_VOL_HOVER                   0.2f
#define GUI_SND_FILE_SCROLLBAR              "gui/sound/scrollbar.wav"
#define GUI_SND_VOL_SCROLLBAR               0.2f
#define GUI_SND_FILE_ATTENTION              "gui/sound/attention.wav"
#define GUI_SND_VOL_ATTENTION               0.2f

namespace vrc
{
namespace gameutils
{

//! Auto-instance for registering game code config settings at startup
std::auto_ptr< VRCConfigRegistry > _autoptr_VRCConfigRegistry = std::auto_ptr< VRCConfigRegistry >( new VRCConfigRegistry );

// Implementation of configuration settings registry
VRCConfigRegistry::VRCConfigRegistry() :
_playerName( "NoName" ),
_playerConfig( "player.cfg" ),
_playerConfigDir( "player" ),
_mouseSensitivity( 1.0f ),
_mouseInverted( false ),
_moveForward( "W" ),
_moveBackward( "S" ),
_moveLeft( "A" ),
_moveRight( "D" ),
_jump( "Space" ),
_cameramode( "F1" ),
_chatmode( "RMB" )
{
    // register this instance for getting game state changes
    yaf3d::GameState::get()->registerCallbackStateChange( this );
}

VRCConfigRegistry::~VRCConfigRegistry()
{
}

void VRCConfigRegistry::onStateChange( unsigned int state )
{
    // we register the settings only once during application 'Initializing'
    if ( state != yaf3d::GameState::Initializing )
        return;

    // register settings
    yaf3d::Configuration::get()->addSetting( VRC_GS_PLAYER_NAME,         _playerName       );
    yaf3d::Configuration::get()->addSetting( VRC_GS_PLAYER_CONFIG_DIR,   _playerConfigDir  );
    yaf3d::Configuration::get()->addSetting( VRC_GS_PLAYER_CONFIG,       _playerConfig     );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_MOVE_FORWARD,    _moveForward      );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_MOVE_BACKWARD,   _moveBackward     );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_MOVE_LEFT,       _moveLeft         );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_MOVE_RIGHT,      _moveRight        );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_JUMP,            _jump             );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_CAMERAMODE,      _cameramode       );
    yaf3d::Configuration::get()->addSetting( VRC_GS_KEY_CHATMODE,        _chatmode         );
    yaf3d::Configuration::get()->addSetting( VRC_GS_MOUSESENS,           _mouseSensitivity );
    yaf3d::Configuration::get()->addSetting( VRC_GS_INVERTMOUSE,         _mouseInverted    );

    // now load the setting values from config file
    yaf3d::Configuration::get()->load();
}

// Implementation of player utils
GuiUtils::GuiUtils() :
_p_mainWindow( NULL ),
_p_rootWindow( NULL )
{
    // register this instance for getting game state changes, needed for shutdown
    yaf3d::GameState::get()->registerCallbackStateChange( this );

    // setup standard gui sounds
    createSound( GUI_SND_NAME_CLICK, GUI_SND_FILE_CLICK, GUI_SND_VOL_CLICK );
    createSound( GUI_SND_NAME_HOVER, GUI_SND_FILE_HOVER, GUI_SND_VOL_HOVER );
    createSound( GUI_SND_NAME_SCROLLBAR, GUI_SND_FILE_SCROLLBAR, GUI_SND_VOL_SCROLLBAR );
    createSound( GUI_SND_NAME_ATTENTION, GUI_SND_FILE_ATTENTION, GUI_SND_VOL_ATTENTION );
}

GuiUtils::~GuiUtils()
{
    // release sounds
    try
    {
        MapSound::iterator p_beg = _soundMap.begin(), p_end = _soundMap.end();
        for ( ; p_beg != p_end; ++p_beg )
            yaf3d::SoundManager::get()->releaseSound( p_beg->second );
    }
    catch ( const yaf3d::SoundExpection& e )
    {
        log_error << "GuiUtils: problem releasing sounds, reason: " << e.what() << std::endl;
    }
}

void GuiUtils::onStateChange( unsigned int state )
{
    // shutdown the singleton on application shutdown
    if ( state == yaf3d::GameState::Quitting )
        destroy();
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
        log_error << "*** error setting up main gui window" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
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

void GuiUtils::showMousePointer( bool show )
{
    if ( show )
        yaf3d::GuiManager::get()->showMousePointer( true );
    else
        yaf3d::GuiManager::get()->showMousePointer( false );
}

unsigned int GuiUtils::createSound( const std::string& name, const std::string& filename, float volume )
{
    // check if there is already a sound with given name
    if ( _soundMap.find( name ) != _soundMap.end() )
    {
        log_error << "GuiUtils::createSound sound source with name '" << name << "' already exists." << std::endl;
        return 0;
    }

    unsigned int soundID = 0;

    try 
    {
        soundID   = yaf3d::SoundManager::get()->createSound( filename, volume, false, yaf3d::SoundManager::fmodDefaultCreationFlags2D );
    } 
    catch ( const yaf3d::SoundExpection& e )
    {
        log_error << "GuiUtils::createSound" << "  error creating sound: " << filename << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
        return 0;
    }

    // store the sound
    _soundMap[ name ] = soundID;

    return soundID;
 }

unsigned int GuiUtils::getSoundID( const std::string& name )
{
    // try to find the sound
    MapSound::iterator soundID = _soundMap.find( name );
    if ( soundID == _soundMap.end() )
        return 0;

    return soundID->second;
}

void GuiUtils::playSound( const std::string& name )
{
    MapSound::iterator soundID = _soundMap.find( name );
    if ( soundID == _soundMap.end() )
    {
        log_error << " sound source with name '" << name << "' does not exist." << std::endl;
        return;
    }
    yaf3d::SoundManager::get()->playSound( soundID->second );
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
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_CONFIG_DIR, playercfgdir );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_CONFIG, playercfgfile );
    // assemble full path of player cfg file
    std::string cfg = yaf3d::Application::get()->getMediaPath() + playercfgdir + "/" + playercfgfile;
    // load player config
    std::string profile( cfg );
    yaf3d::Settings* p_settings = yaf3d::SettingsManager::get()->createProfile( profile, cfg );
    if ( !p_settings )
    {
        log_error << "Menu: cannot find player settings: " << cfg << std::endl;
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

void PlayerUtils::addRemotePlayer( yaf3d::BaseEntity* p_entity )
{
    // first check whether the entity is already in list
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _remotePlayers.begin(), p_end = _remotePlayers.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == p_entity )
            break;
   
    assert( ( p_beg == p_end ) && "remote player already exists in list!" );
    
    _remotePlayers.push_back( p_entity );
}

void PlayerUtils::removeRemotePlayer( yaf3d::BaseEntity* p_entity )
{
    // first check whether the entity is in list
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _remotePlayers.begin(), p_end = _remotePlayers.end();
    for ( ; p_beg != p_end; ++p_beg )
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
    ++s_postfix;
    // setup the preview pics for StaticImage field
    if ( files.size() > 0 )
    {
        for ( size_t cnt = 0; cnt < files.size(); ++cnt )
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
        for ( ; p_beg != p_end; ++p_beg )
        {
            CEGUI::ImagesetManager::getSingleton().destroyImageset( p_beg->first );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "DialogLevelSelect: problem cleaning up entity." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
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
