/***************************************************************************
                      texcoord.h  -  Texture coordinate data
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

#ifndef __NEOENGINE_3DSMAX_TEXCOORD_H
#define __NEOENGINE_3DSMAX_TEXCOORD_H


#include "exporter.h"

#include <neoengine/nemath.h>


namespace NeoMaxExporter
{


class MaxTexCoord
{
	public:

		float                                         u,v;

		                                              MaxTexCoord() : u( 0.0f ), v( 0.0f ) {}

		bool                                          operator == ( const MaxTexCoord &rkTexCoord ) { return( ( fabsf( u - rkTexCoord.u ) < NeoEngine::EPSILON ) && ( fabsf( v - rkTexCoord.v ) < NeoEngine::EPSILON ) ); }
};


};


#endif
