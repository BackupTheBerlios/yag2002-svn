/***************************************************************************
                          vertex.h  -  Vertex data
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

#ifndef __NEOENGINE_3DSMAX_VERTEX_H
#define __NEOENGINE_3DSMAX_VERTEX_H


#include "exporter.h"
#include "texcoord.h"

#include <neoengine/skin.h>


namespace NeoMaxExporter
{


class MaxVertex
{
	public:
		
		NeoEngine::Vector3d                           m_kCoord;

		NeoEngine::Vector3d                           m_kNormal;

		MaxTexCoord                                   m_kUV;
		bool                                          m_bUV;

		MaxTexCoord                                   m_kLmUV;
		bool                                          m_bLmUV;

		NeoEngine::SkinVertex                        *m_pkSkinVertex;

		                                              MaxVertex() : m_bUV( false ), m_bLmUV( false ), m_pkSkinVertex( 0 ) {}

		virtual                                      ~MaxVertex() { delete m_pkSkinVertex; }

		bool                                          operator == ( const MaxVertex &rkVertex ) { return( ( m_kCoord == rkVertex.m_kCoord ) && ( m_kNormal == rkVertex.m_kNormal ) && ( m_kUV == rkVertex.m_kUV ) ); }
};


};


#endif
