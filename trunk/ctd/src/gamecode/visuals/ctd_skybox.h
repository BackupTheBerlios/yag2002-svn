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
 # entity skybox
 #
 #   date of creation:  03/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_SKYBOX_H_
#define _CTD_SKYBOX_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_SKYBOX    "SkyBox"

class EnSkyBox :  public BaseEntity
{
    public:
                                                    EnSkyBox();

        virtual                                     ~EnSkyBox();

        //! This entity can be either persistent or not!
        const bool                                  isPersistent() const { return _isPersistent; }

        //! Set the persistence flag. 
        //! Note: this flag is checked by framework on destruction of a level.
        void                                        setPersistent( bool persistence ) { _isPersistent = persistence; }
        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Enable / disable skybox rendering
        void                                        enable( bool en );

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

    protected:

        //! This entity is persistent so it has to trigger its destruction on shutdown ifself.
        void                                        handleNotification( const EntityNotification& notification );

        EyeTransform*                               _p_transformEyePoint;
           
        osg::ref_ptr< osg::Group >                  _p_skyGrp;

        osg::ref_ptr< osg::Group >                  _node;
           
        osg::ref_ptr< osg::Geode >                  _geode;

        osg::Node*                                  makeBox();

        std::map< short, std::string >              _textureFilenameMap;

        bool                                        _isPersistent;

        bool                                        _enable;
};

//! Entity type definition used for type registry
class SkyBoxEntityFactory : public BaseEntityFactory
{
    public:
                                                    SkyBoxEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_SKYBOX, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~SkyBoxEntityFactory() {}

        Macro_CreateEntity( EnSkyBox );
};

}

#endif // _CTD_SKYBOX_H_
