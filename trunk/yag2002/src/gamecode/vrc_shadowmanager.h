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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _SHADOWMANAGER_H_
#define _SHADOWMANAGER_H_

#include <vrc_main.h>

namespace vrc
{

namespace gameutils 
{
    class VRCStateHandler;
}
class CameraCullCallback;
class UpdateCameraAndTexGenCallback;

//! Shadow manager handles shadowers and shadowed geometries and the light source
class ShadowManager : public yaf3d::Singleton< vrc::ShadowManager >
{
    public:

        //! Setup shadow manager.
        /*!
        * shadowTextureWidth  is the width of shadow texture
        * shadowTextureHeight is the height of shadow texture
        * shadowTextureUnit   is the texture unit for the shadow texture.
        */
        void                                        setup( 
                                                           unsigned int shadowTextureWidth  = 1024, 
                                                           unsigned int shadowTextureHeight = 1024, 
                                                           unsigned int shadowTextureUnit   = 1
                                                          );

        //! Enable / disable the shadow management
        void                                        enable( bool en );

        //! Add p_node to the list of shadow throwing / receiving nodes
        void                                        addShadowNode( osg::Node* p_node );

        //! Remove p_node from the list of shadow throwing / receiving nodes
        void                                        removeShadowNode( osg::Node* p_node );

        //! Set light source position. Currently only one single light source can be used.
        void                                        setLightPosition( const osg::Vec3f& position );

        //! Get light position
        inline const osg::Vec3f&                    getLightPosition();

        //! Set shadow's gain and bias ( frag color = tex color * ( bias + shadow color * gain ) )
        void                                        setShadowColorGainAndBias( float gain, float bias );

        //! get shadow's gain and bias 
        inline void                                 getShadowColorGainAndBias( float& gain, float& bias );

    protected:

                                                    ShadowManager();

        virtual                                     ~ShadowManager();

        //! Shutdown the shadow manager
        void                                        shutdown();

        unsigned int                                _shadowTextureWidth;

        unsigned int                                _shadowTextureHeight;
        
        unsigned int                                _shadowTextureUnit;

        bool                                        _enable;

        osg::Vec3f                                  _lightPosition;

        osg::Vec2f                                  _shadowAmbientBias;

        osg::ref_ptr< osg::CameraNode >             _shadowCameraGroup;

        osg::ref_ptr< osg::Group >                  _shadowedGroup;

        osg::ref_ptr< osg::Group >                  _shadowSceneGroup;

        UpdateCameraAndTexGenCallback*              _p_updateCallback;

        CameraCullCallback*                         _p_cullCallback;

        osg::Uniform*                               _p_colorGainAndBiasParam;

    friend class yaf3d::Singleton< vrc::ShadowManager >;
    friend class gameutils::VRCStateHandler;
};

inline const osg::Vec3f& ShadowManager::getLightPosition()
{
    return _lightPosition;
}

inline void ShadowManager::getShadowColorGainAndBias( float& gain, float& bias )
{
    gain = _shadowAmbientBias._v[ 0 ];
    bias = _shadowAmbientBias._v[ 1 ];
}

} // namespace vrc

#endif // _SHADOWMANAGER_H_
