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
 # base class for all entities
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_baseentity.h"
#include "ctd_log.h"
#include "ctd_entitymanager.h"

using namespace std;
using namespace CTD;

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
        if ( _p_transformNode.valid() )
        {
            size_t parents = _p_transformNode->getParents().size();
            if ( parents == 0 )
            {
                log << Log::LogLevel( Log::L_WARNING ) << "the transformation node of entity '" << getInstanceName() << "' has no parent!" << endl;
            }
            else
            {
                for ( size_t p = 0; p < parents; p++ )
                {
                    _p_transformNode->getParent( p )->removeChild( _p_transformNode.get() );
                }
            }
        }
    }
    catch( ... )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "exception occured in destructor of '" << getInstanceName() << "'" << endl;
    }

    // remove all attributes
    _p_attributeManager->removeAllAttributes();
    delete _p_attributeManager;
}

BaseEntity* BaseEntity::clone( const string& instanceName, osg::Group* p_scenegroup )
{
    BaseEntity* p_entity = EntityManager ::get()->createEntity( _typeName, instanceName );
    assert( p_entity );

    // copy attribute values
    p_entity->getAttributeManager() = getAttributeManager();

    // copy transform node if one exists
    if ( _p_transformNode.valid() )
    {
        osg::PositionAttitudeTransform* p_trans = new osg::PositionAttitudeTransform( *_p_transformNode /*, osg::CopyOp::DEEP_COPY_ALL*/ );
        p_entity->setTransformationNode( p_trans );
        if ( !p_scenegroup )
             EntityManager::get()->addToScene( p_entity, _p_transformNode->getParent( 0 ) );
        else
            p_scenegroup->addChild( p_trans );
    }

    return p_entity;
}

BaseEntity* BaseEntity::cloneAndInitialize( const string& instanceName, osg::Group* p_scenegroup )
{
    BaseEntity* p_entity = clone( instanceName, p_scenegroup );
    if ( !p_entity )
        return NULL;

    p_entity->initialize();
    p_entity->postInitialize();
    return p_entity;
}
