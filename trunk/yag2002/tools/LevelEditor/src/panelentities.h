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
 # entity panel
 #
 #   date of creation:  01/23/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _PANELENTITIES_H_
#define _PANELENTITIES_H_


#include "guibase.h"

#define SYMBOL_PANELENTITIES_STYLE      wxRAISED_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_PANELENTITIES_IDNAME     ID_PANEL_ENTITIES
#define SYMBOL_PANELENTITIES_SIZE       wxDefaultSize
#define SYMBOL_PANELENTITIES_POSITION   wxDefaultPosition


class GameInterface;
class EntityProperties;


//! Entity panel class
class PanelEntities: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( PanelEntities )
    DECLARE_EVENT_TABLE()

    public:

        explicit                                    PanelEntities( wxWindow* parent, GameInterface* p_interface );

                                                    ~PanelEntities();

        //! Setup the controls
        void                                        setupControls( bool enable, const std::string& levelfilename = "" );

        //! Select given entity in tree.
        void                                        selectEntity( yaf3d::BaseEntity* p_entity );

    protected:

        //! Make wx happy
                                                    PanelEntities() {}

        //! Create the controls and sizers
        void                                        createControls();

        //! Fill the entity tree.
        void                                        updateEntityTree();

        //! Get entity from tree selection. This method validates the stored entity in tree info data.
        yaf3d::BaseEntity*                          findTreeEntity( wxTreeItemId item );

        //! Swap the entities in entity manager's pool, used by move functionality.
        void                                        swapEntities( wxTreeItemId item1, wxTreeItemId item2 );

        //! Move an entity from itemsrc to itemdst in entity manager's pool, used by copy/paste and entity add functionalities.
        void                                        moveEntity( wxTreeItemId itemsrc, wxTreeItemId itemdst );

        //! Create a label for an entity tree item given entity type and instance name.
        std::string                                 createTreeItemLabel( const std::string& entitytype, const std::string& instancename );

        //! Update the properties reflecting the entity parameters. Pass NULL for cleaning the property list.
        void                                        updateProperties( yaf3d::BaseEntity* p_entity );

        //! wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
        void                                        onTreectrlSelChanged( wxTreeEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_ADD
        void                                        onButtonAddEntityClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_COPY
        void                                        onButtonEntityCopyClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_CUT
        void                                        onButtonEntityCutClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_PASTE
        void                                        onButtonEntityPasteClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_MOVE_UP
        void                                        onButtonEntityMoveUpClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_MOVE_DOWN
        void                                        onButtonEntityMoveDownClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_DELETE
        void                                        onButtonEntityDeleteClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ENTITY_UPDATE
        void                                        onButtonEntityUpdateClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_ENTITY_AUTO_UPDATE
        void                                        onCheckboxEntityAutoUpdateClick( wxCommandEvent& event );

        //! wxEVT_PG_CHANGED event handler for ID_PROPGRID_ENTITY_PROPS
        void                                        onPropertyGridChange( wxPropertyGridEvent& event );

        //! Should we show tooltips?
        static bool                                 ShowToolTips();

        GameInterface*                              _p_gameInterface;

        wxComboBox*                                 _p_comboEntityTypes;

        wxButton*                                   _p_btnEntityUpdate;

        wxCheckBox*                                 _p_checkEntityAutoUpdate;

        wxTreeCtrl*                                 _p_treeEntity;

        wxPropertyGrid*                             _p_entityProps;

        bool                                        _stateSetupTree;

        std::string                                 _treeRootName;

        wxTreeItemId                                _cutCopySelection;

        bool                                        _copyOperation;

        enum
        {
            ID_PANEL_ENTITIES               = 10018,
            ID_BUTTON_ENTITY_ADD            = 10019,
            ID_COMBOBOX                     = 10020,
            ID_BUTTON_ENTITY_COPY           = 10021,
            ID_BUTTON_ENTITY_CUT            = 10022,
            ID_BUTTON_ENTITY_PASTE          = 10023,
            ID_BUTTON_ENTITY_MOVE_UP        = 10024,
            ID_BUTTON_ENTITY_MOVE_DOWN      = 10025,
            ID_BUTTON_ENTITY_DELETE         = 10026,
            ID_BUTTON_ENTITY_UPDATE         = 10027,
            ID_CHECKBOX_ENTITY_AUTO_UPDATE  = 10028,
            ID_TREECTRL                     = 10029,
            ID_LISTCTRL                     = 10030,
            ID_PROPGRID_PROPS               = 10031
        };
};

#endif // _PANELENTITIES_H_
