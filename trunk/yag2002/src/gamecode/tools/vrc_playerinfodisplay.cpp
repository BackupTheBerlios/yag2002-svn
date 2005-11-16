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
 # entity for displaying player information
 #
 #   date of creation:  06/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_playerinfodisplay.h"
#include "../player/vrc_player.h"

namespace vrc
{

#define PLAYERINFO_WND    "_playerinfo_"

//! Implement and register the player ingo entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerInfoDisplayEntityFactory );

EnPlayerInfoDisplay::EnPlayerInfoDisplay() :
_position( osg::Vec3f( 0.001f, 0.1f, 0 ) ),
_enable( true ),
_p_playerEntity( NULL ),
_p_wnd( NULL ),
_p_outputText( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position"    , _position    );
    getAttributeManager().addAttribute( "enable"      , _enable      );
}

EnPlayerInfoDisplay::~EnPlayerInfoDisplay()
{        
    CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
}

void EnPlayerInfoDisplay::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:
            break;

        case YAF3D_NOTIFY_SHUTDOWN:

            yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            enable( _enable );
            _p_wnd->setPosition( CEGUI::Point( _position.x(), _position.y() ) );
        }
        break;

        case YAF3D_NOTIFY_PLAYER_DESTRUCTION:
        {
            // reset entity reference
            _p_playerEntity = NULL;
        }
        break;

        default:
            ;
    }
}

void EnPlayerInfoDisplay::initialize()
{
    try
    {        
        _p_wnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", PLAYERINFO_WND "mainFrame" ) );
        _p_wnd->setSize( CEGUI::Size( 0.35f, 0.15f ) );
        _p_wnd->setText( "player info" );
        _p_wnd->setPosition( CEGUI::Point( _position.x(), _position.y() ) );
        _p_wnd->setAlpha( 0.7f );
        _p_wnd->setAlwaysOnTop( true );

        _p_outputText = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", PLAYERINFO_WND "output" ) );
        _p_outputText->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        _p_outputText->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
        _p_outputText->setFont( YAF3D_GUI_CONSOLE );
        _p_wnd->addChildWindow( _p_outputText );

        yaf3d::GuiManager::get()->getRootWindow()->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnPlayerInfoDisplay: problem creating gui" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerUpdate( this, true );
    // register entity in order to get notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );

    getPlayerEntity();
}

void EnPlayerInfoDisplay::updateEntity( float deltaTime )
{
    if ( !_enable || !_p_playerEntity )
        return;

    static char text[ 1024 ];
    osg::Vec3 pos( _p_playerEntity->getPosition() );
    osg::Quat rot( _p_playerEntity->getRotation() );
    sprintf( text, 
        " name:      %s\n"
        " position:  %0.2f %0.2f %0.2f\n"
        " rotation:  %0.2f %0.2f %0.2f %0.2f\n",
        _p_playerEntity->getPlayerName().c_str(),
        pos.x(), pos.y(), pos.z(),
        rot.x(), rot.y(), rot.z(), rot.w()
        );

    CEGUI::String info( text );

    _p_outputText->setText( info );
}

void EnPlayerInfoDisplay::enable( bool en )
{
    if ( en )
    {
        getPlayerEntity(); // upate the entity reference
        yaf3d::GuiManager::get()->getRootWindow()->addChildWindow( _p_wnd );
    }
    else
    {
        yaf3d::GuiManager::get()->getRootWindow()->removeChildWindow( _p_wnd );
    }

    _enable = en;
}

bool EnPlayerInfoDisplay::getPlayerEntity()
{
    // try to get local player's entity considering that in a networked session several player entities can exist
    std::string playername;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_PLAYER_NAME, playername );

    _p_playerEntity = static_cast< EnPlayer* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLAYER, playername ) );
    if ( !_p_playerEntity )
    {
        // now try to get any player entity
        _p_playerEntity = static_cast< EnPlayer* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLAYER ) );
        if ( !_p_playerEntity )
        {
            log_warning << "EnPlayerInfoDisplay: there is no player to get info from!" << std::endl;
            CEGUI::String text( "there is no player entity" );
            _p_outputText->setText( text );
            return false;
        }
    }
    
    // register in player for getting it destruction notified
    // note: player tollerates multiple registration of same entity
    _p_playerEntity->registerNotifyDeletion( this );

    return true;
}
} // namespace vrc
