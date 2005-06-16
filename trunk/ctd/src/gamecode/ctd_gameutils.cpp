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

namespace CTD
{
namespace gameutils
{

bool getPlayerConfig( unsigned int mode, std::string& cfgfile )
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
            key = "clientConfig";
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

} // namespace gameutils
} // namespace CTD
