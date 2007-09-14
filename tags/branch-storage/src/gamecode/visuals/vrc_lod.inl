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
 # lod related classes
 #
 #   date of creation:  02/15/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_LOD_H_
    #error "do not include this file directly, include vrc_lod.h instead!"
#endif


inline GLenum LODSettings::getMode() const
{
    return _mode;
}

inline GLenum LODSettings::getAdaptationMode() const
{
    return _adaptMode;
}

inline GLenum LODSettings::getErrorMode() const
{
    return _errorMode;
}

inline GLfloat LODSettings::getErrorThreshold() const
{
    return _errorThreshold;
}

inline GLuint LODSettings::getMaxTriangles() const
{
    return _maxTriangles;
}

inline const LODSettings* GLODMesh::getLODSettings() const
{
    return _p_lodSettings.get();
}

inline const GLuint GLODMesh::getGroupName() const
{
    return _lodGroupName;
}

inline const GLuint GLODMesh::getObjectName() const
{
    return _lodObjectName;
}

inline const GLuint GLODMesh::getNumPatches() const
{
    return _lodPatchName;
}
inline const bool GLODMesh::adaptLOD() const
{
    return _adaptLOD;
}
 
inline void GLODMesh::setAdaptLOD( bool adapt )
{
    _adaptLOD = adapt;
}

inline void LODVisitor::setLODSettings( LODSettings* p_lodsettings )
{
    _p_lodSettings = p_lodsettings;
}

inline const LODSettings* LODVisitor::getLODSettings() const
{
    return _p_lodSettings.get();
}

