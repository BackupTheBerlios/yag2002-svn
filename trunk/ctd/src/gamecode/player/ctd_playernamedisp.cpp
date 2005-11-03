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
 # entity for displaying name of player in front
 #
 #   date of creation:  11/02/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playernamedisp.h"
#include "../ctd_gameutils.h"
#include "../visuals/ctd_camera.h"
#include "ctd_playerimpl.h"

namespace CTD
{

#define NAMEDISPLAY_PREFIX  "_playernamedisplay_"

//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerNameDisplayEntityFactory );
    
EnPlayerNameDisplay::EnPlayerNameDisplay() :
_position( osg::Vec3f( 0.02f, 0.9f ,0.0f ) ),
_updateTimer( 0.0f ),
_nameBox( NULL )
{
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "creating player name display entity"  << std::endl;

    getAttributeManager().addAttribute( "position", _position );
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
        log << Log::LogLevel( Log::L_ERROR ) << "EnPlayerNameDisplay: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnPlayerNameDisplay::handleNotification( const EntityNotification& notification )
{
    // handle menu notifications
    switch( notification.getId() )
    {
        case CTD_NOTIFY_MENU_ENTER:
        {
            if ( _nameBox )
                _nameBox->hide();
        }         
        break;

        case CTD_NOTIFY_MENU_LEAVE:
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
        CEGUI::Window* p_rootwnd = GuiManager::get()->getRootWindow();

        _nameBox = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", NAMEDISPLAY_PREFIX ) );
        _nameBox->setPosition( CEGUI::Point( 0.0f, 0.97f ) );
        _nameBox->setSize( CEGUI::Size( 0.10f, 0.03f ) );
        _nameBox->setFont( "CTD-8" );
        _nameBox->hide();

        p_rootwnd->addChildWindow( _nameBox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "EnPlayerNameDisplay: problem creating gui" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    // register for getting system notifications
    EntityManager::get()->registerNotification( this, true );
}

void EnPlayerNameDisplay::postInitialize()
{
    // register entity in order to get updated per simulation step.
    EntityManager::get()->registerUpdate( this, true );
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
    // do a simple in-sight check for all remote clients
    EnPlayer* p_player = dynamic_cast< EnPlayer* >( CTD::gameutils::PlayerUtils::get()->getLocalPlayer() );
    if ( !p_player )
        return;

    EnCamera*   p_playercamera    = p_player->getPlayerImplementation()->getPlayerCamera();
    const osg::Vec3f& campos      = p_playercamera->getCameraPosition() - p_playercamera->getCameraOffsetPosition();
    const osg::Quat&  camrotlocal = p_playercamera->getLocalRotation();
    const osg::Quat&  camrot      = p_playercamera->getCameraRotation();
    
    // calculate the player look direction
    osg::Vec3f lookdir( 0.0f, 1.0f, 0.0f );
    lookdir = camrotlocal * camrot * lookdir;
    
    // a line between a remote and camera
    osg::Vec3f  line;

    //! TODO: determining and outputting the name(s)
    std::string names;
    std::vector< BaseEntity* >& remoteplayers = CTD::gameutils::PlayerUtils::get()->getRemotePlayers();
    std::vector< BaseEntity* >::iterator p_beg = remoteplayers.begin(), p_end = remoteplayers.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        line = ( *p_beg )->getPosition() - campos;
        line.normalize();
        if ( ( line * lookdir ) > cos( osg::PI / 20.0f ) )
        {
            static int s_i = 0;
            std::stringstream i;
            i << s_i++;
            CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << i.str() << " player '"  << ( *p_beg )->getInstanceName() << "' is in fron" << std::endl;

            names += ( *p_beg )->getInstanceName() + " ";
        }
    }

    _nameBox->setText( names );
}

} // namespace CTD
