/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
 *  http://yag2002.sourceforge.net
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
 # 3DS Max -- ASE importer
 #
 # this is an ASE importer for 3d studio max 5.1
 # it supports up to two texture and uv channels
 #
 #
 #   date of creation:  03/01/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "ctd_ASEimporter.h"


extern "C"
{


BOOL WINAPI DllMain( HINSTANCE hInstDLL, ULONG ulReason, LPVOID pReserved )
{
	g_hInstance = hInstDLL;
	return( TRUE );
}


__declspec( dllexport ) const TCHAR *LibDescription()
{
	return "Import ASE considering lightmaps";
}


__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}


__declspec( dllexport ) ClassDesc *LibClassDesc( int i )
{
	switch( i )
	{
		case 0:
			return GetImporterDesc();

		default:
			return 0;
	}
}


__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}


// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}



}

