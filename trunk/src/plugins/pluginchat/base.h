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
 # neoengine, base header
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
 #  12/01/2003 boto       creation of base.h
 #
 ################################################################*/



#ifndef _CTD_PLUGIN_CHAT_BASE_H_
#define _CTD_PLUGIN_CHAT_BASE_H_


#include <neoengine/base.h>
#include <neoengine/nemath.h>
#include <neoengine/mesh.h>
#include <neoengine/logstream.h>
#include <neoengine/renderentity.h>
#include <neochunkio/scene.h>
#include <neochunkio/mesh.h>
#include <neoengine/sprite.h>
#include <neoengine/sceneentity.h>
#include <neoengine/scenenode.h>
#include <neoengine/core.h>
#include <neoengine/aabb.h>
#include <neoengine/inputentity.h>
#include <neoengine/light.h>

#include <ctd_descriptor.h>
#include <ctd_baseentity.h>
#include <ctd_plugin_defs.h>
#include <ctd_printf.h>

#include <ctd_framework.h>
#include <ctd_network.h>

#include <ctd_chat_defs.h>

#include <string>

namespace CTD_IPluginChat 
{

	// clear movement flags
	extern void ClearMovementFlags();
	// lock movement inputs for chatmember entity
	extern void LockMovementInput( bool bLock );

}
//------------------------------------------------------//

#endif //_CTD_PLUGIN_CHAT_BASE_H_
