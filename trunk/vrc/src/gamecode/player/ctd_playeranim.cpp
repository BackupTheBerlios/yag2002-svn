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
 # player animation
 #
 # this class implements the player animation control
 #
 #   date of creation:  03/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_physics.h>
#include <ctd_log.h>
#include <ctd_application.h>
#include "ctd_playeranim.h"
#include "ctd_player.h"

using namespace osg;
using namespace rbody;
using namespace std;


#define ACTION_IDLE     "ACT_IDLE_LONG"
#define ACTION_WALK     "ACT_WALK"
#define ACTION_JUMP     "ACT_STAND"

namespace CTD
{

//! Implement and register the player animation entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerAnimationEntityFactory );

EnPlayerAnimation::EnPlayerAnimation() :
_p_player( NULL ),
_p_characterGrp( NULL ),
_p_replicantBodyMgr( ReplicantBodyMgr::instance() ),
_p_node( NULL ),
_p_body( NULL )
{ 
    // the deletion must not be controled by entity manager, but by player
    setAutoDelete( false );

    // register attributes
    getAttributeManager().addAttribute( "animconfig"   , _animCfgFile );
}

EnPlayerAnimation::~EnPlayerAnimation()
{
}

void EnPlayerAnimation::initialize()
{
    try
    {
        if ( !_animCfgFile.length() )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "*** missing animation config file parameter" << endl;
            return;
        }

        string file = Application::get()->getMediaPath() + _animCfgFile;

        // create a body from the core 
        OsgBodyNode* p_osgBody = new OsgBodyNode( file, NULL );

        p_osgBody->setName( getInstanceName() );
        p_osgBody->setUpdateMode( OsgBodyNode::UPDATE_NONE );

        // set default action if one exists
        ActionRequest* p_action = p_osgBody->getBody()->getActionPrototype( p_osgBody->getBody()->getCoreBody()->getDefaultActivity() );
        if ( p_action ) 
        {
            // This action should be low priority
            p_action->setPrioritized( false );
            p_osgBody->getBody()->executeAction( p_action, false );
            p_osgBody->getBody()->update( 0 );
        }
        _p_body = p_osgBody;

        _p_node = new PositionAttitudeTransform;
        _p_node->addChild( p_osgBody );

        //! TODO: implement support for offesting position and rotations in rbody file
        float offset = p_osgBody->getBody()->getCoreBody()->getFootOffset();
        _p_node->setPosition( Vec3f( 0, 0, offset ) );

        // we are updated by player entity, so disable getting updated by the entity system
        activate( false );
    }
    catch ( std::exception& e ) 
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** problem evaluating character anim file '" << _animCfgFile << "'" << endl;
        log << Log::LogLevel( Log::L_ERROR ) << "***  reason: " << e.what() << endl;
        assert( NULL );
        return;
    }
}

void EnPlayerAnimation::setPlayer( EnPlayer* p_player )
{
    _p_player = p_player;
    // add the new mesh into player's transformable scene group
    _p_player->addToTransformableNode( _p_node );
}

void EnPlayerAnimation::destroy()
{
    _p_player->removeFromTransformationNode( _p_node );
}

void EnPlayerAnimation::update( float deltaTime )
{
    _p_body->getBody()->update( deltaTime );
}

void EnPlayerAnimation::actionIdle()
{
    if ( _action == eIdle )
        return;

    ActionRequest* p_action = _p_body->getBody()->getActionPrototype( ACTION_IDLE );
    _p_body->getBody()->executeAction( p_action, true );

    if ( _action == eWalk )
        _p_body->getBody()->stopAction( ACTION_WALK );

    if ( _action == eJump )
        _p_body->getBody()->stopAction( ACTION_JUMP );

    _action = eIdle;
}

void EnPlayerAnimation::actionWalk()
{
    if ( _action == eWalk )
        return;

    ActionRequest* p_action = _p_body->getBody()->getActionPrototype( ACTION_WALK );
    _p_body->getBody()->executeAction( p_action, false );

    if ( _action == eIdle )
        _p_body->getBody()->stopAction( ACTION_IDLE );

    if ( _action == eJump )
        _p_body->getBody()->stopAction( ACTION_JUMP );

    _action = eWalk;
}

void EnPlayerAnimation::actionJump()
{
    if ( _action == eJump )
        return;

    ActionRequest* p_action = _p_body->getBody()->getActionPrototype( ACTION_JUMP );
    _p_body->getBody()->executeAction( p_action, false );

    if ( _action == eIdle )
        _p_body->getBody()->stopAction( ACTION_IDLE );

    if ( _action == eWalk )
        _p_body->getBody()->stopAction( ACTION_WALK );

    _action = eJump;
}

} // namespace CTD
