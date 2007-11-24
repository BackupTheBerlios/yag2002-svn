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
 # a manager for handling dynamic shadows
 #
 #   date of creation:  06/27/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _SHADOWMANAGER_H_
#define _SHADOWMANAGER_H_

#include <base.h>
#include <singleton.h>
#include <application.h>

namespace yaf3d
{

class ShadowSceneCullCallback;

//! Shadow manager handles shadowers and shadowed geometries and the light source
class ShadowManager : public yaf3d::Singleton< yaf3d::ShadowManager >
{
    public:

        //! Shadow node mode
        enum ShadowNodeMode
        {
            eThrowShadow    = 0x80000000,
            eReceiveShadow  = 0x40000000
        };

        //! Setup shadow manager.
        /*!
        * shadowTextureWidth  is the width of shadow texture
        * shadowTextureHeight is the height of shadow texture
        * shadowTextureUnit   is the texture unit for the shadow texture.
        */
        void                                        setup( 
                                                           unsigned int shadowTextureWidth  = 1024, 
                                                           unsigned int shadowTextureHeight = 1024, 
                                                           unsigned int shadowTextureUnit   = 1,
                                                           float        shadowAmbient       = 0.2f
                                                          );

        //! Enable / disable the shadow management
        void                                        enable( bool en );

        //! Add p_node to the list of shadow throwing / receiving nodes. shadowmode is a combination of ShadowNodeMode enums and defines if the node should throw / receive shadow or both.
        //! The two most significant bits of p_nodes' nodemask are modified by this method! The culldistance is used for disabling shadow throwing when the camera is too far from the node.
        void                                        addShadowNode( osg::Node* p_node, unsigned int shadowmode, float culldistance = 100.0f );

        //! Remove p_node from the list of shadow throwing / receiving nodes
        void                                        removeShadowNode( osg::Node* p_node );

        //! Call this if you manually modify shadow nodes in scenegraph without calling addShadowNode() or removeShadowNode()
        void                                        updateShadowArea();

        //! Set light source position. Currently only one single light source can be used.
        void                                        setLightPosition( const osg::Vec3f& position );

        //! Get light position
        inline const osg::Vec3f&                    getLightPosition();

        //! Set shadow's gain and bias ( frag color = tex color * ( bias + shadow color * gain ) )
        void                                        setShadowColorGainAndBias( float gain, float bias );

        //! get shadow's gain and bias 
        inline void                                 getShadowColorGainAndBias( float& gain, float& bias );

        //! Enable / disable shadow map display ( used for debugging )
        void                                        displayShadowMap( bool enable );

    protected:

                                                    ShadowManager();

        virtual                                     ~ShadowManager();

        //! Shutdown the shadow manager
        void                                        shutdown();

        //! Create a windget for showing the shadow map texture
        osg::Node*                                  createDebugDisplay( osg::Texture* p_texture );

        unsigned int                                _shadowTextureWidth;

        unsigned int                                _shadowTextureHeight;
        
        unsigned int                                _shadowTextureUnit;

        float                                       _shadowAmbient;

        bool                                        _enable;

        osg::Vec3f                                  _lightPosition;

        osg::Vec2f                                  _shadowAmbientBias;

        osg::ref_ptr< osg::Camera >                 _shadowCameraGroup;

        osg::ref_ptr< osg::Group >                  _shadowedGroup;

        osg::ref_ptr< osg::Group >                  _shadowSceneGroup;

        ShadowSceneCullCallback*                    _p_cullCallback;

        osg::Uniform*                               _p_colorGainAndBiasParam;

        osg::Texture2D*                             _p_shadowMapTexture;

        osg::ref_ptr< osg::Node >                   _debugDisplay;

    friend class yaf3d::Singleton< yaf3d::ShadowManager >;
    friend class yaf3d::Application;
};


// Inline methods

inline const osg::Vec3f& ShadowManager::getLightPosition()
{
    return _lightPosition;
}

inline void ShadowManager::getShadowColorGainAndBias( float& gain, float& bias )
{
    gain = _shadowAmbientBias._v[ 0 ];
    bias = _shadowAmbientBias._v[ 1 ];
}

} // namespace yaf3d

#endif // _SHADOWMANAGER_H_
