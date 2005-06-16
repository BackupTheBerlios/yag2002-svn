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
 # entity for displaying fps
 #
 #   date of creation:  23/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_fpsdisplay.h"

namespace CTD
{

#define STAT_WND    "_statistics_"

//! Implement and register the statistics entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( FPSDisplayEntityFactory );

EnFPSDisplay::EnFPSDisplay() :
_position( osg::Vec3f( 0.001f, 0.001f, 0 ) ),
_enable( true ),
_fpsTimer( 0 ),
_fps( 0 ),
_p_outputText( NULL ),
_p_wnd( NULL )
{
    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this, true );
    // register entity in order to get notifications
    EntityManager::get()->registerNotification( this, true );

    // register entity attributes
    _attributeManager.addAttribute( "position"    , _position    );
    _attributeManager.addAttribute( "enable"      , _enable      );
}

EnFPSDisplay::~EnFPSDisplay()
{        
    CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
}

void EnFPSDisplay::handleNotification( const EntityNotification& notify )
{
    // handle some notifications
    switch( notify.getId() )
    {
        case CTD_NOTIFY_MENU_ENTER:
            break;

        case CTD_NOTIFY_MENU_LEAVE:
            break;

        case CTD_NOTIFY_SHUTDOWN:

            EntityManager::get()->deleteEntity( this );
            break;

        case CTD_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            enable( _enable );
            _p_wnd->setPosition( CEGUI::Point( _position.x(), _position.y() ) );
        }
        break;

        default:
            ;
    }
}

void EnFPSDisplay::initialize()
{
    _p_wnd = CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", STAT_WND "mainWnd" );
    _p_wnd->setSize( CEGUI::Size( 0.1f, 0.05f ) );
    _p_wnd->setPosition( CEGUI::Point( _position.x(), _position.y() ) );
    _p_wnd->setAlpha( 0.7f );
    _p_wnd->setAlwaysOnTop( true );

    _p_outputText = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", STAT_WND "output" ) );
    _p_outputText->setSize( CEGUI::Size( 1.0f, 1.0f ) );
    _p_outputText->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
    _p_outputText->setHorizontalFormatting( CEGUI::StaticText::HorzCentred );
    _p_wnd->addChildWindow( _p_outputText );

    GuiManager::get()->getRootWindow()->addChildWindow( _p_wnd );
}

void EnFPSDisplay::updateEntity( float deltaTime )
{
    if ( !_enable )
        return;

    _fpsTimer += deltaTime;
    if ( _fpsTimer > 1.0f )
    {
        _fpsTimer -= 1.0f;
        std::stringstream text;
        text << "FPS " << _fps;
        _p_outputText->setText( text.str().c_str() );
        _fps = 0;
    }
    else
    {
        _fps++;
    }
}

void EnFPSDisplay::enable( bool en )
{
    if ( en )
    {
        GuiManager::get()->getRootWindow()->addChildWindow( _p_wnd );
    }
    else
    {
        GuiManager::get()->getRootWindow()->removeChildWindow( _p_wnd );
    }

    _enable = en;
}

} // namespace CTD
