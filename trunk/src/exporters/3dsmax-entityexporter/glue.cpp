/***************************************************************************
                        glue.cpp  -  3DSMax entry points
                             -------------------
    begin                : Thu Jun 5 2003
    copyright            : (C) 2003 by Reality Rift Studios
    email                : mattias@realityrift.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * This software is provided 'as-is', without any express or implied       *
 * warranty. In no event will the authors be held liable for any damages   *
 * arising from the use of this software.                                  *
 *                                                                         *
 * Permission is granted to anyone to use this software for any purpose,   *
 * including commercial applications, and to alter it and redistribute     *
 * it freely, subject to the following restrictions:                       *
 *                                                                         *
 *   1. The origin of this software must not be misrepresented; you must   *
 *      not claim that you wrote the original software. If you use this    *
 *      software in a product, an acknowledgment in the product            *
 *      documentation would be appreciated but is not required.            *
 *                                                                         *
 *   2. Altered source versions must be plainly marked as such, and must   *
 *      not be misrepresented as being the original software.              *
 *                                                                         *
 *   3. This notice may not be removed or altered from any source          *
 *      distribution.                                                      *
 *                                                                         *
 ***************************************************************************/

#include "exporter.h"


extern "C"
{


BOOL WINAPI DllMain( HINSTANCE hInstDLL, ULONG ulReason, LPVOID pReserved )
{
	NeoMaxExporter::g_hInstance = hInstDLL;
	return( TRUE );
}


__declspec( dllexport ) const TCHAR *LibDescription()
{
	return "Export to NeoEngine Scene File";
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
			return NeoMaxExporter::GetExporterDesc();

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

