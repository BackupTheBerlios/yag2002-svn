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
 ################################################################*/

#include <ctd_main.h>
#include "ctd_player.h"
#include "ctd_playerimplstandalone.h"

using namespace osg;
using namespace std;

namespace CTD
{

//! Implement and register the player entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerEntityFactory );
    
EnPlayer::EnPlayer() :
_gameMode( GameState::get()->getMode() ),
_p_playerImpl( NULL )
{
    EntityManager::get()->registerUpdate( this, true );         // register entity in order to get updated per simulation step
    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)

    // assign some defaults
    _attributeContainer._chatGuiConfig = "gui/chat.xml";
    _attributeContainer._rot = 0;

    getAttributeManager().addAttribute( "name"                      , _attributeContainer._playerName           );
    getAttributeManager().addAttribute( "physicsentity"             , _attributeContainer._physicsEntity        );
    getAttributeManager().addAttribute( "animationentity"           , _attributeContainer._animationEntity      );
    getAttributeManager().addAttribute( "soundentity"               , _attributeContainer._soundEntity          );
    getAttributeManager().addAttribute( "position"                  , _attributeContainer._pos                  );
    getAttributeManager().addAttribute( "rotation"                  , _attributeContainer._rot                  );
    getAttributeManager().addAttribute( "cameraPosOffsetSpheric"    , _attributeContainer._camPosOffsetSpheric  );
    getAttributeManager().addAttribute( "cameraRotOffsetSpheric"    , _attributeContainer._camRotOffsetSpheric  );
    getAttributeManager().addAttribute( "cameraPosOffsetEgo"        , _attributeContainer._camPosOffsetEgo      );
    getAttributeManager().addAttribute( "cameraRotOffsetEgo"        , _attributeContainer._camRotOffsetEgo      );
    getAttributeManager().addAttribute( "chatGuiConfig"             , _attributeContainer._chatGuiConfig        );
}

EnPlayer::~EnPlayer()
{
    if ( _p_playerImpl )
        delete _p_playerImpl;
}

void EnPlayer::handleNotification( EntityNotification& notify )
{
    if ( _p_playerImpl )
        _p_playerImpl->handleNotification( notify );
}

void EnPlayer::initialize()
{
    // set initial rotation and position
    setPosition( _attributeContainer._pos );
    osg::Quat rotation = osg::Quat( _attributeContainer._rot, osg::Vec3f( 0, 0, 1 ) );
    setRotation( rotation );

    // build the player implementation
    switch ( _gameMode )
    {
        case GameState::Standalone:
        {
            _p_playerImpl = new BasePlayerImplStandalone( this );
            _p_playerImpl->initialize();
        }
        break;

        default:
            assert( NULL && "unsupported game mode" );
    }
}

void EnPlayer::postInitialize()
{
    _p_playerImpl->postInitialize();
}

void EnPlayer::updateEntity( float deltaTime )
{
    _p_playerImpl->update( deltaTime );
}

} // namespace CTD
