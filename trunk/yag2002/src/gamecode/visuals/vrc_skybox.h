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
 # entity skybox
 #
 #   date of creation:  03/24/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_SKYBOX_H_
#define _VRC_SKYBOX_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>

namespace vrc
{

#define ENTITY_NAME_SKYBOX    "SkyBox"

class EnSkyBox :  public yaf3d::BaseEntity
{
    public:
                                                    EnSkyBox();

        virtual                                     ~EnSkyBox();
        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! If this entity is used in menu system then we want it to be persistent
        bool                                        isPersistent() const { return _usedInMenu; }

        //! Enable / disable skybox rendering
        void                                        enable( bool en );

    protected:

        //! Entity parameter for texnames (6)
        std::string                                 _texNames[ 6 ];

        //! This entity is persistent so it has to trigger its destruction on shutdown ifself.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup skybox
        osg::ref_ptr< osg::Group >                  setupSkybox();

        vrc::gameutils::EyeTransform*               _p_transformEyePoint;
           
        osg::ref_ptr< osg::Group >                  _p_skyGrp;

        osg::ref_ptr< osg::Group >                  _node;
           
        osg::ref_ptr< osg::Geode >                  _geode;

        osg::Node*                                  makeBox();

        std::map< short, std::string >              _textureFilenameMap;

        bool                                        _usedInMenu;

        bool                                        _enable;
};

//! Entity type definition used for type registry
class SkyBoxEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    SkyBoxEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_SKYBOX, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~SkyBoxEntityFactory() {}

        Macro_CreateEntity( EnSkyBox );
};

} // namespace vrc

#endif // _VRC_SKYBOX_H_
