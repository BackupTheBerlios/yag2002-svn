/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 # mesh entity
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_mesh.h"

namespace CTD
{

//! Implement and register the mesh entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( MeshEntityFactory );

EnMesh::EnMesh() :
_enable( true )
{
    // register entity attributes
    _attributeManager.addAttribute( "meshFile"    , _meshFile    );
    _attributeManager.addAttribute( "position"    , _position    );
    _attributeManager.addAttribute( "rotation"    , _rotation    );
    _attributeManager.addAttribute( "enable"      , _enable      );
}

EnMesh::~EnMesh()
{
}

void EnMesh::initialize()
{
    osg::Node* p_node = LevelManager::get()->loadMesh( _meshFile );
    if ( !p_node )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return;
    }

    _mesh = p_node;
    addToTransformationNode( _mesh.get() );
    setPosition( _position );
    osg::Quat   rot( 
                     _rotation.x(), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     _rotation.y(), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     _rotation.z(), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );
    setRotation( rot );
}

} // namespace CTD
