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
 # neoengine, entity descriptor class
 #
 # this class implements an abstract descriptor for all kinds of 
 #  plugin entities
 #
 #
 #   date of creation:  12/01/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/01/2003 boto       creation of EntityDescriptor
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/


#ifndef _CTD_DESCRIPTORS_H_
#define _CTD_DESCRIPTORS_H_

#include <string>


namespace CTD 
{

// type strings for parameters, they are used in level file (xml)
#define CTD_PD_TYPE_INTEGER         "Integer"
#define CTD_PD_TYPE_STRING          "String"
#define CTD_PD_TYPE_FLOAT           "Float"
#define CTD_PD_TYPE_BOOL            "Bool"
#define CTD_PD_TYPE_VECTOR3         "Vector3"
#define CTD_PD_TYPE_STATIC_MESH     "StaticMesh"

// macro for thread-safe entity name retrieval used in entity descriptor
#define CTD_RETURN_ENTITYNAME( EntityName )     static std::string strEntityName( EntityName ); return strEntityName;

class BaseEntity;

// parameter descriptor
class ParameterDescriptor
{
    public:

                        ParameterDescriptor() { m_eType = CTD_PD_UNKNOWN; m_pVar = 0; }
                        
                       ~ParameterDescriptor() { }

        /**
        * Parameter type
        */
        enum m_ePdType { 

            CTD_PD_UNKNOWN,
            CTD_PD_STRING,
            CTD_PD_INTEGER, 
            CTD_PD_FLOAT, 
            CTD_PD_BOOL, 
            CTD_PD_VECTOR3,
        
            CTD_PD_STATIC_MESH 
        };

        /**
        * Set function for parameter name.
        */
        void            SetName( std::string name ) { m_strName = name; }
        /**
        * Get function for parameter name.
        */
        std::string     GetName() { return m_strName; }

        /**
        * Set function for parameter type.
        */
        void            SetType( m_ePdType type ) { m_eType = type; }
        /**
        * Get function for parameter type.
        */
        m_ePdType       GetType() { return m_eType; }

        /**
        * Set function for parameter variable. Pass a void pointer of parameter. The callee function has to reserve memory!
        */
        void            SetVar( void *var ) { m_pVar = var; }

        /**
        * Get function for parameter variable. The content of this pointer mus be casted to appropriate type
        *  depending on paramerter type.
        */
        void*           GetVar() { return m_pVar; }

    private:

        std::string     m_strName;
        m_ePdType       m_eType;
        void            *m_pVar;

};

// base simulation entity, derive from this class for your entity description
class  EntityDescriptor
{

    public:

                                        EntityDescriptor() {};

        virtual                        ~EntityDescriptor() {};

        /**
        * This function must return the entity name which is used to 
        *  define the entity type. This name should be uniqe for every kind of entity.
        * Attention: In order to be thread-safe use the following body, or the Macro: CTD_RETURN_ENTITYNAME( "EntityName" )
        * GetEntityName() { static std::string strEntityName( "EntityName" ); return strEntityName; }
        */
        virtual const std::string&      GetEntityName() = 0;

        /**
        * This function is called by framework uppon level initialization and creates
        *  an entity object.
        */
        virtual BaseEntity*             CreateEntityInstance() = 0;


};

} // namespace CTD

#endif //_CTD_DESCRIPTORS_H_
