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
 #      this code is basing on Delta3D's implementation of skybox
 #                  ( http://www.delta3d.org/ )
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
#include <ctd_utils.h>

namespace CTD
{

#define ENTITY_NAME_SKYBOX    "SkyBox"

class EnSkyBox :  public BaseEntity
{
    public:
                                                    EnSkyBox();

        virtual                                     ~EnSkyBox();


        //! This entity does not need a transform node, which would be created by level manager on loading
        //!   We create an own one and add it into scene's root node
        bool                                        needTransformation() { return false; }

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
          
        EyeTransform*                               _p_transformEyePoint;
           
        osg::ref_ptr< osg::Group >                  _node;
           
        osg::ref_ptr< osg::Geode >                  _geode;

        osg::Node*                                  makeBox();

        std::map< short, std::string >              _textureFilenameMap;
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
