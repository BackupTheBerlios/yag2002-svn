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
 # neoengine (0.06.2), sound system specific defines
 #
 #   date of creation:  12/16/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/16/2003 boto       creation of messaging defines
 #
 ################################################################*/


#ifndef _CTD_SOUND_DEFS_H_
#define _CTD_SOUND_DEFS_H_


// plugin name
#define CTD_PLUGINNAME_SOUND				"Plugin -- Sound"


// defines for controling a sound entity via messaging
#define CTD_ENTITY_SOUND_MSG_START_SOUND	0xEF000000
#define CTD_ENTITY_SOUND_MSG_STOP_SOUND		0xEF000010
#define CTD_ENTITY_SOUND_MSG_CONT_SOUND		0xEF000020
#define CTD_ENTITY_SOUND_MSG_SET_VOLUME		0xEF000025
#define CTD_ENTITY_SOUND_MSG_MUTE_SOUND		0xEF000030
#define CTD_ENTITY_SOUND_MSG_DEMUTE_SOUND	0xEF000040
#define CTD_ENTITY_SOUND_MSG_FADE_IN		0xEF000050
#define CTD_ENTITY_SOUND_MSG_FADE_OUT		0xEF000060
#define CTD_ENTITY_SOUND_MSG_SET_LOOPING	0xEF000070
#define CTD_ENTITY_SOUND_MSG_SET_POSITION	0xEF000080


#endif //_CTD_SOUND_DEFS_H_
