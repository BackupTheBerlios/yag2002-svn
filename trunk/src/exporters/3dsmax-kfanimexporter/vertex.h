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
 # 3dsmax exporter for keyframe animated meshes
 #
 # this code is basing on nsce exporter of Reality Rift Studios
 #  ( mattias@realityrift.com )
 #
 #
 #   date of creation:  02/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

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