/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # entity properties gui
 #
 #   date of creation:  01/23/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _ENTITY_PROPS_H_
#define _ENTITY_PROPS_H_


#include <vrc_main.h>
#include "guibase.h"


#define SYMBOL_ENTITYPROPERTIES_STYLE       wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_ENTITYPROPERTIES_TITLE       _( "Entity Properties" )
#define SYMBOL_ENTITYPROPERTIES_IDNAME      ID_ENTITYPROPERTIES
#define SYMBOL_ENTITYPROPERTIES_SIZE        wxSize(250, 300)
#define SYMBOL_ENTITYPROPERTIES_POSITION    wxDefaultPosition


//! Class for creating an entity property dialog or list
class EntityProperties: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( EntityProperties )
    DECLARE_EVENT_TABLE()

    public:

                                                    EntityProperties();

                                                    ~EntityProperties();

        //! Create a dialog with entity properties
        void                                        createFrame( wxWindow* p_parent );

        //! Update the properties reflecting the entity parameters.
        void                                        updateProperties( yaf3d::BaseEntity* p_entity );

        //! Set the current property values in given entity.
        void                                        updateEntity( yaf3d::BaseEntity* p_entity );

        //! Public control identifiers
        enum
        {
            ID_BUTTON_ENTITIY_PROPS_CANCEL  = 11020,
            ID_BUTTON_ENTITIY_PROPS_OK      = 11021
        };

    protected:

        //! Create the controls and sizers for dialog
        void                                        createControls();

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITIY_PROPS_CANCEL
        void                                        onButtonEntitiyPropsCancelClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITIY_PROPS_OK
        void                                        onButtonEntitiyPropsOkClick( wxCommandEvent& event );

        //! Should we show tooltips?
        static bool                                 ShowToolTips();

        wxPropertyGrid*                             _p_entityProps;

        //! Control identifiers
        enum
        {
            ID_ENTITYPROPERTIES             = 11001,
            ID_PANEL_ENTITY_PROPS           = 11002,
            ID_PROPGRID_ENTITY_PROPS        = 11004
        };
};

#endif // _ENTITY_PROPS_H_
