/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
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
 # entity skybox
 #      this code is basing on Delta3d's implementation of skybox
 #
 #   date of creation:  03/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_SKYBOX_H_
#define _CTD_SKYBOX_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_SKYBOX    "SkyBox"

class EnSkyBox :  public BaseEntity
{
    public:
                                                    EnSkyBox();

        virtual                                     ~EnSkyBox();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        typedef enum   {
            SKYBOX_FRONT = 0,
            SKYBOX_RIGHT,
            SKYBOX_BACK,      
            SKYBOX_LEFT,
            SKYBOX_TOP,
            SKYBOX_BOTTOM
        }                                           _side;

    protected:

        //! Entity parameter for texnames (6)
        std::string                                 _texNames[ 6 ];

    private:

        class EyeTransform : public osg::Transform
        {
            public:

            /** Get the transformation matrix which moves from local coords to world coords.*/
            virtual bool                            computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const 
            {
                osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
                if ( cv )
                {
                    osg::Vec3 eyePointLocal = cv->getEyeLocal();
                    matrix.preMult( osg::Matrix::translate( eyePointLocal.x(), eyePointLocal.y(), eyePointLocal.z() ) );
                    // matrix.preMult(osg::Matrix::rotate(osg::DegreesToRadians(-mAzimuth-90.f), 0.f, 0.f, 1.f));
                }
                return true;
            }

            /** Get the transformation matrix which moves from world coords to local coords.*/
            virtual bool                            computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
            {    
                osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
                if ( cv )
                {
                    osg::Vec3 eyePointLocal = cv->getEyeLocal();
                    matrix.postMult( osg::Matrix::translate( -eyePointLocal.x(), -eyePointLocal.y(), -eyePointLocal.z() ) );
                }
                return true;
            }
        };
           
        EyeTransform*                                _xform;
           
        osg::ref_ptr< osg::Group >                   _node;
           
        osg::ref_ptr< osg::Geode >                   _geode;

        osg::Node*                                   makeBox();

        std::map< short, std::string >               _textureFilenameMap;
};

//! Entity type definition used for type registry
class SkyBoxEntityFactory : public BaseEntityFactory
{
    public:
                                                    SkyBoxEntityFactory() : BaseEntityFactory( ENTITY_NAME_SKYBOX ) {}

        virtual                                     ~SkyBoxEntityFactory() {}

        Macro_CreateEntity( EnSkyBox );
};

}

#endif // _CTD_SKYBOX_H_
