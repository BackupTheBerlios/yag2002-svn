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
 # class for scripted fish NPC
 #
 #   date of creation:  04/18/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#ifndef _VRC_NPCFISH_H_
#define _VRC_NPCFISH_H_

#include "../../scripting/vrc_script.h"

namespace vrc
{

#define ENTITY_NAME_NPCFISH      "NPCFish"

//! Class for a script controlled fish
class EnNPCFish : public BaseScript< EnNPCFish >, public yaf3d::BaseEntity
{
    public:

                                                EnNPCFish();

        virtual                                 ~EnNPCFish();

        //! Initialize
        void                                    initialize();

        //! This entity needs updating
        void                                    updateEntity( float deltaTime );

        // Exposed methods to Lua
        // ######################

        //! Get position
        void                                    lgetPosition( const Params& arguments, Params& returnvalues );

        //! Set position
        void                                    lsetPosition( const Params& arguments, Params& returnvalues );

        //! Get direction
        void                                    lgetDirection( const Params& arguments, Params& returnvalues );

        //! Set direction
        void                                    lsetDirection( const Params& arguments, Params& returnvalues );

        // ######################

    protected:

        //! Setup the script and bindings
        bool                                    setupScript();

        //! Handle notifications
        void                                    handleNotification( const yaf3d::EntityNotification& notification );

        //! Script file
        std::string                             _scriptFile;

        //! Mesh file
        std::string                             _meshFile;

        //! Mesh node
        osg::ref_ptr< osg::Node >               _mesh;

        //! Position
        osg::Vec3f                              _position;

        //! Position
        osg::Vec3f                              _direction;

        //! Flag indicating need for rotation update
        bool                                    _updateRotation;

        //! Arguments for calling script function 'update'
        Params*                                 _p_fcnArgUpdate;
};

//! Entity type definition used for type registry
class NPCFishEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    NPCFishEntityFactory() : 
                                                     yaf3d::BaseEntityFactory
                                                     ( 
                                                       ENTITY_NAME_NPCFISH, 
                                                       yaf3d::BaseEntityFactory::Client | yaf3d::BaseEntityFactory::Standalone
                                                      )
                                                    {}

        virtual                                     ~NPCFishEntityFactory() {}

        Macro_CreateEntity( EnNPCFish );
};

}

#endif _VRC_NPCFISH_H_
