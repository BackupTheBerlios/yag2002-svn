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
 # entity game settings
 #   all game settings are edited with the guis in this entity
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_GAMESETTINGS_H_
#define _CTD_GAMESETTINGS_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include <ctd_utils.h>

namespace CTD
{

#define ENTITY_NAME_GAMESETTINGS    "GameSettings"

class DialogGameSettings;

//! All game settings are controled by this entity.
class EnGameSettings :  public BaseEntity
{
    public:
                                                    EnGameSettings();

        virtual                                     ~EnGameSettings();


        //! This entity does not need a transform node, which would be created by level manager on loading
        //!   We create an own one and add it into scene's root node
        bool                                        needTransformation() { return false; }

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

    protected:

        //! Entity parameters

        std::string                                 _settingsDialog;

    protected:

        std::auto_ptr< DialogGameSettings >         _mainDialog;
};

//! Entity type definition used for type registry
class GameSettingsEntityFactory : public BaseEntityFactory
{
    public:
                                                    GameSettingsEntityFactory() : BaseEntityFactory( ENTITY_NAME_GAMESETTINGS ) {}

        virtual                                     ~GameSettingsEntityFactory() {}

        Macro_CreateEntity( EnGameSettings );
};

}

#endif // _CTD_GAMESETTINGS_H_
