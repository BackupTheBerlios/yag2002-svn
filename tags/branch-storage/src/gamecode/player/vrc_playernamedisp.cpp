/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # entity for displaying name of player in front
 #
 #   date of creation:  11/02/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_playernamedisp.h"
#include "../visuals/vrc_camera.h"
#include "vrc_playerimpl.h"

namespace vrc
{

#define NAMEDISPLAY_PREFIX  "_playernamedisplay_"

//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerNameDisplayEntityFactory )
    
EnPlayerNameDisplay::EnPlayerNameDisplay() :
_position( osg::Vec3f( 0.02f, 0.9f ,0.0f ) ),
_detectionAngle( 20.0f ),
_viewAngle( 0.0f ),
_updateTimer( 0.0f ),
_nameBox( NULL )
{
    log_debug << "creating player name display entity"  << std::endl;

    getAttributeManager().addAttribute( "position",       _position       );
    getAttributeManager().addAttribute( "detectionAngle", _detectionAngle );
}

EnPlayerNameDisplay::~EnPlayerNameDisplay()
{
    try
    {
        if ( _nameBox )
            CEGUI::WindowManager::getSingleton().destroyWindow( _nameBox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnPlayerNameDisplay: problem cleaning up gui resources" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnPlayerNameDisplay::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle menu notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _nameBox )
                _nameBox->hide();
        }         
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _nameBox )
                _nameBox->show();
        }
        break;

        default:
            ;
    }
}

void EnPlayerNameDisplay::initialize()
{
    try
    {
        CEGUI::Window* p_rootwnd = yaf3d::GuiManager::get()->getRootWindow();

        _nameBox = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", NAMEDISPLAY_PREFIX ) );
        _nameBox->setPosition( CEGUI::Point( 0.0f, 0.97f ) );
        _nameBox->setSize( CEGUI::Size( 0.2f, 0.03f ) );
        _nameBox->setFont( YAF3D_GUI_FONT8 );
        _nameBox->setBackgroundEnabled( false );
        _nameBox->setFrameEnabled( false );
        _nameBox->hide();

        p_rootwnd->addChildWindow( _nameBox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnPlayerNameDisplay: problem creating gui" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    // register for getting system notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );

    //! setup view angle
    _viewAngle = cos( osg::DegreesToRadians( _detectionAngle ) );
}

void EnPlayerNameDisplay::postInitialize()
{
    // register entity in order to get updated per simulation step.
    yaf3d::EntityManager::get()->registerUpdate( this, true );
}

void EnPlayerNameDisplay::updateEntity( float deltaTime )
{
    if ( _updateTimer > PLAYERNAME_UPDATE_PERIOD )
    {
        _updateTimer = 0.0f;
        updateName();
    }
    else
    {
        _updateTimer += deltaTime;
    }
}

void EnPlayerNameDisplay::updateName()
{
    EnPlayer* p_player = dynamic_cast< EnPlayer* >( vrc::gameutils::PlayerUtils::get()->getLocalPlayer() );
    if ( !p_player )
        return;

    // names are only displayed in Ego camera mode
    if ( p_player->getPlayerImplementation()->getCameraMode() != BasePlayerImplementation::Ego )
    {
        _nameBox->setText( "" );
        return;
    }

    EnCamera*   p_playercamera    = p_player->getPlayerImplementation()->getPlayerCamera();
    const osg::Vec3f& campos      = p_playercamera->getCameraPosition() + p_playercamera->getCameraOffsetPosition();
    const osg::Quat&  camrotlocal = p_playercamera->getLocalRotation();
    const osg::Quat&  camrot      = p_playercamera->getCameraRotation();
    
    // calculate the player look direction
    osg::Vec3f lookdir( 0.0f, 1.0f, 0.0f );
    lookdir = camrotlocal * camrot * lookdir;
    
    // a line between a remote client and camera
    osg::Vec3f  line;
    osg::Vec3f  maxdist( 1000000.0f, 0.0f, 0.0f );

    //! find nearest player in front of local player
    yaf3d::BaseEntity* p_playerinfront = NULL;
    const std::vector< yaf3d::BaseEntity* >& remoteplayers = vrc::gameutils::PlayerUtils::get()->getRemotePlayers();
    std::vector< yaf3d::BaseEntity* >::const_iterator p_beg = remoteplayers.begin(), p_end = remoteplayers.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        line = ( *p_beg )->getPosition() - campos;
        osg::Vec3f  dir( line );
        dir.normalize();

        // check if the player is in our view
        if ( ( dir * lookdir ) > _viewAngle )
        {
            // store the nearest distance ( sorting )
            if ( maxdist.length2() > line.length2() )
            {
                maxdist = line;
                p_playerinfront = *p_beg;
            }
        }
    }

    if ( p_playerinfront )
    {
        EnPlayer* p_playerentity = dynamic_cast< EnPlayer* >( p_playerinfront );
        assert( p_playerentity && "wrong object type: EnPlayer expected!" );
        std::string playername = p_playerentity->getPlayerName();
        _nameBox->setText( playername );
    }
    else
    {
        _nameBox->setText( "" );
    }
}

} // namespace vrc
