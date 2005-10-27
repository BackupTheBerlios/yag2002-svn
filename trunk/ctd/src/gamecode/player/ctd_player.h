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
 # player entiy
 #
 # the actual behaviour is defined by one of its implementations 
 #  for Server, Client, or Standalone.
 #
 #   date of creation:  01/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/14/2005 boto       creation of Player
 #
 #  05/27/2005 boto       separation of player implementation for
 #                         Server, Client, and Standalone
 #
 ################################################################*/

#ifndef _CTD_PLAYER_H_
#define _CTD_PLAYER_H_

#include <ctd_main.h>

namespace CTD
{

//! Entity name
#define ENTITY_NAME_PLAYER              "Player"

//! Notification for player deletion ( all entity specific notification begin with 0xA )
#define CTD_NOTIFY_PLAYER_DESTRUCTION   0xA0000010

class BasePlayerImplementation;

//! Player entity
class EnPlayer : public BaseEntity
{
    public:


                                                    EnPlayer();

        virtual                                     ~EnPlayer();


        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        void                                        postInitialize();

        //! Use this method for adding a node to player's transformation node
        inline void                                 appendTransformationNode( osg::Node* p_node );

        //! Use this method for removing a node from player's transformation node
        inline void                                 removeTransformationNode( osg::Node* p_node );

        //! Set player's implementation. This method is used by player's networking component when new clients connect to networking session.
        inline void                                 setPlayerImplementation( BasePlayerImplementation* p_impl );

        //! Get player's implementation. This method is used by player's networking component when new clients connect to networking session.
        inline BasePlayerImplementation*            getPlayerImplementation();

        //! Register an entity for getting player deletion notification
        void                                        registerNotifyDeletion( BaseEntity* p_entity );

        //! Get last update time
        inline float                                getDeltaTime() const;

        //! Entity attribute container
        class PlayerAttributes
        {
            public:

                //! Physics entity's instance name which will be attached to player
                std::string                                 _physicsEntity;

                //! Animation entity's instance name which will be attached to player
                std::string                                 _animationEntity;

                //! Sound entity's instance name which will be attached to player
                std::string                                 _soundEntity;

                //! Initial position
                osg::Vec3f                                  _pos;

                //! Initial rotation about Z axis
                float                                       _rot;

                //! Camera's position offset for spheric mode
                osg::Vec3f                                  _camPosOffsetSpheric;

                //! Camera's rotation offset for spheric mode ( roll/pitch/yaw in degrees )
                osg::Vec3f                                  _camRotOffsetSpheric;

                //! Camera's position offset for ego mode
                osg::Vec3f                                  _camPosOffsetEgo;

                //! Camera's rotation offset for ego mode ( roll/pitch/yaw in degrees )
                osg::Vec3f                                  _camRotOffsetEgo;

                //! CEGUI layout file for built-in chat
                std::string                                 _chatGuiConfig;
        };

        //! Return player's attribute container
        inline const PlayerAttributes &             getPlayerAttributes() const;

        //! Get player name
        inline const std::string&                   getPlayerName() const;

        //! Set player name
        inline void                                 setPlayerName( const std::string& name );

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Override this method of BaseEntity to get notifications (from menu system)
        void                                        handleNotification( const EntityNotification& notification );

        //! Spawn player considering spawn points in level
        void                                        spawn();

        //! Player name
        std::string                                 _playerName;

        //! Player attributes encapsulated in a container, ready for transfering to player implementation
        PlayerAttributes                            _attributeContainer;

        //! Game mode ( can be Server, Client, or Standalone )
        unsigned int                                _gameMode;

        //! Player implementation
        BasePlayerImplementation*                   _p_playerImpl;

        //! List of registered entities for getting deletion notification
        std::vector< BaseEntity* >                  _deletionNotifications;

        //! Stored deltaTime needed by some player components
        float                                       _deltaTime;
};

//! Entity type definition used for type registry
class PlayerEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_PLAYER, BaseEntityFactory::Standalone | BaseEntityFactory::Client | BaseEntityFactory::Server )
                                                    {}

        virtual                                     ~PlayerEntityFactory() {}

        Macro_CreateEntity( EnPlayer );
};

inline const EnPlayer::PlayerAttributes& EnPlayer::getPlayerAttributes() const
{ 
    return _attributeContainer; 
}

inline void EnPlayer::appendTransformationNode( osg::Node* p_node )
{
    addToTransformationNode( p_node );
}

inline void EnPlayer::removeTransformationNode( osg::Node* p_node )
{
    removeFromTransformationNode( p_node );
}

inline const std::string& EnPlayer::getPlayerName() const
{
    return _playerName;
}

inline void EnPlayer::setPlayerName( const std::string& name )
{
    _playerName = name;
    // update the instance name
    setInstanceName( name );
}

inline void EnPlayer::setPlayerImplementation( BasePlayerImplementation* p_impl )
{
    assert( !_p_playerImpl && "player implementation must be set only once" );
    _p_playerImpl = p_impl;
}

inline BasePlayerImplementation* EnPlayer::getPlayerImplementation()
{
    return _p_playerImpl;
}

inline float EnPlayer::getDeltaTime() const
{
    return _deltaTime;
}

} // namespace CTD

#endif // _CTD_PLAYER_H_
