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
 # neoengine, visuals plugin specific defines
 #
 #   date of creation:  01/04/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/04/2004 boto       creation of this header file
 #
 ################################################################*/


#ifndef _CTD_VISUALS_DEFS_H_
#define _CTD_VISUALS_DEFS_H_


namespace CTD_IPluginVisuals
{

// plugin name
#define CTD_PLUGINNAME_VISUALS "Plugin -- Visuals"

// versioning constants
#define CTD_PLUGIN_VISUALS_VERSION_MAJ			0
#define CTD_PLUGIN_VISUALS_VERSION_MIN			4
#define CTD_PLUGIN_VISUALS_VERSION_REVISION		0

// defines for controling a sprite entity via messaging
//---------------------------------------------------//
//  create a new copy of sprite node, it is used e.g. by pointlight entity
#define CTD_ENTITY_SPRITE_MSG_CREATE	0xAF000000
//  set sprite color
#define CTD_ENTITY_SPRITE_MSG_SET_COLOR	0xAF000002
//---------------------------------------------------//

} // namespace CTD_IPluginVisuals

#endif //_CTD_VISUALS_DEFS_H_
