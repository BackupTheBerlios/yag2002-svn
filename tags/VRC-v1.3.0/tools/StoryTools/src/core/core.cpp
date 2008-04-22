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
 # the tool core singleton
 #
 #   date of creation:  08/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "core.h"
#include "storage.h"
#include "stories.h"
#include "settings.h"
#include "noderenderer.h"
#include "rendermanager.h"
#include "elementfactory.h"

//! Implement the renderer settings singleton
BEDITOR_SINGLETON_IMPL( beditor::Core )

namespace beditor
{

Core::Core()
{
}

Core::~Core()
{
}

void Core::initialize()
{
    beditorlog.addSink( "syslog", "editor.log", Log::L_VERBOSE );
    log_info << "system starting" << std::endl;
    log_info << "editor version " << STORYEDITOR_VERSION << std::endl;
    log_info << getFormatedDateAndTime() << std::endl;

    try
    {
        Settings* p_cfg = SettingsManager::get()->createProfile( CFG_PROFILE, TOOL_CONFIG_FILE );
        // register the settings
        p_cfg->registerSetting( CFG_LASTFILE, std::string( "" ) );
        p_cfg->registerSetting( CFG_LASTDIR, std::string( "" ) );
        p_cfg->registerSetting( CFG_FONT, std::string( "" ) );

        // try to load the profile, if not successful then create the settings file
        if ( !SettingsManager::get()->loadProfile( CFG_PROFILE ) )
        {
            SettingsManager::get()->storeProfile( CFG_PROFILE );
        }

        ElementFactory::get()->initialize();
        RenderManager::get()->initialize();
        Storage::get()->initialize();
        Stories::get()->initialize();
    }
    catch( const std::exception& e )
    {
        log_error << "Core: error occured during system initialization: " << e.what() << std::endl;
    }
}

void Core::shutdown()
{
    log_info << "shutting down the system ..." << std::endl;
    log_info << getFormatedDateAndTime() << std::endl;

    Stories::get()->shutdown();
    Storage::get()->shutdown();

    ElementFactory::get()->shutdown();
    RenderManager::get()->shutdown();

    SettingsManager::get()->storeProfile( CFG_PROFILE );
    SettingsManager::get()->shutdown();

    // destroy the singleton
    destroy();
}

} // namespace beditor
