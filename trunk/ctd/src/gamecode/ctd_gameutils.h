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
 # common game utilities
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_GAMEUTILS_H_
#define _CTD_GAMEUTILS_H_

#include <ctd_base.h>

namespace CTD
{
namespace gameutils
{

//! Retrieve player configuration file path depending on game settings and given game mode ( Server, Client, Standalone ) and
//! in case of Client the remote flag determines local or remote client.
//! Returns false if something went wrong.
bool getPlayerConfig( unsigned int mode, bool remote, std::string& cfgfile );

} // namespace gameutils
} // namespace CTD

#endif //_CTD_GAMEUTILS_H_
