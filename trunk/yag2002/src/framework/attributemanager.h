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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/13/2005 boto       creation of AttributeManager
 #
 ################################################################*/


#ifndef _ATTRIBUTEMANAGER_H_
#define _ATTRIBUTEMANAGER_H_

#include <base.h>
#include <log.h>

namespace yaf3d 
{

//! Attribute types for entities
struct EntityAttributeType
{
        //! Attribute type
        enum Type {
            UNKNOWN = 0,
            FLOAT,
            BOOL,
            INTEGER,
            VECTOR2,
            VECTOR3,
            STRING
        };

};

//! Base of attribute
class EntityAttributeBase
{

    public:

                                                    EntityAttributeBase() {}
                                                    
        virtual                                     ~EntityAttributeBase() {}

        /**
        * Get attribute name
        * \return                                   Attribute name
        */
        virtual const std::string&                  getName() const = 0;

        /**
        * Get attribute type
        * \retrun                                   Attribute type
        */
        virtual unsigned int                        getType() const = 0;

        /**
        * Get type info of this attribute
        * \return                                   Type info
        */
        virtual const std::type_info&               getTypeInfo() const = 0;
 
};

//! One single attribute holding name and a reference to an existing variable
template< class TypeT >
class EntityAttribute : public EntityAttributeBase
{


    public:
    
        /**
        * Construct an attribute
        * \param strName                            Attribute name
        * \param ref                                Reference to an existing variable
        */
                                                    EntityAttribute( const std::string& name, TypeT &ref );

        virtual                                     ~EntityAttribute() {}

        /**
        * Get attribute value
        * \return                                   Attribute reference
        */
        TypeT                                       getValue() const;

        /**
        * Get attribute value
        * \return                                   Attribute reference
        */
        void                                        setValue( TypeT value );

        /**
        * Get attribute name
        * \return                                   Attribute name
        */
        const std::string&                          getName() const;

        /**
        * Get attribute type
        * \retrun                                   Attribute type
        */
        unsigned int                                getType() const;

        /**
        * Get type info of this attribute
        * \return                                   Type info
        */
        const std::type_info&                       getTypeInfo() const;

    protected:

        //! Reference to entity's attribute
        TypeT&                                      _reference;

        //! Attribute name
        std::string                                 _name;

        //! Attribute type
        unsigned int                                _type;

    private:

        EntityAttribute&                            operator = ( const EntityAttribute& );

                                                    EntityAttribute( const EntityAttribute& );
};

//! Attribute manager
class AttributeManager
{

    public:

                                                    AttributeManager() {} 

        virtual                                     ~AttributeManager() {}


        /**
        * Add an attribute
        * \param name                               Attribute name
        * \param reference                          Reference to the member variable in entity
        */
        template< class TypeT >
        void                                        addAttribute( const std::string &name, TypeT &reference );

        /**
        * Get attribute list
        * \return                                   List of attributes
        */
        std::vector< EntityAttributeBase* >&        getAttributes();

        /**
        * Get all attribute names and values as string.
        * \param attributes                         The result will be stored in this list.
        */
        void                                        getAttributesAsString( std::vector< std::pair< std::string, std::string > >& attributes );

        /**
        * Get attribute value given its name. This method does a type check.
        * \param name                               Attribute name
        * \param value                              Requested Value
        * \return                                   true if the value has been found, otherwise false
        */
        template< class TypeT >
        bool                                        getAttributeValue( const std::string& name, TypeT& value );

        /**
        * Set attribute value. This method does a type check.
        * \param name                               Attribute name
        * \param value                              New Value to be set
        * \return                                   true if the value has been found, otherwise false
        */
        template< class TypeT >
        bool                                        setAttributeValue( const std::string& name, TypeT value );

        /**
        * Set attribute value given an existing attribute.
        * \param name                               Attribute name
        * \param attribute                          The value of this Attribute is set to named local attribute.
        * \return                                   true if the value has been found, otherwise false
        */
        bool                                        setAttributeValue( const std::string& name, const EntityAttributeBase& attribute );

        /**
        * Set attribute value given its name, type ( see EntityAttributeType ) and value, all as strings
        * \param name                               Attribute name
        * \param type                               Attribute type
        * \param value                              Attribute value
        * \return                                   false if attribute could not be found
        */
        bool                                        setAttributeValue( const std::string &name, const std::string &type, const std::string &value );

        /**
        * Set attribute value using a string. This method does only a format check, not type check ( e.g. the value of an attribute with type boolean can be only "true" or "false" ).
        * \param name                               Attribute name
        * \param valuestring                        New Value to be set
        * \return                                   true if the value has been found and the value string had a proper format, otherwise false
        */
        bool                                        setAttributeValueByString( const std::string& name, const std::string& valuestring );

        /**
        * Remove all attributes.
        */
        void                                        removeAllAttributes();

        /**
        * Assignment operator.
        * It updates all own attribute values from a given attribute manager.
        * Note: it is assumed that both managers possess the same attributes in name and value!
        */
        AttributeManager&                           operator = ( AttributeManager& attr );

    protected:

        //! Attributes
        std::vector< EntityAttributeBase* >         _attributes;
};

#include "attributemanager.inl"

} // namespace yaf3d

#endif //_ATTRIBUTEMANAGER_H_
