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

#include <ctd_base.h>
#include <ctd_attributemanager.h>

using namespace std;
namespace CTD
{

bool AttributeManager::setAttributeValue( const string &name, const string &type, const string &value )
{
    bool bRet = false;
    stringstream strBuffer( value );

    // the type information is case-insensitve
    string citype;    
    for ( size_t i = 0; i < type.size(); i++ ) citype += toupper( type[ i ] );

    if ( ( citype == "BOOLEAN" ) || ( citype == "BOOL" ) ) {
        string bVal;
        strBuffer >> bVal;
        bool   bValue = ( bVal == "true" ) ? true : false;
        bRet = setAttributeValue( name, bValue );
    }
    else 
    if ( citype == "INTEGER" ) {
        int iValue;
        strBuffer >> iValue;
        bRet = setAttributeValue( name, iValue );
    }
    else 
    if ( citype == "FLOAT" ) {
        float fValue;
        strBuffer >> fValue;
        bRet = setAttributeValue( name, fValue );
    }
    else 
    if ( citype == "VECTOR3" ) {
        osg::Vec3f vecValue;
        strBuffer >> vecValue._v[0] >> vecValue._v[1] >> vecValue._v[2];
        bRet = setAttributeValue( name, vecValue );
    }
    else 
    if ( citype == "STRING" ) {
        bRet = setAttributeValue( name, value );
    }

    return bRet;
}

bool AttributeManager::setAttributeValue( const string& name, const EntityAttributeBase& attribute )
{
    unsigned int type = const_cast< EntityAttributeBase& >( attribute ).getType();
    switch ( type ) 
    {
        case EntityAttributeType::FLOAT:
        {
            float value = ( ( EntityAttribute< float >* )&attribute )->getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::BOOL:
        {
            bool value = ( ( EntityAttribute< bool >* )&attribute )->getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::INTEGER:
        {
            int value = ( ( EntityAttribute< int >* )&attribute )->getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::VECTOR3:
        {
            osg::Vec3f value = ( ( EntityAttribute< osg::Vec3f >* )&attribute )->getValue();
            return setAttributeValue( name, value );
        }
        break;

        case EntityAttributeType::STRING:
        {
            string value = ( ( EntityAttribute< string >* )&attribute )->getValue();
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
    std::vector< EntityAttributeBase* >::iterator pp_attr = _attributes.begin(), pp_end = _attributes.end();
    while ( pp_attr != pp_end ) {
        delete *pp_attr;
        pp_attr++;
    }
    _attributes.clear();
}

AttributeManager& AttributeManager::operator = ( const AttributeManager& attr )
{
    // copy attribute values
    std::vector< EntityAttributeBase* >::iterator p_beg = const_cast< AttributeManager& >( attr ).getAttributes().begin(), p_end = const_cast< AttributeManager& >( attr ).getAttributes().end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        setAttributeValue( ( *p_beg )->getName(), **p_beg );
    }

    return *this;
}

} // namespace CTD
