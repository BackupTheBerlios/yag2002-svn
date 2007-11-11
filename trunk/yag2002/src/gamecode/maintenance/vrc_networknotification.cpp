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
 # entity for network notifications
 #  ( used only in networked mode )
 #
 #   date of creation:  08/08/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_networknotification.h"

namespace vrc
{

//! Default destruction timeout for messagebox in seconds
#define MSGBOX_DESTRUCTION_TIMEOUT      10.0f

//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( NetworkNotificationEntityFactory )

EnNetworkNotification::EnNetworkNotification() :
_p_msgBox( NULL ),
_p_networking( NULL ),
_destructionTimeOut( MSGBOX_DESTRUCTION_TIMEOUT ),
_cnt( 0.0f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "version",             _version            );
    getAttributeManager().addAttribute( "msgOfDay",            _msgOfDay           );
}

EnNetworkNotification::~EnNetworkNotification()
{
    if ( _p_msgBox )
    {
        _p_msgBox->destroy();
        _p_msgBox = NULL;                
    }

    if ( _p_networking )
        delete _p_networking;
}

void EnNetworkNotification::handleNotification( const yaf3d::EntityNotification& notification )
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

void EnNetworkNotification::initialize()
{
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        _p_networking = new NetworkNotificationNetworking( this );
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
                text += "Server runs VRC version: ";
                text += getVersionInfo() + "\n";
                text += "Visit the project site for updates.\nhttp://yag2002.sf.net\n";
                text += "\n";
            }
            if ( getMsgOfDay().length() )
            {
                text += "Message of the Day\n";
                text += "------------------\n";
                // convert the line feeds to real line feeds
                std::string msgod( getMsgOfDay() );
                std::size_t pos = 1;
                while ( ( pos = msgod.find( "|" ) ) != std::string::npos  )
                {
                    msgod.replace( pos, 1, "\n" );
                }
                text += msgod;
                text += "\n";
            }
            if ( text.length() )
            {
                createMessagBox( "Attention", text );
            }
            // register entity in order to get notifications
            yaf3d::EntityManager::get()->registerNotification( this, true );
            // register entity in order to get updated
            yaf3d::EntityManager::get()->registerUpdate( this, true );
        }
        catch ( const CEGUI::Exception& e )
        {
            log_error << "EnNetworkNotification: problem creating gui" << std::endl;
            log_out << "      reason: " << e.getMessage().c_str() << std::endl;
        }
    }
}

void EnNetworkNotification::sendNotification( const std::string& title, const std::string& text, float destructionTimeout )
{
   if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server )
   {
       log_error << "EnNetworkNotification: the method 'sendNotification' has been called in a non-server mode! ignoring." << std::endl;
       return;
   }

   // convert the line feeds to real line feeds
   std::string textln( text );
   std::size_t pos = 1;
   while ( ( pos = textln.find( "|" ) ) != std::string::npos  )
   {
       textln.replace( pos, 1, "\n" );
   }

   // fill the notification data struct and send it over net
   tNotificationData notify;
   notify._destructionTimeout = destructionTimeout;
   strncpy( notify._text, textln.c_str(), std::min( sizeof( notify._text ), textln.length() + 1 ) );
   strncpy( notify._title, title.c_str(), std::min( sizeof( notify._title ), title.length() + 1 ) );

   _p_networking->notifyClients( notify );
}

void EnNetworkNotification::createMessagBox( const std::string& title, const std::string& text, float destructionTimeout )
{
   if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Client )
   {
       log_error << "EnNetworkNotification: the method 'createMessagBox' has been called in a non-client mode! ignoring." << std::endl;
       return;
   }
 
    if ( _p_msgBox )
        _p_msgBox->destroy();

    _p_msgBox = new yaf3d::MessageBoxDialog( title, text, yaf3d::MessageBoxDialog::OK, true );
    _p_msgBox->setClickCallback( new ClickClb( this ) );
    _p_msgBox->show();

    if ( destructionTimeout > 0.0f )
        _destructionTimeOut = destructionTimeout;
    else
        _destructionTimeOut = MSGBOX_DESTRUCTION_TIMEOUT;
}

void EnNetworkNotification::updateEntity( float deltaTime )
{
    if ( _p_msgBox )    
    {       
        _cnt += deltaTime;
        if ( _cnt > _destructionTimeOut )
        {
            _p_msgBox->destroy();
            _p_msgBox = NULL;
            _cnt      = 0.0f;
        }
    }
}

} // namespace vrc


NetworkNotificationNetworking::NetworkNotificationNetworking( vrc::EnNetworkNotification* p_entity ) :
_p_entity( p_entity )
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

NetworkNotificationNetworking::~NetworkNotificationNetworking()
{
}

void NetworkNotificationNetworking::PostObjectCreate()
{ // this method is called only on ghosts
    _p_entity = dynamic_cast< vrc::EnNetworkNotification* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_NETWORKNOTIFICATION ) );
    if ( !_p_entity )
    {
        log_error << "NetworkNotificationNetworking: cannot create entity type '" << ENTITY_NAME_NETWORKNOTIFICATION << "'" << std::endl;
        return;
    }

    _p_entity->setMsgOfDay( _p_message );
    _p_entity->setVersionInfo( _p_versionInfo );
    _p_entity->initialize();
    _p_entity->postInitialize();
}

void NetworkNotificationNetworking::setVersionInfo( const std::string& version )
{
    if ( version.length() )
        strcpy( _p_versionInfo, version.c_str() );
    else
        _p_versionInfo[ 0 ] = 0;
}

void NetworkNotificationNetworking::setMsgOfDay( const std::string& msg )
{
    if ( msg.length() < 255 )
    {
        strcpy( _p_message, msg.c_str() );
    }
    else if ( msg.length() >= 255  )
    {
        log_error << "NetworkNotificationNetworking: message of the day is too long. max characters is 255!" << std::endl;
    }
    else if ( msg.length() == 0 )
    {
        _p_message[ 0 ] = 0;
    }
}

void NetworkNotificationNetworking::RPC_RecvNotification( tNotificationData notify )
{
    // trim the strings for being on safe side
    notify._text[ sizeof( notify._text ) - 1 ] = 0;
    notify._title[ sizeof( notify._title ) - 1 ] = 0;
    log_info << "got network notification:" << std::endl;
    log_out      << " title: " << notify._title << std::endl;
    log_out      << " text:  " << notify._text << std::endl;

    _p_entity->createMessagBox( notify._title, notify._text, notify._destructionTimeout );
}

void NetworkNotificationNetworking::notifyClients( tNotificationData notify )
{
    // call the function on all replicas
    ALL_REPLICAS_FUNCTION_CALL( RPC_RecvNotification( notify ) );
}
