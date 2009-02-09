/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # serialization functionality
 #
 #   date of creation:  02/03/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "fileio.h"

FileOutputLevel::FileOutputLevel()
{
}

FileOutputLevel::~FileOutputLevel()
{
}

bool FileOutputLevel::write( const std::vector< yaf3d::BaseEntity* >& level, const std::string& filename, const std::string& levelname, bool writehidden )
{
    std::ofstream out( filename.c_str(), std::ios_base::binary | std::ios_base::out );
    if ( !out )
        return false;

    out << "<?xml version=\"1.0\" standalone=no>\r" << std::endl;
    out << "<!-- Yaf3d level configuration file -->\r" << std::endl;
    out << "<!-- Created: " << yaf3d::getTimeStamp() << " -->\r" << std::endl;
    out << "\r" << std::endl;
    out << "<Level Name=\"" << levelname << "\">\r\n\r" << std::endl;

    std::vector< yaf3d::BaseEntity* >::const_iterator p_beg = level.begin(), p_end = level.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        // skip hidden entities if not explicitly forced by 'writehidden'
        if ( !writehidden && ( *p_beg )->getInstanceName().length() && ( ( *p_beg )->getInstanceName()[ 0 ] == '_' ) )
            continue;

        out << writeEntity( *p_beg );
        out << "\r" << std::endl;
    }

    out << "</Level>\r" << std::endl;

    return true;
}

std::string FileOutputLevel::writeEntity( yaf3d::BaseEntity* p_entity )
{
    std::stringstream out;
    std::string ident1( "    " );
    std::string ident2( "        " );

    out << ident1 << "<Entity Type=\"" << p_entity->getTypeName() << "\"  InstanceName=\"" << 
           ( p_entity->getInstanceName().length() ? p_entity->getInstanceName() : "" )  << "\">\r" << std::endl;

    std::vector< std::vector< std::string > > attributes;
    p_entity->getAttributeManager().getAttributesAsString( attributes );
    std::vector< std::vector< std::string > >::iterator p_beg = attributes.begin(), p_end = attributes.end();
    for ( ; p_beg != p_end; ++p_beg )
        out << ident2 << "<Parameter Name=\"" << ( *p_beg )[ 0 ] << "\"  Type=\"" << ( *p_beg )[ 2 ] << "\"  Value=\"" << ( *p_beg )[ 1 ] << "\" />\r" << std::endl;

    out << ident1 << "</Entity>\r" << std::endl;

    return out.str();
}
