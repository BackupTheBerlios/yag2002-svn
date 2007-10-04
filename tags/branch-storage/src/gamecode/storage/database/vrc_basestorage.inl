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
 # base class for storage types
 #
 #   date of creation:  09/30/2007
 #
 #   author:            mpretz 
 #
 ################################################################*/


#ifndef _VRC_BASESTORAGE_H_
#error "don't include this file directly! include vrc_basestorage.h instead"
#endif

inline const std::string& BaseStorage::getType() const
{
    return _type;
}

inline ConnectionData* BaseStorage::getConnectionData() const
{
    return _p_connectionData;
}

inline void BaseStorage::setConnectionData( const ConnectionData& connData )
{
    if ( _p_connectionData )
        delete _p_connectionData;

    _p_connectionData = new ConnectionData( connData );
}
