/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # base class for all entities
 #
 #   date of creation:  12/01/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

inline const std::string& BaseEntity::getTypeName() 
{ 
    return _typeName; 
} 

inline void BaseEntity::setInstanceName( const std::string& name ) 
{ 
    _instanceName = name; 
}

inline const std::string& BaseEntity::getInstanceName() 
{ 
    return _instanceName; 
} 

inline AttributeManager& BaseEntity::getAttributeManager() 
{ 
    return _attributeManager; 
}

inline void BaseEntity::setTransformationNode( osg::PositionAttitudeTransform* p_trans ) 
{ 
    _p_transformNode = p_trans;
}

inline void BaseEntity::addTransformableNode( osg::Node* p_node )
{
    assert( _p_transformNode && "this entity has no transformation node!" );
    _p_transformNode->addChild( p_node );
}

inline void BaseEntity::setPosition( const osg::Vec3d &pos )
{
    _p_transformNode->setPosition( pos );
}

inline const osg::Vec3d& BaseEntity::getPosition()
{
    return _p_transformNode->getPosition();
}

inline void BaseEntity::setRotation( const osg::Quat& quats )
{
    _p_transformNode->setAttitude( quats );
}

inline const osg::Quat& BaseEntity::getRotation()
{
    return _p_transformNode->getAttitude();
}

