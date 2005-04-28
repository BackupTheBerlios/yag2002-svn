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
 #   date of creation:  12/01/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

using namespace std;
using namespace CTD;
 
BaseEntity::~BaseEntity()
{
    if ( _p_transformNode )
    {
        _p_transformNode->getParent( 0 )->removeChild( _p_transformNode );
        _p_transformNode = NULL;
    }
}

BaseEntity* BaseEntity::clone( const string& instanceName, osg::Group* p_scenegroup )
{
    BaseEntity* p_entity = EntityManager ::get()->createEntity( _typeName, instanceName );
    assert( p_entity );

    // copy attribute values
    vector< EntityAttributeBase* >& attributes = getAttributeManager().getAttributes();
    vector< EntityAttributeBase* >::iterator attr = attributes.begin(), attrEnd = attributes.end();
    for ( ; attr != attrEnd; attr++ )
    {
        p_entity->getAttributeManager().setAttributeValue( ( *attr )->getName(), **attr );
    }

    // copy transform node if one exists
    if ( _p_transformNode )
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