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

#include <main.h>
#include "elementfactory.h"


//! Implement the element factory singleton
BEDITOR_SINGLETON_IMPL( beditor::ElementFactory )


namespace beditor
{

ElementFactory::ElementFactory()
{
}

ElementFactory::~ElementFactory()
{
}

BaseNodePtr ElementFactory::createNode( const std::string& type ) throw( ... )
{
    std::map< std::string, BaseElementRegistry* >::iterator p_factory;
    p_factory = _stock.find( type );
    if ( p_factory != _stock.end() )
    {
        return p_factory->second->createNode();
    }
    else
    {
        log_error << "ElementFactory: no registered type found '" + type + "'" << std::endl;
        throw ElementFactoryException( "ElementFactory: no registered type found '" + type + "'" );
    }

    return BaseNodePtr( NULL );
}

void ElementFactory::registerType( BaseElementRegistry* p_registry )
{
    if ( !p_registry )
    {
        log_error << "ElementFactory: invalid registry object!" << std::endl;
        return;
    }

    std::map< std::string, BaseElementRegistry* >::iterator p_factory;
    p_factory = _stock.find( p_registry->getType() );
    if ( p_factory != _stock.end() )
    {
        log_error << "ElementFactory: trying to register an existing type '" + p_registry->getType() + "'" << std::endl;
        return;
    }

    _stock[ p_registry->getType() ] = p_registry;
}

void ElementFactory::deregisterType( BaseElementRegistry* p_registry )
{
    if ( !p_registry )
        return;

    std::map< std::string, BaseElementRegistry* >::iterator p_factory;
    p_factory = _stock.find( p_registry->getType() );
    if ( p_factory == _stock.end() )
        return;

    _stock.erase( p_factory );
}

void ElementFactory::initialize() throw ( ... )
{
    // currently nothing to do
}

void ElementFactory::shutdown()
{
    // currently nothing to do
}

//! Implementation of element registry
BaseElementRegistry::BaseElementRegistry( const std::string& type ) :
 _type( type )
{
    ElementFactory::get()->registerType( this );
}

BaseElementRegistry::~BaseElementRegistry()
{
    ElementFactory::get()->deregisterType( this );
}

} // namespace beditor
