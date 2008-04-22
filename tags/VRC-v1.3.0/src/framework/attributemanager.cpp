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
 # attribute manager primarily used for game entities
 #
 #   date of creation:  01/13/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <base.h>
#include <attributemanager.h>

namespace yaf3d
{

AttributeManager::~AttributeManager()
{ 
    removeAllAttributes(); 
}

void AttributeManager::getAttributesAsString( std::vector< std::pair< std::string, std::string > >& attributes )
{
    std::vector< BaseEntityAttribute* >::iterator p_beg = _attributes.begin(), p_end = _attributes.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        BaseEntityAttribute* p_attribute = *p_beg;
        std::stringstream strvalue;
        unsigned int type = p_attribute->getType();
        switch ( type ) 
        {
            case EntityAttributeType::FLOAT:
            {
                float value = dynamic_cast< EntityAttribute< float >* >( p_attribute )->getValue();
                strvalue << value;
            }
            break;

            case EntityAttributeType::BOOL:
            {
                bool value = dynamic_cast< EntityAttribute< bool >* >( p_attribute )->getValue();
                strvalue << ( value ? "true" : "false" );
            }
            break;

            case EntityAttributeType::INTEGER:
            {
                int value = dynamic_cast< EntityAttribute< int >* >( p_attribute )->getValue();
                strvalue << value;
            }
            break;

            case EntityAttributeType::VECTOR2:
            {
                osg::Vec2f value = dynamic_cast< EntityAttribute< osg::Vec2f >* >( p_attribute )->getValue();
                strvalue << value.x() << " " << value.y();
            }
            break;

            case EntityAttributeType::VECTOR3:
            {
                osg::Vec3f value = dynamic_cast< EntityAttribute< osg::Vec3f >* >( p_attribute )->getValue();
                strvalue << value.x() << " " << value.y() << " " << value.z();
            }
            break;

            case EntityAttributeType::STRING:
            {
                std::string value = dynamic_cast< EntityAttribute< std::string >* >( p_attribute )->getValue();
                strvalue << value;
            }
            break;

            default:
                assert( NULL && "invalid attribute type!" );
        }
        attributes.push_back( make_pair( p_attribute->getName(), strvalue.str() ) );
    }
}

bool AttributeManager::setAttributeValue( const std::string &name, const std::string &type, const std::string &value )
{
    bool bRet = false;
    std::stringstream strBuffer( value );

    // the type information is case-insensitve
    std::string citype;    
    for ( size_t i = 0; i < type.size(); ++i ) citype += toupper( type[ i ] );

    if ( ( citype == "BOOLEAN" ) || ( citype == "BOOL" ) ) 
    {
        std::string bVal;
        strBuffer >> bVal;
        bool   bValue = ( bVal == "true" ) ? true : false;
        bRet = setAttributeValue( name, bValue );
    }
    else if ( citype == "INTEGER" ) 
    {
        int iValue;
        strBuffer >> iValue;
        bRet = setAttributeValue( name, iValue );
    }
    else if ( citype == "FLOAT" ) 
    {
        float fValue;
        strBuffer >> fValue;
        bRet = setAttributeValue( name, fValue );
    }
    else if ( citype == "VECTOR2" ) 
    {
        osg::Vec2f vecValue;
        strBuffer >> vecValue._v[ 0 ] >> vecValue._v[ 1 ];
        bRet = setAttributeValue( name, vecValue );
    }
    else if ( citype == "VECTOR3" ) 
    {
        osg::Vec3f vecValue;
        strBuffer >> vecValue._v[ 0 ] >> vecValue._v[ 1 ] >> vecValue._v[ 2 ];
        bRet = setAttributeValue( name, vecValue );
    }
    else if ( citype == "STRING" ) 
    {
        bRet = setAttributeValue( name, value );
    }
    else
        log_error << "trying to set unsupported attribute type '" << citype << "'" << std::endl;

    return bRet;
}

bool AttributeManager::setAttributeValueByString( const std::string& name, const std::string& valuestring )
{        
    std::vector< BaseEntityAttribute* >::iterator p_beg = _attributes.begin(), p_end = _attributes.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->getName() == name )
            break;
    }
    if ( p_beg == p_end )
    {
        log_error << "cannot find attribute '" << name << "'" << std::endl;
        return false;
    }

    BaseEntityAttribute* p_attribute = *p_beg;
    std::stringstream strvalue;
    strvalue << valuestring;
    unsigned int type = p_attribute->getType();
    switch ( type ) 
    {
        case EntityAttributeType::FLOAT:
        {
            float value;
            strvalue >> value;
            setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::BOOL:
        {
            bool value = ( valuestring == "true" ? true : false );
            setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::INTEGER:
        {
            int value;
            strvalue >> value;
            setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::VECTOR2:
        {
            osg::Vec2f value;
            strvalue >> value._v[ 0 ] >> value._v[ 1 ];
            setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::VECTOR3:
        {
            osg::Vec3f value;
            strvalue >> value._v[ 0 ] >> value._v[ 1 ] >> value._v[ 2 ];
            setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::STRING:
        {
            setAttributeValue( name, valuestring );
        }
        break;

        default:
            assert( NULL && "invalid attribute type!" );
    }

    return true;
}

bool AttributeManager::setAttributeValue( const std::string& name, const BaseEntityAttribute& attribute )
{
    unsigned int type = attribute.getType();
    switch ( type ) 
    {
        case EntityAttributeType::FLOAT:
        {
            float value = dynamic_cast< const EntityAttribute< float >& >( attribute ).getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::BOOL:
        {
            bool value = dynamic_cast< const EntityAttribute< bool >& >( attribute ).getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::INTEGER:
        {
            int value = dynamic_cast< const EntityAttribute< int >& >( attribute ).getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::VECTOR2:
        {
            osg::Vec2f value = dynamic_cast< const EntityAttribute< osg::Vec2f >& >( attribute ).getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::VECTOR3:
        {
            osg::Vec3f value = dynamic_cast< const EntityAttribute< osg::Vec3f >& >( attribute ).getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::STRING:
        {
            std::string value = dynamic_cast< const EntityAttribute< std::string >& >( attribute ).getValue();
            return setAttributeValue( name, value );
        }
        break;

        default:
            assert( NULL && "invalid attribute type!" );
    }

    return false;
}

void AttributeManager::removeAllAttributes()
{
    std::vector< BaseEntityAttribute* >::iterator pp_attr = _attributes.begin(), p_end = _attributes.end();
    while ( pp_attr != p_end ) 
    {
        delete *pp_attr;
        pp_attr++;
    }
    _attributes.clear();
}

AttributeManager& AttributeManager::operator = ( AttributeManager& attr )
{
    if ( &attr == this )
        return *this;

    // copy attribute values
    std::vector< BaseEntityAttribute* >::iterator p_beg = attr.getAttributes().begin(), p_end = attr.getAttributes().end();
    for ( ; p_beg != p_end; ++p_beg )
        setAttributeValue( ( *p_beg )->getName(), **p_beg );

    return *this;
}

} // namespace yaf3d
