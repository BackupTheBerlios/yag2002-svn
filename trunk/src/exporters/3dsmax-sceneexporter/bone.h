/***************************************************************************
                          bone.h  -  Bone node data
                             -------------------
    begin                : Thu Jun 5 2003
    copyright            : (C) 2003 by Reality Rift Studios
    email                : mattias@realityrift.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __NEOENGINE_3DSMAX_BONE_H
#define __NEOENGINE_3DSMAX_BONE_H


#include "exporter.h"

#include <IConversionManager.h>

#include <string>


class IGameNode;


namespace NeoMaxExporter
{


class MaxBone
{
	public:

		std::string                                   m_strName;

		int                                           m_iID;

		int                                           m_iParent;

		GMatrix                                       m_kWorldTM;

		GMatrix                                       m_kLocalTM;

		IGameNode                                    *m_pkNode;
};


};


#endif
