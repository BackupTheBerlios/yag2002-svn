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
 # entity for notifying the player if a new version is available
 #  ( used only in networked mode )
 #
 #   date of creation:  08/08/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_updatenotifier.h"

namespace vrc
{

//! Default destruction timeout for messagebox in seconds
#define MSGBOX_DESTRUCTION_TIMEOUT      15.0f

//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( UpdateNotifierEntityFactory )

EnUpdateNotifier::EnUpdateNotifier() :
_p_msgBox( NULL ),
_p_networking( NULL ),
_destructionTimeOut( MSGBOX_DESTRUCTION_TIMEOUT ),
_cnt( 0.0f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "version",             _version            );
    getAttributeManager().addAttribute( "msgOfDay",            _msgOfDay           );
}

EnUpdateNotifier::~EnUpdateNotifier()
{
    if ( _p_msgBox )
    {
        _p_msgBox->destroy();
        _p_msgBox = NULL;                
    }

    if ( _p_networking )
        delete _p_networking;
}

void EnUpdateNotifier::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _p_msgBox )
            {
                _p_msgBox->destroy();
                _p_msgBox = NULL;
            }
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            if ( _p_networking )
            {
                _p_networking->setMsgOfDay( _msgOfDay );
                _p_networking->setVersionInfo( _version );
            }
        }
        break;

        default:
            ;
    }
}

void EnUpdateNotifier::initialize()
{
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        _p_networking = new UpdateNotifierNetworking( this );
        _p_networking->Publish();
        log_info << "VRC version info: " << getVersionInfo() << std::endl;
        log_info << "VRC version info: " << getMsgOfDay() << std::endl;
    }
    else if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
    {
        try
        {
            std::string text;
            if ( getVersionInfo() != VRC_VERSION )
            {
                text += "A new VRC version is available: ";
                text += getVersionInfo() + "\n";
                text += "Visit the project site for download.\nhttp://yag2002.sf.net\n";
                text += "\n";
            }
            if ( getMsgOfDay().length() )
            {
                text += "Message of the Day\n";
                text += "------------------\n";
                // convert the line feeds to real line feeds
                std::string msgod( getMsgOfDay() );
                std::size_t pos = 1;
                while ( ( pos = msgod.find( "\\n" ) ) != std::string::npos  )
                {
                    msgod.replace( pos, 1, "\n" );
                    msgod.erase( pos + 1, 1 );
                }
                text += msgod;
                text += "\n";
            }
            if ( text.length() )
            {
                _p_msgBox = new yaf3d::MessageBoxDialog( "Attention", text, yaf3d::MessageBoxDialog::OK, true );
                _p_msgBox->setClickCallback( new ClickClb( this ) );
                _p_msgBox->show();
            }

            // register entity in order to get notifications
            yaf3d::EntityManager::get()->registerNotification( this, true );
            // register entity in order to get updated
            yaf3d::EntityManager::get()->registerUpdate( this, true );
        }
        catch ( const CEGUI::Exception& e )
        {
            log_error << "EnUpdateNotifier: problem creating gui" << std::endl;
            log << "      reason: " << e.getMessage().c_str() << std::endl;
        }
    }
}

void EnUpdateNotifier::updateEntity( float deltaTime )
{
    _cnt += deltaTime;
    if ( _cnt > _destructionTimeOut )
    {
        // this entity is not longer needed
        yaf3d::EntityManager::get()->deleteEntity( this );
    }
}

} // namespace vrc


UpdateNotifierNetworking::UpdateNotifierNetworking( vrc::EnUpdateNotifier* p_entity )
{
    memset( _p_versionInfo, 0, sizeof( _p_versionInfo ) );
    memset( _p_message, 0, sizeof( _p_message ) );

    // if p_entity is not NULL then we are running on server
    if ( p_entity )
    {
        strcpy( _p_versionInfo, p_entity->getVersionInfo().c_str() );
        strcpy( _p_message, p_entity->getMsgOfDay().c_str() );
    }
}

UpdateNotifierNetworking::~UpdateNotifierNetworking()
{
}

void UpdateNotifierNetworking::PostObjectCreate()
{
    vrc::EnUpdateNotifier* p_entity = dynamic_cast< vrc::EnUpdateNotifier* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_UPDATENOTIFIER ) );
    if ( !p_entity )
    {
        log_error << "UpdateNotifierNetworking: cannot create entity type '" << ENTITY_NAME_UPDATENOTIFIER << "'" << std::endl;
        return;
    }

    p_entity->setMsgOfDay( _p_message );
    p_entity->setVersionInfo( _p_versionInfo );
    p_entity->initialize();
    p_entity->postInitialize();
}

void UpdateNotifierNetworking::setVersionInfo( const std::string& version )
{
    if ( version.length() )
        strcpy( _p_versionInfo, version.c_str() );
    else
        _p_versionInfo[ 0 ] = 0;
}

void UpdateNotifierNetworking::setMsgOfDay( const std::string& msg )
{
    if ( msg.length() < 255 )
    {
        strcpy( _p_message, msg.c_str() );
    }
    else if ( msg.length() >= 255  )
    {
        log_error << "UpdateNotifierNetworking: message of the day is too long. max characters is 255!" << std::endl;
    }
    else if ( msg.length() == 0 )
    {
        _p_message[ 0 ] = 0;
    }
}
