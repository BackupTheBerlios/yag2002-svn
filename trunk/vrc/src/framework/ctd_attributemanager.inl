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
 # attribute manager primarily used for game entities
 #
 #   date of creation:  01/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


template< class TypeT >
EntityAttribute< TypeT >::EntityAttribute( const std::string& name, TypeT &ref ) :
_name( name ),
_reference( ref )
{
    // set the proper attibute type
    if (      typeid( ref ) == typeid( int ) )              _type = EntityAttributeType::INTEGER;
    else if ( typeid( ref ) == typeid( float ) )            _type = EntityAttributeType::FLOAT;
    else if ( typeid( ref ) == typeid( bool ) )             _type = EntityAttributeType::BOOL;
    else if ( typeid( ref ) == typeid( osg::Vec3f ) )       _type = EntityAttributeType::VECTOR3;
    else if ( typeid( ref ) == typeid( std::string ) )      _type = EntityAttributeType::STRING;
    else EntityAttributeType::UNKNOWN;
}

template< class TypeT >
TypeT EntityAttribute< TypeT >::getValue() 
{ 
    return _reference; 
}

template< class TypeT >
void EntityAttribute< TypeT >::setValue( TypeT value ) 
{ 
    _reference = value; 
}

template< class TypeT >
const std::string& EntityAttribute< TypeT >::getName()
{
    return _name;
}

template< class TypeT >
unsigned int EntityAttribute< TypeT >::getType()
{
    return _type;
}

template< class TypeT >
const type_info& EntityAttribute< TypeT >::getTypeInfo()
{
    return typeid( TypeT );
}


inline std::vector< EntityAttributeBase* >& AttributeManager::getAttributes()
{
    return _attributes;
}

template< class TypeT >
inline void AttributeManager::addAttribute( const std::string &name, TypeT &_reference )
{
    _attributes.push_back( new EntityAttribute< TypeT >( name, _reference ) );
}

template< class TypeT >
bool AttributeManager::getAttributeValue( const std::string& name, TypeT& value ) 
{
    std::vector< EntityAttributeBase* >::iterator pp_attr = _attributes.begin(), pp_end = _attributes.end();
    while ( pp_attr != pp_end ) {
        if ( ( *pp_attr )->getName() == name ) {
            assert( typeid( TypeT ) == ( *pp_attr )->getTypeInfo() && "*** wrong type for requested attribute!" );
            value = ( static_cast< EntityAttribute< TypeT >* >( *pp_attr ) )->getValue();
            return true;
        }
        pp_attr++;
    }
    return false;
}


template< class TypeT >
bool AttributeManager::setAttributeValue( const std::string& name, TypeT value ) 
{
    std::vector< EntityAttributeBase* >::iterator pp_attr = _attributes.begin(), pp_end = _attributes.end();
    while ( pp_attr != pp_end ) {
        if ( ( *pp_attr )->getName() == name ) {
            assert( typeid( TypeT ) == ( *pp_attr )->getTypeInfo() && "*** wrong type for requested attribute!" );
            ( static_cast< EntityAttribute< TypeT >* >( *pp_attr ) )->setValue( value );
            return true;
        }
        pp_attr++;
    }
    return false;
}
