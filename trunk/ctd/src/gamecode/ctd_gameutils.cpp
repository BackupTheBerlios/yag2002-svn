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

#include <ctd_main.h>
#include "ctd_gameutils.h"


CTD_SINGLETON_IMPL( CTD::gameutils::PlayerUtils );

namespace CTD
{
namespace gameutils
{

PlayerUtils::PlayerUtils() :
_p_localPlayer( NULL )
{
}

bool PlayerUtils::getPlayerConfig( unsigned int mode, bool remote, std::string& cfgfile )
{
    std::string playercfgdir;
    std::string playercfgfile;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_CONFIG_DIR, playercfgdir );
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_CONFIG, playercfgfile );
    // assemble full path of player cfg file
    std::string cfg = Application::get()->getMediaPath() + playercfgdir + "/" + playercfgfile;
    // load player config
    std::string profile( cfg );
    Settings* p_settings = SettingsManager::get()->createProfile( profile, cfg );
    if ( !p_settings )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "Menu: cannot find player settings: " << cfg << std::endl;
        return false;
    }
    std::string key, value;
    switch ( mode )
    {
        case GameState::Standalone:
            key = "standaloneConfig";
            break;

        case GameState::Client:
            if ( remote )
                key = "remoteClientConfig";
            else
                key = "clientConfig";
            break;

        case GameState::Server:
                key = "serverConfig";
            break;

        default:
            return false;
    }
    p_settings->registerSetting( key, value );
    SettingsManager::get()->loadProfile( profile );
    p_settings->getValue( key, cfgfile );
    SettingsManager::get()->destroyProfile( profile );

    return true;
}

void PlayerUtils::setLocalPlayer( BaseEntity* p_entity )
{
    _p_localPlayer = p_entity;
}

BaseEntity* PlayerUtils::getLocalPlayer()
{
    return _p_localPlayer;
}

void PlayerUtils::addRemotePlayer( BaseEntity* p_entity )
{
    // first check whether the entity is already in list
    std::vector< BaseEntity* >::iterator p_beg = _remotePlayers.begin(), p_end = _remotePlayers.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( *p_beg == p_entity )
            break;
   
    assert( ( p_beg == p_end ) && "remote player already exists in list!" );
    
    _remotePlayers.push_back( p_entity );
}

void PlayerUtils::removeRemotePlayer( BaseEntity* p_entity )
{
    // first check whether the entity is in list
    std::vector< BaseEntity* >::iterator p_beg = _remotePlayers.begin(), p_end = _remotePlayers.end();
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
    std::string searchdir = Application::get()->getMediaPath() + dir;
    std::vector< std::string > files;
    getDirectoryListing( files, searchdir, "lvl" );

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
                CEGUI::Texture*  p_texture = GuiManager::get()->getGuiRenderer()->createTexture( textureFile, std::string( "_levelPics_" ) + postfix.str() );
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
        log << Log::LogLevel( Log::L_ERROR ) << "DialogLevelSelect: problem cleaning up entity." << std::endl;
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
} // namespace CTD
