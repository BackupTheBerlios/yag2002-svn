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


#ifndef _CTDFRAMWORK_BASE_H_
#define _CTDFRAMWORK_BASE_H_

#include <neoengine/base.h>
#include <neoengine/config.h>
#include <neoengine/core.h>
#include <neoengine/render.h>
#include <neoengine/shadow.h>
#include <neoengine/filemanager.h>
#include <neoengine/logstream.h>
#include <neoengine/file.h>
#include <neoengine/input.h>
#include <neoengine/inputentity.h>
#include <neoengine/room.h>
#include <neoengine/loadableentity.h>
#include <neoengine/console.h>
#include <neoengine/camera.h>
#include <neoengine/timer.h>
#include <neoengine/profile.h>
#include <neoengine/submesh.h>
#include <neoengine/aabb.h>
#include <neoengine/collision.h>
#include <neoengine/strutil.h>
#include <neoengine/audio.h>
#include <neoengine/sprite.h>
#include <neolua/script.h>
#include <neochunkio/scene.h>
#include <neochunkio/mesh.h>
#include <neochunkio/core.h>
#include <neodevopengl/link.h>
#include <neodevdsound/link.h>
#include <neoacwav/link.h>
#include <neoacvorbis/link.h>
#include <neofcbzip2/link.h>
#include <neoicpng/link.h>
#include <neoictga/link.h>
#include <neoicjpeg/link.h>
#include <neoicbmp/link.h>
#include <neoabt/link.h>
#include <neobsp/link.h>

#ifdef WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

#include <string>
#include <vector>

#endif // _CTDFRAMWORK_BASE_H_
