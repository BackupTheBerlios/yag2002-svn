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

#ifndef _VRC_PLAYER_H_
#define _VRC_PLAYER_H_

#include <vrc_main.h>


class PlayerNetworking;

namespace vrc
{

//! Entity name
#define ENTITY_NAME_PLAYER                "Player"

class EnPlayerAnimation;
class BasePlayerImplementation;

//! Player entity
class EnPlayer : public yaf3d::BaseEntity
{
    public:


                                                    EnPlayer();

        virtual                                     ~EnPlayer();


        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Post-initializing function, this is called after all plugins' entities are initilized.
        //! One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        void                                        postInitialize();

        //! Get player's implementation. This method is used by player's networking component when new clients connect to networking session.
        inline BasePlayerImplementation*            getPlayerImplementation();

        //! Get player's IP address, used for remote clients
        inline std::string                          getIPAdress();

        //! Is voice chat enabled for this player?
        inline bool                                 isVoiceChatEnabled();

        //! Get last update time
        inline float                                getDeltaTime() const;

        //! Get player name
        const std::string                           getPlayerName() const;

        //! Set player name
        void                                        setPlayerName( const std::string& name );

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

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Override this method of yaf3d::BaseEntity to get notifications (from menu system)
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Spawn player considering spawn points in level
        void                                        spawn();

        //! Use this method for adding a node to player's transformation node
        inline void                                 appendTransformationNode( osg::Node* p_node );

        //! Use this method for removing a node from player's transformation node
        inline void                                 removeTransformationNode( osg::Node* p_node );

        //! Set player's implementation. This method is used by player's networking component when new clients connect to networking session.
        inline void                                 setPlayerImplementation( BasePlayerImplementation* p_impl );

        //! Set the network ip address for this player. It is used by networking component for remote clients.
        inline void                                 setIPAdress( const std::string& ip );

        //! Set the voice chat flag for this player. It is used by networking component for remote clients.
        inline void                                 setVoiceChatEnabled( bool en );

        //! Player attributes encapsulated in a container, ready for transfering to player implementation
        PlayerAttributes                            _attributeContainer;

        //! Game mode ( can be Server, Client, or Standalone )
        unsigned int                                _gameMode;

        //! Player implementation
        BasePlayerImplementation*                   _p_playerImpl;

        //! Network IP address, used for getting remote clients' ip address
        std::string                                 _ipAddress;

        //! Indicated whether voice chat is enabled for this player ( including remote clients )
        bool                                        _voiceChatEnabled;

        //! Stored deltaTime needed by some player components
        float                                       _deltaTime;

    friend class ::PlayerNetworking;
    friend class EnPlayerAnimation;
};

//! Entity type definition used for type registry
class PlayerEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLAYER, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client | yaf3d::BaseEntityFactory::Server )
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

inline void EnPlayer::setPlayerImplementation( BasePlayerImplementation* p_impl )
{
    assert( !_p_playerImpl && "player implementation must be set only once" );
    _p_playerImpl = p_impl;
}

inline BasePlayerImplementation* EnPlayer::getPlayerImplementation()
{
    return _p_playerImpl;
}

inline void EnPlayer::setIPAdress( const std::string& ip )
{
    _ipAddress = ip;
}

inline std::string EnPlayer::getIPAdress()
{
    return _ipAddress;
}

inline bool EnPlayer::isVoiceChatEnabled()
{
    return _voiceChatEnabled;
}

inline void EnPlayer::setVoiceChatEnabled( bool en )
{
    _voiceChatEnabled = en;
}

inline float EnPlayer::getDeltaTime() const
{
    return _deltaTime;
}

} // namespace vrc

#endif // _VRC_PLAYER_H_
