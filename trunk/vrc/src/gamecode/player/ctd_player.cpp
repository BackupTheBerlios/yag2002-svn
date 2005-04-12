/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # player
 #
 # this class implements the player
 #
 #
 #   date of creation:  01/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_levelmanager.h>
#include <ctd_physics.h>
#include <ctd_log.h>
#include "ctd_player.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"

using namespace osg;
using namespace std;
using namespace CTD;

//! Implement and register the player entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerEntityFactory );

namespace CTD
{
    
EnPlayer::EnPlayer() :
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_p_playerSound( NULL ),
_playerName( "noname" ),
_rotation( 0 ),
_moveDir( Vec3f( 0, 1, 0 ) ),
_p_inputHandler( NULL )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    getAttributeManager().addAttribute( "name"            , _playerName       );
    getAttributeManager().addAttribute( "physicsentity"   , _physicsEntity    );
    getAttributeManager().addAttribute( "animationentity" , _animationEntity  );
    getAttributeManager().addAttribute( "soundentity"     , _soundEntity      );
    getAttributeManager().addAttribute( "position"        , _position         );
    getAttributeManager().addAttribute( "rotation"        , _rotation         );

    // create a new input handler for this player
    _p_inputHandler = new InputHandler( this );
}

EnPlayer::~EnPlayer()
{
    if ( _p_playerPhysics )
    {
        EntityManager::get()->deleteEntity( _p_playerPhysics );
        _p_playerPhysics->destroy();
    }

    if ( _p_playerAnimation )
    {
        EntityManager::get()->deleteEntity( _p_playerAnimation );
        _p_playerAnimation->destroy();
    }

    if ( _p_playerSound )
    {
        EntityManager::get()->deleteEntity( _p_playerSound );
        _p_playerSound->destroy();
    }

    // delete input handler immediately
    // remove this handler from viewer's handler list
    osgProducer::Viewer::EventHandlerList& eh = Application::get()->getViewer()->getEventHandlerList();
    osgProducer::Viewer::EventHandlerList::iterator beg = eh.begin(), end = eh.end();
    for ( ; beg != end; beg++ )
    {
        if ( *beg == _p_inputHandler )
        {
            eh.erase( beg );
            break;
        }
    }
}

void EnPlayer::initialize()
{
}

void EnPlayer::postInitialize()
{

    log << Log::LogLevel( Log::L_INFO ) << "  initializing player instance '" << getInstanceName() << "' ..." << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _physicsEntity << "' ..." << endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _physicsEntity ) );
    assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type!" );
    _p_playerPhysics->setPlayer( this );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for animation entity '" << _animationEntity << "' ..." << endl;
    // find and attach animation component
    _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _animationEntity ) );
    assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type!" );
    _p_playerAnimation->setPlayer( this );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  animation entity successfully attached" << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for sound entity '" << _soundEntity << "' ..." << endl;
    // find and attach sound component, tollerate missing sound for now
    _p_playerSound = dynamic_cast< EnPlayerSound* >( EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _soundEntity ) );
    if ( !_p_playerSound )
        log << Log::LogLevel( Log::L_ERROR ) << "  *** could not find sound entity '" << _soundEntity << "' of type PlayerSound. player sound deactivated" << endl;
    else
    {
        _p_playerSound->setPlayer( this );
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  sound entity successfully attached" << endl;
    }

    setPosition( _position );

    log << Log::LogLevel( Log::L_INFO ) << "  player instance successfully initialized" << endl;
}

void EnPlayer::updateEntity( float deltaTime )
{
    _p_playerPhysics->update( deltaTime );
}

// input handler implementation
//-----------------------------
EnPlayer::InputHandler::InputHandler( EnPlayer*p_player ) : 
_p_player( p_player ) ,
_rotateRight( false ),
_rotateLeft( false ),
_moveForward( false ),
_moveBackward( false )
{
    // register us in viewer to get event callbacks
    osg::ref_ptr< EnPlayer::InputHandler > ih( this );
    Application::get()->getViewer()->getEventHandlerList().push_back( ih.get() );
}

EnPlayer::InputHandler::~InputHandler() 
{
}

bool EnPlayer::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    unsigned int eventType   = ea.getEventType();
    int          key         = ea.getKey();

    // dispatch key activity
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        if ( key == osgGA::GUIEventAdapter::KEY_Up )
            _moveForward = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Down )
            _moveBackward = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Right )
            _rotateRight = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Left )
            _rotateLeft = true;

        if ( key == 'm' )
        {
            _p_player->_p_playerPhysics->jump();
            _p_player->_p_playerAnimation->animJump();
        }
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {
        if ( ( key == osgGA::GUIEventAdapter::KEY_Up ) || ( key == osgGA::GUIEventAdapter::KEY_Down ) )
        {
            if ( key == osgGA::GUIEventAdapter::KEY_Up )
                _moveForward = false;

            if ( key == osgGA::GUIEventAdapter::KEY_Down )
                _moveBackward = false;

            _p_player->_p_playerPhysics->setForce( 0, 0 );
        }

        if ( key == osgGA::GUIEventAdapter::KEY_Right )
            _rotateRight = false;

        if ( key == osgGA::GUIEventAdapter::KEY_Left )
            _rotateLeft = false;
    }

    if ( _rotateRight )
    {
        _p_player->_rotation += _p_player->_p_playerPhysics->getAngularForce();
        if ( _p_player->_rotation > PI * 2.0f )
            _p_player->_rotation -= PI * 2.0f;

        _p_player->_moveDir._v[ 0 ] = sinf( _p_player->_rotation );
        _p_player->_moveDir._v[ 1 ] = cosf( _p_player->_rotation );

        _p_player->_p_playerAnimation->animTurn();
    }

    if ( _rotateLeft )
    {
        if ( _p_player->_rotation < 0 )
            _p_player->_rotation += PI * 2.0f;
        _p_player->_rotation -= _p_player->_p_playerPhysics->getAngularForce();

        _p_player->_moveDir._v[ 0 ] = sinf( _p_player->_rotation );
        _p_player->_moveDir._v[ 1 ] = cosf( _p_player->_rotation );

        _p_player->_p_playerAnimation->animTurn();
    }

    if ( _moveForward )
    {
        _p_player->_p_playerPhysics->setForce( _p_player->_moveDir._v[ 0 ], _p_player->_moveDir._v[ 1 ] );
        _p_player->_p_playerAnimation->animWalk();
    }

    if ( _moveBackward )
    {
        _p_player->_p_playerPhysics->setForce( -_p_player->_moveDir._v[ 0 ], -_p_player->_moveDir._v[ 1 ] );
        _p_player->_p_playerAnimation->animWalk();
    }

    if ( !_moveForward && !_moveBackward )
    {
        if ( !_rotateLeft && !_rotateRight )
            _p_player->_p_playerAnimation->animIdle();

        if ( _p_player->getPlayerSound() )
            _p_player->getPlayerSound()->stopPlayingAll();
    }

    return false; // let other handlers get all inputs handled here
}

} // namespace CTD
