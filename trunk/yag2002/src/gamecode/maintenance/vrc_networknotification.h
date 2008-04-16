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

#ifndef _VRC_NETWORKNOTIFICATION_H_
#define _VRC_NETWORKNOTIFICATION_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_NetworkNotification.h"

//! Networking class
class NetworkNotificationNetworking;

namespace vrc
{

#define ENTITY_NAME_NETWORKNOTIFICATION    "NetworkNotification"

//! Entity NetworkNotifier
class EnNetworkNotification :  public yaf3d::BaseEntity
{
    public:
                                                    EnNetworkNotification();

        virtual                                     ~EnNetworkNotification();

        //! Initialize entity
        void                                        initialize();

        //! This entity needs no transformation
        inline const bool                           isTransformable() const;

        //! Return the version string
        inline const std::string&                   getVersionInfo();

        //! Set the version string
        inline void                                 setVersionInfo( const std::string& version );

        //! Get message of the day
        inline const std::string&                   getMsgOfDay();

        //! Set the message of the day string
        inline void                                 setMsgOfDay( const std::string& msg );

        //! Send a notification to clients. This method can be used only on server!
        void                                        sendNotification( const std::string& title, const std::string& text, float destructionTimeout = 0.0f );

        //! Create a notification message box, the box is destroyed after given timeout. if zero timeout given then the default timeout is used.
        //! Only used in clients, on server the method has no effect
        void                                        createMessagBox( const std::string& title, const std::string& text, float destructionTimeout = 0.0f );

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Callback class for getting clicks from messagebox
        class ClickClb: public yaf3d::MessageBoxDialog::ClickCallback
        {
            public:

                explicit                                ClickClb( EnNetworkNotification* p_entity ) : 
                                                         _p_entity( p_entity )
                                                        {
                                                        }

                void                                    onClicked( unsigned int /* btnId */ )
                                                        {
                                                            _p_entity->_p_msgBox = NULL;
                                                        }

            protected:

                EnNetworkNotification*                       _p_entity;
        };

        //! Version information
        std::string                                 _version;

        //! Message of the day
        std::string                                 _msgOfDay;

        //! Display time
        float                                       _displayTime;

        //! The messagebox
        yaf3d::MessageBoxDialog*                    _p_msgBox;

        //! Networking
        NetworkNotificationNetworking*              _p_networking;

        //! Timeout for destruction of messagebox if the user not already done by clicking on Ok button
        float                                       _destructionTimeOut;

        //! Messagebox destruction timer
        float                                       _timer;
};

//! Entity type definition used for type registry
class NetworkNotificationEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    NetworkNotificationEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_NETWORKNOTIFICATION, yaf3d::BaseEntityFactory::Server | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~NetworkNotificationEntityFactory() {}

        Macro_CreateEntity( EnNetworkNotification );
};

//! Inline methods
inline const bool EnNetworkNotification::isTransformable() const 
{ 
    return false; 
}

inline const std::string& EnNetworkNotification::getVersionInfo() 
{ 
    return _version; 
}

inline void EnNetworkNotification::setVersionInfo( const std::string& version )
{ 
    _version = version; 
}

inline const std::string& EnNetworkNotification::getMsgOfDay()
{ 
    return _msgOfDay; 
}

inline void EnNetworkNotification::setMsgOfDay( const std::string& msg ) 
{ 
    _msgOfDay = msg; 
}

} // namespace vrc


//! Networking class for network notification
class NetworkNotificationNetworking : _RO_DO_PUBLIC_RO( NetworkNotificationNetworking )
{

    public:

        explicit                                    NetworkNotificationNetworking( vrc::EnNetworkNotification* p_entity = NULL );

        virtual                                     ~NetworkNotificationNetworking();

        //! Set the version info string
        void                                        setVersionInfo( const std::string& version );

        //! Set the message of the day string
        void                                        setMsgOfDay( const std::string& msg );

        //! Notify all clients, call this only on server
        void                                        notifyClients( tNotificationData notify );

        // ReplicaNet overrides
        //-----------------------------------------------------------------------------------//

        //! Object can now be initialized in scene ( on clients )
        void                                        PostObjectCreate();

        //! Receive network notification, server sends and clients receive
        void                                        RPC_RecvNotification( tNotificationData notify );

    protected:

        char                                        _p_versionInfo[ 128 ];

        char                                        _p_message[ 256 ];

        vrc::EnNetworkNotification*                 _p_entity;

    friend class _MAKE_RO( NetworkNotificationNetworking );
};

#endif // _VRC_NETWORKNOTIFICATION_H_
