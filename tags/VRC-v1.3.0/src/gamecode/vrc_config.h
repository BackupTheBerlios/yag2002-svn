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
 # main game code configuration file
 #
 #   date of creation:  08/04/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 #
 ################################################################*/


#ifndef _VRC_CONFIG_H_
#define _VRC_CONFIG_H_

//! Build version, set this define for publishing releases
#define VRC_BUILD_PUBLISH

//! Set the VRC version string
#ifdef VRC_BUILD_PUBLISH
  #define VRC_VERSION         "1.3.0"
#else
  #define VRC_VERSION         "1.3.0 - Dev"
#endif

#endif // _VRC_CONFIG_H_
