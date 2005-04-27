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

inline void BaseEntity::setAutoDelete( bool autoDel )
{
    _autoDelete = autoDel;
}

inline bool BaseEntity::getAutoDelete()
{
    return _autoDelete;
}

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

inline osg::PositionAttitudeTransform* BaseEntity::getTransformationNode() 
{ 
    return _p_transformNode;
}

inline void BaseEntity::addToTransformationNode( osg::Node* p_node )
{
    assert( _p_transformNode && "this entity has no transformation node!" );
    _p_transformNode->addChild( p_node );
}

inline void BaseEntity::removeFromTransformationNode( osg::Node* p_node )
{
    if ( _p_transformNode )
    {
        _p_transformNode->removeChild( p_node );
    }
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

