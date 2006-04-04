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
 # console command for dumping entity information such as type
 #
 #  and parameters
 #
 #   date of creation:  08/15/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdentitydump.h"
#include "vrc_console.h"

namespace vrc
{
// maximal character spacing between parameter name and type
#define MAX_TYPE_SPACING  30


//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdEntityDump );


CmdEntityDump::CmdEntityDump() :
 BaseConsoleCommand( CMD_NAME_ENTITYDUMP )
{
    setUsage( CMD_USAGE_ENTITYDUMP );
}

CmdEntityDump::~CmdEntityDump()
{
}

const std::string& CmdEntityDump::execute( const std::vector< std::string >& arguments )
{
    bool        dumpall = false;
    std::string dumpfile = yaf3d::Application::get()->getMediaPath();
    std::string dumpcontent;
    
    _cmdResult = "";
    if ( arguments.size() < 2 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
 
    if ( arguments[ 0 ] == "-all" )
    {
        dumpall = true;
    }

    dumpfile += arguments[ 1 ];

    dumpcontent = "<!-- entity dump file containing entity type and parameter information -->\n";
    std::string timestamp = yaf3d::getTimeStamp();
    timestamp.erase( timestamp.size() - 1 ); // remove line-feed at end of timestamp string
    dumpcontent += "<!-- created by vrc console: " + timestamp + " -->\n\n";

    // we get the parameters out of entities after we have constructed them. 
    // we shutdown the application if the option -all is used. the shutdown is necessary as some entities are meant to be singletons or perform 
    //  particular tasks in their constructor which may lead to instability or crashes when we create a new instance of them in a brute-force 
    //  manner in order to get their parameters.

    // note: getting the parameters of already exisiting entities makes no problems so no shutdown is needed ( i.e. no use of -all option ).

    if ( !dumpall )
    {
        yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->findInstance( arguments[ 0 ] );
        if ( !p_entity )
        {
            _cmdResult = " entity '" + arguments[ 0 ] + "' cannot be found\n";
            _cmdResult += getUsage();
            return _cmdResult;
        }

        _cmdResult = "dumping entity '" +  arguments[ 0 ] + "' to file: " + dumpfile;
        
        yaf3d::BaseEntityFactory* p_factory = yaf3d::EntityManager::get()->getEntityFactory( p_entity->getTypeName() );
        dumpcontent += dumpEntity( p_entity, p_factory->getCreationPolicy() );
    }
    else
    {
        std::vector< yaf3d::BaseEntityFactory* > factories;
        yaf3d::EntityManager::get()->getAllEntityFactories( factories );
        
        std::stringstream totcount;
        totcount << factories.size();
        dumpcontent += "<!-- total count of available entities: " + totcount.str() + " -->\n\n";
        
        std::vector< yaf3d::BaseEntityFactory* >::iterator p_beg = factories.begin(), p_end = factories.end();

        for ( ; p_beg != p_end; ++p_beg )
        {
            yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->createEntity( ( *p_beg )->getType() );
            dumpcontent += dumpEntity( p_entity, ( *p_beg )->getCreationPolicy() );
            dumpcontent += "\n";
            yaf3d::EntityManager::get()->deleteEntity( p_entity );
        }

        // shutdown application now
        yaf3d::Application::get()->stop();
    }

    // write out the dump file
    std::ofstream of;
    of.open( dumpfile.c_str(), std::ios::out | std::ios::binary );
    if ( !of )
    {
        _cmdResult += " cannot open file '" +  dumpfile + "' for writing the dump.";
        return _cmdResult;
    }

    of << dumpcontent;
    of.close();

    return _cmdResult;
}

std::string CmdEntityDump::dumpEntity( yaf3d::BaseEntity* p_entity, unsigned int creationPolicy )
{
    std::string dump;

    dump = "<Entity Type=\"" + p_entity->getTypeName() + "\" CreationPolicy=\"";

    std::string cpolicy;
    if ( creationPolicy & yaf3d::BaseEntityFactory::Standalone )
        cpolicy += " Standalone ";
    if ( creationPolicy & yaf3d::BaseEntityFactory::Server )
        cpolicy += " Server ";
    if ( creationPolicy & yaf3d::BaseEntityFactory::Client )
        cpolicy += " Client ";

    dump += cpolicy + "\" />\n\n";

    yaf3d::AttributeManager& attrmgr = p_entity->getAttributeManager();
    std::vector< yaf3d::BaseEntityAttribute* >& attributes = attrmgr.getAttributes();
    std::vector< yaf3d::BaseEntityAttribute* >::iterator p_beg = attributes.begin(), p_end = attributes.end();

    for ( ; p_beg != p_end; ++p_beg )
    {
        unsigned int type = ( *p_beg )->getType();
        std::string  stype;
        switch ( type ) 
        {
            case yaf3d::EntityAttributeType::FLOAT:
            {
                stype = "float\"  ";
            }
            break;

            case yaf3d::EntityAttributeType::BOOL:
            {
                stype = "bool\"   ";
            }
            break;

            case yaf3d::EntityAttributeType::INTEGER:
            {
                stype = "integer\"";
            }
            break;

            case yaf3d::EntityAttributeType::VECTOR2:
            {
                stype = "vector2\"";
            }
            break;

            case yaf3d::EntityAttributeType::VECTOR3:
            {
                stype = "vector3\"";
            }
            break;

            case yaf3d::EntityAttributeType::STRING:
            {
                stype = "string\" ";
            }
            break;

            default:
                assert( NULL && "invalid attribute type!" );
        }

        dump += "    <Parameter Name=\"" + ( *p_beg )->getName() + "\" ";
        for ( unsigned int i = ( *p_beg )->getName().length(); i < MAX_TYPE_SPACING; ++i )
            dump += " ";

        dump += "Type=\"" + stype + " />\n";
    }

    dump +="\n</Entity>\n\n";

    return dump;
}

} // namespace vrc
