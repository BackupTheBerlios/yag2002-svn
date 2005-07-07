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
TypeT EntityAttribute< TypeT >::getValue() const
{ 
    return _reference; 
}

template< class TypeT >
void EntityAttribute< TypeT >::setValue( TypeT value ) 
{ 
    _reference = value; 
}

template< class TypeT >
const std::string& EntityAttribute< TypeT >::getName() const
{
    return _name;
}

template< class TypeT >
unsigned int EntityAttribute< TypeT >::getType() const
{
    return _type;
}

template< class TypeT >
const type_info& EntityAttribute< TypeT >::getTypeInfo() const
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
    EntityAttribute< TypeT >* p_attr = new EntityAttribute< TypeT >( name, _reference );
    _attributes.push_back( p_attr );
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
            if ( typeid( TypeT ) != ( *pp_attr )->getTypeInfo() )
            {
                log << Log::LogLevel( Log::L_ERROR ) << "*** wrong type for requested attribute '" << name << "', skipping!" << endl;
                pp_attr++;
                continue;
            }
            ( static_cast< EntityAttribute< TypeT >* >( *pp_attr ) )->setValue( value );
            return true;
        }
        pp_attr++;
    }
    return false;
}
