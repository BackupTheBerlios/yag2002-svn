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
 # base class for all entities
 #
 #   date of creation:  02/17/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <base.h>
#include "baseentity.h"
#include "log.h"
#include "entitymanager.h"
#include "gamestate.h"


namespace yaf3d
{

BaseEntity::BaseEntity() :
_p_attributeManager( NULL ),
_initialized( false )
{
    _p_attributeManager = new AttributeManager;
}

BaseEntity::~BaseEntity()
{
    try
    {
        // remove transformation node from its parents
        if ( _p_transformNode.valid() )
        {
            unsigned int parents = _p_transformNode->getNumParents();
            if ( parents > 0 )
            {
                for ( unsigned int p = 0; p < parents; ++p )
                {
                    // every removal of child also removes the parent form parent list! so remove always form index 0.
                    _p_transformNode->getParent( 0 )->removeChild( _p_transformNode.get() );
                }
            }
        }
    }
    catch( ... )
    {
        log_error << "exception occured in destructor of '" << getInstanceName() << "'" << std::endl;
    }

    // delete attribute manager
    delete _p_attributeManager;
}

BaseEntity* BaseEntity::clone( const std::string& instanceName )
{
    BaseEntity* p_entity = EntityManager ::get()->createEntity( _typeName, instanceName );
    assert( p_entity );

    // copy attribute values
    p_entity->getAttributeManager() = getAttributeManager();

    return p_entity;
}

BaseEntity* BaseEntity::cloneAndInitialize( const std::string& instanceName )
{
    BaseEntity* p_entity = clone( instanceName );
    if ( !p_entity )
        return NULL;

    // consider the initialization policy
    BaseEntityFactory* p_factory = EntityManager::get()->getEntityFactory( p_entity->getTypeName() );
    assert( p_factory && "entity factory could not be found!" );

    unsigned int initpolicy = p_factory->getInitializationPolicy();

    if ( ( initpolicy == BaseEntityFactory::AnyTime ) ||
         ( ( initpolicy == BaseEntityFactory::OnLoadingLevel ) && ( GameState::get()->getState() == GameState::StartingLevel ) ) ||
         ( ( initpolicy == BaseEntityFactory::OnRunningLevel ) && ( GameState::get()->getState() == GameState::MainLoop ) )
       )
    {
        p_entity->initialize();
        p_entity->postInitialize();
    }

    return p_entity;
}

} // namespace yaf3d
