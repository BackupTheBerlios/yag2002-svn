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
 # entity candle including fire rendering and light animation
 #
 #   date of creation:  07/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_candle.h"
#include "ctd_pointlight.h"

using namespace std;

namespace CTD
{

//! Implement and register the candle entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( CandleEntityFactory );

// Implementation of water entity
EnCandle::EnCandle() :
_sizeX( 1000.0f ),
_sizeY( 1000.0f ),
_p_pointLightEntity( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "lightEntity"           , _lightEntity           );
    getAttributeManager().addAttribute( "position"              , _position              );
    getAttributeManager().addAttribute( "sizeX"                 , _sizeX                 );
    getAttributeManager().addAttribute( "sizeY"                 , _sizeY                 );
}

EnCandle::~EnCandle()
{
}

void EnCandle::handleNotification( const EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // disable water rendering when in menu
        case CTD_NOTIFY_MENU_ENTER:
            break;

        // enable water entity when get back in game
        case CTD_NOTIFY_MENU_LEAVE:
            break;

        case CTD_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            break;

        default:
            ;
    }
}

void EnCandle::postInitialize()
{
    _p_pointLightEntity = dynamic_cast< EnPointLight* >( EntityManager::get()->findEntity( ENTITY_NAME_POINTLIGHT, _lightEntity ) );
    if ( !_p_pointLightEntity )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "Canlde: cannot find PointLight entity '" << _lightEntity << "', animation disabled!" << endl;
        return;
    }

    _p_pointLightEntity->setPosition( _position );

    // register entity in order to get notifications (e.g. from menu entity)
    EntityManager::get()->registerNotification( this, true );   
    EntityManager::get()->registerUpdate( this, true );   
}

void EnCandle::updateEntity( float deltaTime )
{
    // TODO: replace this fast hack by a generic light animation algorithm
    static float v = 0;
    v += 0.80f * deltaTime;

    float z = 0.025f * sinf( v );
    float lc = 0.01 + 0.01f * cosf( v - osg::PI * 0.3f );
    float la = 0.05f + 0.05f * cosf( v );

    osg::Light* p_light = _p_pointLightEntity->getLight();
    p_light->setConstantAttenuation( lc );
    p_light->setLinearAttenuation( la );
    osg::Vec3f pos = _position;
    pos._v[ 2 ] += z;
    _p_pointLightEntity->setPosition( pos );
}

}
