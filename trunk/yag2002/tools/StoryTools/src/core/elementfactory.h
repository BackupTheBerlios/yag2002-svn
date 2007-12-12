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
 # factory for all kind of elements such as event, operation, etc.
 #
 #   date of creation:  11/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _ELEMENTFACTORY_H_
#define _ELEMENTFACTORY_H_

#include <main.h>
#include <core/node.h>
#include <core/elementtypes.h>

namespace beditor
{

//! Class for element factory related exceptions
class ElementFactoryException : public std::exception
{
    public:
                                                        ElementFactoryException( const std::string& reason ) :
                                                         std::exception( reason.c_str() )
                                                        {
                                                        }

        virtual                                         ~ElementFactoryException() throw() {}

                                                        ElementFactoryException( const ElementFactoryException& e ) :
                                                         std::exception( e )
                                                        {
                                                        }

    protected:

                                                        ElementFactoryException();

        ElementFactoryException&                        operator = ( const ElementFactoryException& );
};

//! Forward declaration
class BaseElementRegistry;

/**
  Factory for creating elements of registered types.
  Element types are registered/deregistered using BaseElementRegistry objects.
  The factory does not delete the registry objects on shutdown! Use the macro ELEMENT_REGISTRY_IMPL below for
  element type registry.
*/
class ElementFactory : public Singleton< ElementFactory >
{
    public:

        //! Create a node given its type, throws ElementFactoryException when type not registered.
        BaseNodePtr                                     createNode( const std::string& type ) throw( ... );

    protected:

                                                        ElementFactory();

        virtual                                         ~ElementFactory();

        //! Initialize the singleton, throws ElementFactoryException when something goes wrong.
        void                                            initialize() throw ( ... );

        //! Shut down the singleton
        void                                            shutdown();

        //! Register given element type, used by element registry objects
        void                                            registerType( BaseElementRegistry* p_registry );

        //! Deregister given element type, used by element registry objects
        void                                            deregisterType( BaseElementRegistry* p_registry );

        //! Registered elements
        std::map< std::string, BaseElementRegistry* >   _stock;

    friend class Core;
    friend class BaseElementRegistry;
    friend class Singleton< ElementFactory >;
};

//! Every element must implement its registry using this base class.
class BaseElementRegistry
{
    public:

        //! Get the registry type
        const std::string                               getType() const { return _type; }

    protected:

        explicit                                        BaseElementRegistry( const std::string& type );

        virtual                                         ~BaseElementRegistry();

        //! Create node
        virtual BaseNodePtr                             createNode() = 0;

        //! Registry type
        std::string                                     _type;

    friend class ElementFactory;
};


/**
 Convenient macro which can be used for derived registry classes.
 Place this macro in an .h file.
 Example:

    ELEMENT_REGISTRY_CLASS( MyElementRegistry, MyElementNode )
*/
#define ELEMENT_REGISTRY_CLASS( regclass, node ) \
    class regclass : public BaseElementRegistry \
    {\
        public: \
            explicit regclass( const std::string& type ) : BaseElementRegistry( type ) {} \
            virtual  ~regclass() {} \
        protected: \
            virtual BaseNodePtr createNode() { return BaseNodePtr( new node ); } \
    }; \


/**
 Macro for implementing a registry type. The type itself is stored as string.
 Place this macro in a .cpp file.
 Example:

    ELEMENT_REGISTRY_IMPL( MyElementRegistry, "MyElement" )
*/
#define ELEMENT_REGISTRY_IMPL( regclass, type )       std::auto_ptr< regclass > ##regclass_registry( new regclass( type ) );

} // namespace beditor

#endif // _ELEMENTFACTORY_H_
