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

#ifndef _VRC_UPDATENOTIFIER_H_
#define _VRC_UPDATENOTIFIER_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_UpdateNotifier.h"

//! Networking class
class UpdateNotifierNetworking;

namespace vrc
{

#define ENTITY_NAME_UPDATENOTIFIER    "UpdateNotifier"

//! Entity UpdateNotifier
class EnUpdateNotifier :  public yaf3d::BaseEntity
{
    public:
                                                    EnUpdateNotifier();

        virtual                                     ~EnUpdateNotifier();

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

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Callback class for getting clicks from messagebox
        class ClickClb: public yaf3d::MessageBoxDialog::ClickCallback
        {
            public:

                explicit                                ClickClb( EnUpdateNotifier* p_entity ) : 
                                                         _p_entity( p_entity )
                                                        {
                                                        }

                void                                    onClicked( unsigned int /* btnId */ )
                                                        {
                                                            _p_entity->_p_msgBox = NULL;
                                                        }

            protected:

                EnUpdateNotifier*                       _p_entity;
        };

        //! Version information       
        std::string                                 _version;

        //! Message of the day
        std::string                                 _msgOfDay;

        //! The messagebox
        yaf3d::MessageBoxDialog*                    _p_msgBox;

        //! Networking
        UpdateNotifierNetworking*                   _p_networking;

        //! Timeout for destruction of messagebox if the user not already done by clicking on Ok button
        float                                       _destructionTimeOut;
        //! Messagebox destruction counter
        float                                       _cnt;
};

//! Entity type definition used for type registry
class UpdateNotifierEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    UpdateNotifierEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_UPDATENOTIFIER, yaf3d::BaseEntityFactory::Server | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~UpdateNotifierEntityFactory() {}

        Macro_CreateEntity( EnUpdateNotifier );
};

//! Inline methods
inline const bool EnUpdateNotifier::isTransformable() const 
{ 
    return false; 
}

inline const std::string& EnUpdateNotifier::getVersionInfo() 
{ 
    return _version; 
}

inline void EnUpdateNotifier::setVersionInfo( const std::string& version )
{ 
    _version = version; 
}

inline const std::string& EnUpdateNotifier::getMsgOfDay()
{ 
    return _msgOfDay; 
}

inline void EnUpdateNotifier::setMsgOfDay( const std::string& msg ) 
{ 
    _msgOfDay = msg; 
}

} // namespace vrc


//! Networking class for update notifier
class UpdateNotifierNetworking : _RO_DO_PUBLIC_RO( UpdateNotifierNetworking )
{

    public:

        explicit                                    UpdateNotifierNetworking( vrc::EnUpdateNotifier* p_entity = NULL );

        virtual                                     ~UpdateNotifierNetworking();

        //! Set the version info string
        void                                        setVersionInfo( const std::string& version );

        //! Set the message of the day string
        void                                        setMsgOfDay( const std::string& msg );

        // ReplicaNet overrides
        //-----------------------------------------------------------------------------------//

        //! Object can now be initialized in scene ( on clients )
        void                                        PostObjectCreate();

    protected:

        char                                        _p_versionInfo[ 128 ];

        char                                        _p_message[ 256 ];

    friend class _MAKE_RO( UpdateNotifierNetworking );
};

#endif // _VRC_UPDATENOTIFIER_H_
