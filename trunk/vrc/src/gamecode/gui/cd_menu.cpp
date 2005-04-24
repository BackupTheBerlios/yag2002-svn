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
 # entity game menu
 #
 #   date of creation:  04/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_guimsgbox.h>
#include <ctd_log.h>
#include "ctd_menu.h"
#include "ctd_dialogsettings.h"

using namespace std;
using namespace CTD; 

//! Implement and register the menu entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( MenuEntityFactory );


EnMenu::EnMenu()
{
    EntityManager::get()->registerUpdate( this );   // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "menuConfig"              , _menuConfig           );
    _attributeManager.addAttribute( "settingsDialogConfig"    , _settingsDialogConfig );
}

EnMenu::~EnMenu()
{
}

void EnMenu::initialize()
{
    // setup dialog for editing game settings
    _settingsDialog = auto_ptr< DialogGameSettings >( new DialogGameSettings );
    if ( !_settingsDialog->initialize( _settingsDialogConfig ) )
        return;
    
    _settingsDialog->show( true );
}

void EnMenu::updateEntity( float deltaTime )
{
}
