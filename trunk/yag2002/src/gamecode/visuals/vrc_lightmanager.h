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
 # classes needed for various light types
 #
 #   date of creation:  04/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_LIGHTMANAGER_H_
#define _VRC_LIGHTMANAGER_H_

#include <vrc_main.h>

namespace vrc
{

//! Maximal lights enabled during one frame
#define VRC_MAX_GL_LIGHTS       8

class LightManager;

//! Base class for all light types
class BaseLight
{
    public:
                                                    BaseLight() :
                                                     _lightId( 0 ),
                                                     _ambientColor( osg::Vec3f( 0.2f, 0.2f, 0.2f ) ),
                                                     _diffuseColor( osg::Vec3f( 0.7f, 0.7f, 0.7f ) ),
                                                     _specularColor( osg::Vec3f( 0.9f, 0.9f, 0.9f ) ),
                                                     _constAttenuation( 0.01f ),
                                                     _linearAttenuation( 0.001f ),
                                                     _quadraticAttenuation( 0.0001f )
                                                    {}

        virtual                                     ~BaseLight()
                                                    {
                                                        if ( _lightSource.get() )
                                                            _lightSource = NULL;
                                                    }

        const osg::BoundingSphere&                  getBoundingSphere()
                                                    {
                                                        return _bSphere;
                                                    }


        osg::Light*                                 getLight()
                                                    {
                                                        return _lightSource->getLight();
                                                    }

    protected:

        osg::ref_ptr< osg::LightSource >            _lightSource;

        GLint                                       _lightId;

        osg::BoundingSphere                         _bSphere;

        osg::ref_ptr< osg::Node >                   _mesh;

        // Entity parameters
        osg::Vec3f                                  _position;

        std::string                                 _meshFile;

        osg::Vec3f                                  _ambientColor;

        osg::Vec3f                                  _diffuseColor;

        osg::Vec3f                                  _specularColor;

        float                                       _constAttenuation;

        float                                       _linearAttenuation;

        float                                       _quadraticAttenuation;

    friend class LightManager;
};

class LightCallback;
//! Class for enabling up to VRC_MAX_GL_LIGHTS gl lights per frame considering light source culling
class LightManager : public yaf3d::Singleton< LightManager >, public osgSDL::Viewer::DrawCallback
{
    public:

       inline void                                  operator()( const osgSDL::Viewport* /*p_vp*/ )
                                                    {
                                                        // enable lights
                                                        flush();
                                                    }

        //! Initialize the light manager
        void                                        initialize();

    protected:

                                                    LightManager();


        virtual                                     ~LightManager(){}

        //! Add a new light to frame
        void                                        addLight( BaseLight* p_light );

        //! Enable those lights which are not culled during this frame
        void                                        flush();

        //! List of lights which are not culled during one rendering frame
        std::vector< BaseLight* >                   _lights;

        //! Is used for holding an available light id for next light object
        GLint                                       _currId;

        bool                                        _initialized;

    friend class yaf3d::Singleton< LightManager >;
    friend class LightCallback;
};

//! Culling callback class for auto-activating light per frame using the light manager
class LightCallback : public osg::NodeCallback
{

    public:

        explicit                                    LightCallback( BaseLight* p_lightentity ) :
                                                    _p_lightEntity( p_lightentity )
                                                    {}

        virtual                                     ~LightCallback() {}

        void                                        operator()( osg::Node* node, osg::NodeVisitor* nv );

        BaseLight*                                  _p_lightEntity;
};

}

#endif // _VRC_LIGHTMANAGER_H_
