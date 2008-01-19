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
 # container for holding shader programs
 #
 #   date of creation:  09/10/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _SHADERCONTAINER_H_
#define _SHADERCONTAINER_H_

#include <base.h>
#include <singleton.h>
#include <application.h>

namespace yaf3d
{

//! Shader code container
//! It delivers cached fragment and vertex shaders which can be composed to more complex shader programs.
class ShaderContainer : public yaf3d::Singleton< yaf3d::ShaderContainer >
{
    public:

        //! GLSL vertex shader types
        enum VertexShaderType
        {
            eCommonV                  = 0x10, // common shader functions to be used in main()
            eShadowMapV,
            eTerrainV,
            eTerrainShadowMapV,
            eVegetationV
        };

        //! GLSL fragment shader types
        enum FragmentShaderType
        {
            eCommonF                  = 0x100, // common shader functions to be used in main()
            eShadowMapF,
            eTerrainF,
            eTerrainShadowMapF,
            eVegetationF
        };

        //! Get the vertex shader for given type ( one of the VertexShaderType enums ).
         osg::Shader*                               getVertexShader( unsigned int type );

        //! Get the fragment shader for given type ( one of the FragmentShaderType enums ).
         osg::Shader*                               getFragmentShader( unsigned int type );

         /** Add a shader node with given name to container. It can be retrieved by using 'getShaderNode' below.
             If a prent node is given then the shader node is appended to it.
             Returns false if a shader with same name already exists.
         */
         bool                                       addShaderNode( const std::string& name, osg::ref_ptr< osg::Group > node, osg::Group* p_parent = NULL );

         /** Remove shader node with given name from container.
             Returns false if no shader node with given name exists.
         */
         bool                                       removeShaderNode( const std::string& name );

         /** Get the shader node with given name.
             Returns NULL if no shader node with given name exists.
         */
         osg::ref_ptr< osg::Group >                 getShaderNode( const std::string& name );

    protected:

                                                    ShaderContainer();

        virtual                                     ~ShaderContainer();

        //! Shutdown the shader container
        void                                        shutdown();

        //! Fragment shader cache
        std::map< unsigned int, osg::ref_ptr< osg::Shader > >      _fsCache;

        //! Vertex shader cache
        std::map< unsigned int, osg::ref_ptr< osg::Shader > >      _vsCache;

        //! Shader nodes < shader name, scenegraph node >
        std::map< std::string, osg::ref_ptr< osg::Group > >        _shaderNodes;

    friend class yaf3d::Singleton< yaf3d::ShaderContainer >;
    friend class yaf3d::Application;
};

} // namespace yaf3d

#endif // _SHADERCONTAINER_H_
