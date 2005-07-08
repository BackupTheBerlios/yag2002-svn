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
 # classes needed for various light types
 #
 #   date of creation:  04/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_LIGHTMANAGER_H_
#define _CTD_LIGHTMANAGER_H_

#include <ctd_main.h>

namespace CTD
{

//! Maximal lights enabled during one frame
#define CTD_MAX_GL_LIGHTS       8

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

        osg::ref_ptr< osg::LightSource >            _lightSource;

        GLint                                       _lightId;

        osg::BoundingSphere                         _bSphere;

    protected:

        // Entity parameters
        osg::Vec3f                                  _position;

        std::string                                 _meshFile;

        osg::Vec3f                                  _ambientColor;

        osg::Vec3f                                  _diffuseColor;
        
        osg::Vec3f                                  _specularColor;

        float                                       _constAttenuation;

        float                                       _linearAttenuation;

        float                                       _quadraticAttenuation;
};

class LightCallback;
//! Class for enabling up to CTD_MAX_GL_LIGHTS gl lights per frame considering light source culling
class LightManager : public Singleton< LightManager >, public Producer::Camera::Callback
{
    public:

       inline void                                  operator()( const Producer::Camera & ) 
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

    friend class Singleton< LightManager >;
    friend class LightCallback;
};

//! Culling callback class for auto-activating light per frame using the light manager
class LightCallback : public osg::NodeCallback
{

    public:

        explicit                                    LightCallback( BaseLight* p_lightentity ) : 
                                                    _lightEntity( p_lightentity )  
                                                    {}

        virtual                                     ~LightCallback() {}

        void                                        operator()( osg::Node* node, osg::NodeVisitor* nv );

        BaseLight*                                  _lightEntity;
};

}

#endif // _CTD_LIGHTMANAGER_H_
