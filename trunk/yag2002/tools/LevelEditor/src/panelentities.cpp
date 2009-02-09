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


#include <vrc_main.h>
#include "guibase.h"
#include "panelentities.h"
#include "entityproperties.h"
#include "pix/treeroot.xpm"
#include "pix/treehidden.xpm"
#include "pix/treeent.xpm"
#include "pix/treehiddenent.xpm"
#include "gameinterface.h"
#include "navigation.h"


//! Tree's icon identifiers
#define TREE_ICON_ROOT          0
#define TREE_ICON_HIDDEN        1
#define TREE_ICON_ENTITY        2
#define TREE_ICON_HIDDEN_ENTITY 3

//! Property name for "InstanceName"
#define PROP_INSTANCE_NAME      "_$InstName$_"



//! Class holding entity related data in tree items
class TreeItemData : public wxTreeItemData
{

    public:
                                                    TreeItemData( const std::string& entitytype, const std::string& instancename ) :
                                                     _entityType( entitytype ),
                                                     _entityInstName( instancename ),
                                                     _p_entity( NULL )
                                                    {
                                                    }

        virtual                                     ~TreeItemData()
                                                    {
                                                    }

        const std::string&                          getEntityType() const
                                                    {
                                                        return _entityType;
                                                    }

        yaf3d::BaseEntity*                          getEntity()
                                                    {
                                                        return _p_entity;
                                                    }

        void                                        setEntity( yaf3d::BaseEntity* p_entity )
                                                    {
                                                        _p_entity = p_entity;
                                                    }

        const std::string&                          getEntityInstanceName() const
                                                    {
                                                        return _entityInstName;
                                                    }

        void                                        setEntityType( const std::string& entitytype )
                                                    {
                                                        _entityType  = entitytype;
                                                    }

        void                                        setEntityInstanceName( const std::string& instname )
                                                    {
                                                        _entityInstName  = instname;
                                                    }
    protected:

        //! Entity type
        std::string                                 _entityType;

        //! Entity instance name
        std::string                                 _entityInstName;

        yaf3d::BaseEntity*                          _p_entity;
};


IMPLEMENT_DYNAMIC_CLASS( PanelEntities, wxPanel )

BEGIN_EVENT_TABLE( PanelEntities, wxPanel )

    EVT_TREE_SEL_CHANGED( ID_TREECTRL, PanelEntities::onTreectrlSelChanged )

    EVT_BUTTON( ID_BUTTON_ENTITY_ADD, PanelEntities::onButtonAddEntityClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_COPY, PanelEntities::onButtonEntityCopyClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_CUT, PanelEntities::onButtonEntityCutClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_PASTE, PanelEntities::onButtonEntityPasteClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_MOVE_UP, PanelEntities::onButtonEntityMoveUpClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_MOVE_DOWN, PanelEntities::onButtonEntityMoveDownClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_DELETE, PanelEntities::onButtonEntityDeleteClick )

    EVT_BUTTON( ID_BUTTON_ENTITY_UPDATE, PanelEntities::onButtonEntityUpdateClick )

    EVT_CHECKBOX( ID_CHECKBOX_ENTITY_AUTO_UPDATE, PanelEntities::onCheckboxEntityAutoUpdateClick )

    EVT_PG_CHANGED( ID_PROPGRID_PROPS, PanelEntities::onPropertyGridChange )

END_EVENT_TABLE()


PanelEntities::PanelEntities( wxWindow* parent, GameInterface* p_interface ) :
 _p_gameInterface( p_interface ),
 _p_comboEntityTypes( NULL ),
 _p_btnEntityUpdate( NULL ),
 _p_checkEntityAutoUpdate( NULL ),
 _p_treeEntity( NULL ),
 _p_entityProps( NULL ),
 _stateSetupTree( false ),
 _copyOperation( false )
{
    wxPanel::Create( parent, ID_PANEL_ENTITIES, wxDefaultPosition, wxDefaultSize, SYMBOL_PANELENTITIES_STYLE );

    createControls();

    if ( GetSizer() )
        GetSizer()->Fit( this );
}

PanelEntities::~PanelEntities()
{
}

void PanelEntities::createControls()
{
    PanelEntities* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Manage Entities"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemPanel1, wxID_ANY, _T(""));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemStaticBoxSizer3->Add(itemStaticBoxSizer4, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer5, 0, wxGROW|wxBOTTOM, 5);

    wxButton* itemButton6 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_ADD, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton6->SetHelpText(_("Add a new entity"));
    if (PanelEntities::ShowToolTips())
        itemButton6->SetToolTip(_("Add a new entity"));
    itemBoxSizer5->Add(itemButton6, 1, wxGROW|wxRIGHT|wxTOP, 5);

    wxArrayString _p_comboEntityTypesStrings;
    _p_comboEntityTypes = new wxComboBox( itemPanel1, ID_COMBOBOX, _T(""), wxDefaultPosition, wxDefaultSize, _p_comboEntityTypesStrings, wxCB_READONLY|wxCB_SORT );
    _p_comboEntityTypes->SetHelpText(_("Available entity types"));
    if (PanelEntities::ShowToolTips())
        _p_comboEntityTypes->SetToolTip(_("Available entity types"));
    itemBoxSizer5->Add(_p_comboEntityTypes, 2, wxGROW|wxLEFT|wxTOP, 5);

    wxStaticLine* itemStaticLine8 = new wxStaticLine( itemPanel1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemStaticBoxSizer4->Add(itemStaticLine8, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer9, 0, wxGROW, 5);

    wxButton* itemButton10 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_COPY, _("Copy"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton10->SetHelpText(_("Copy selected entity"));
    if (PanelEntities::ShowToolTips())
        itemButton10->SetToolTip(_("Copy selected entity"));
    itemBoxSizer9->Add(itemButton10, 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton11 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_CUT, _("Cut"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton11->SetHelpText(_("Cut selected entity"));
    if (PanelEntities::ShowToolTips())
        itemButton11->SetToolTip(_("Cut selected entity"));
    itemBoxSizer9->Add(itemButton11, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_PASTE, _("Paste"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetHelpText(_("Paste entity after selected one"));
    if (PanelEntities::ShowToolTips())
        itemButton12->SetToolTip(_("Paste entity after selected one"));
    itemBoxSizer9->Add(itemButton12, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer13, 0, wxGROW, 5);

    wxButton* itemButton14 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_MOVE_UP, _("Move up"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetHelpText(_("Move selected entity up in tree"));
    if (PanelEntities::ShowToolTips())
        itemButton14->SetToolTip(_("Move selected entity up in tree"));
    itemBoxSizer13->Add(itemButton14, 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

    wxButton* itemButton15 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_MOVE_DOWN, _("Move down"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetHelpText(_("Move selected entity down in tree"));
    if (PanelEntities::ShowToolTips())
        itemButton15->SetToolTip(_("Move selected entity down in tree"));
    itemBoxSizer13->Add(itemButton15, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxButton* itemButton16 = new wxButton( itemPanel1, ID_BUTTON_ENTITY_DELETE, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetHelpText(_("Delete selection entity"));
    if (PanelEntities::ShowToolTips())
        itemButton16->SetToolTip(_("Delete selection entity"));
    itemBoxSizer13->Add(itemButton16, 1, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    //! NOTE: we currently do not need this, all property changes take immediately place
#if 0
    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemPanel1, wxID_ANY, _T(""));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemStaticBoxSizer3->Add(itemStaticBoxSizer17, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer18, 0, wxGROW|wxTOP|wxBOTTOM, 5);

    _p_btnEntityUpdate = new wxButton( itemPanel1, ID_BUTTON_ENTITY_UPDATE, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    _p_btnEntityUpdate->SetHelpText(_("Update entity parameters, e.g. after changing them"));
    if (PanelEntities::ShowToolTips())
        _p_btnEntityUpdate->SetToolTip(_("Update entity parameters, e.g. after changing them"));
    _p_btnEntityUpdate->Enable(false);
    itemBoxSizer18->Add(_p_btnEntityUpdate, 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

    _p_checkEntityAutoUpdate = new wxCheckBox( itemPanel1, ID_CHECKBOX_ENTITY_AUTO_UPDATE, _("Auto update"), wxDefaultPosition, wxDefaultSize, 0 );
    _p_checkEntityAutoUpdate->SetValue(true);
    _p_checkEntityAutoUpdate->SetHelpText(_("Update entity parameter immediately after editing"));
    if (PanelEntities::ShowToolTips())
        _p_checkEntityAutoUpdate->SetToolTip(_("Update entity parameter immediately after editing"));
    itemBoxSizer18->Add(_p_checkEntityAutoUpdate, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
#endif

    _p_treeEntity = new wxTreeCtrl( itemPanel1, ID_TREECTRL, wxDefaultPosition, wxSize(100, 100), wxTR_SINGLE );
    itemBoxSizer2->Add(_p_treeEntity, 2, wxGROW|wxALL, 2);

    // setup tree's icons
    wxBitmap iconroot( treeroot_xpm );
    wxBitmap iconhidden( treehidden_xpm );
    wxBitmap iconentity( treeent_xpm );
    wxBitmap iconhiddenentity( treehiddenent_xpm );
    wxImageList* p_imageList = new wxImageList( 16, 16, true );
    p_imageList->Add( wxBitmap( iconroot ) );
    p_imageList->Add( wxBitmap( iconhidden ) );
    p_imageList->Add( wxBitmap( iconentity ) );
    p_imageList->Add( wxBitmap( iconhiddenentity ) );
    _p_treeEntity->AssignImageList( p_imageList);

    // setup the entity property grid
    _p_entityProps = new wxPropertyGrid( itemPanel1, ID_PROPGRID_PROPS, wxDefaultPosition, wxSize(300, 250), wxPG_SPLITTER_AUTO_CENTER | wxPG_BOLD_MODIFIED );

    _p_entityProps->SetMinSize( wxSize( 250, 150 ) );

    itemBoxSizer2->Add( _p_entityProps, 1, wxGROW|wxALL, 2 );
}

void PanelEntities::updateProperties( yaf3d::BaseEntity* p_entity )
{
    assert( _p_entityProps && "gui not created!" );

    if ( !p_entity )
    {
        _p_entityProps->Clear();
        return;
    }

    std::vector< std::vector< std::string > > attributes;
    p_entity->getAttributeManager().getAttributesAsString( attributes );

    _p_entityProps->Clear();

    _p_entityProps->Append( new wxPropertyCategory( wxT( "" ) ) );

    _p_entityProps->Append( new wxStringProperty( "Instance name", PROP_INSTANCE_NAME, p_entity->getInstanceName() ) );

    _p_entityProps->Append( new wxPropertyCategory( wxT( "Parameters" ) ) );

    std::string info;
    std::vector< std::vector< std::string > >::iterator p_beg = attributes.begin(), p_end = attributes.end();
    for ( ; p_beg != p_end; ++p_beg )
        _p_entityProps->Append( new wxStringProperty( ( *p_beg )[ 0 ], wxPG_LABEL, ( *p_beg )[ 1 ] ) );

    // append entity features
    {
        wxPGProperty* p_prop = NULL;
        _p_entityProps->Append( new wxPropertyCategory( wxT( "Features" ) ) );
        p_prop = _p_entityProps->Append( new wxBoolProperty( "Transformable", wxPG_LABEL, p_entity->isTransformable() ) );
        _p_entityProps->DisableProperty( p_prop );
        p_prop = _p_entityProps->Append( new wxBoolProperty( "Persistent", wxPG_LABEL, p_entity->isPersistent() ) );
        _p_entityProps->DisableProperty( p_prop );
        p_prop = _p_entityProps->Append( new wxBoolProperty( "Is updated", wxPG_LABEL, yaf3d::EntityManager::get()->isRegisteredUpdate( p_entity ) ) );
        _p_entityProps->DisableProperty( p_prop );
        p_prop = _p_entityProps->Append( new wxBoolProperty( "Gets notification", wxPG_LABEL, yaf3d::EntityManager::get()->isRegisteredNotification( p_entity ) ) );
        _p_entityProps->DisableProperty( p_prop );
    }

    // append entity policies
    {
        wxPGProperty* p_prop = NULL;
        _p_entityProps->Append( new wxPropertyCategory( wxT( "Policies" ) ) );
        yaf3d::BaseEntityFactory* p_factory = yaf3d::EntityManager::get()->getEntityFactory( p_entity->getTypeName() );
        assert( p_factory && "invalid entity factory!" );

        std::string gamemodepolicy;
        if ( p_factory->getCreationPolicy() & yaf3d::BaseEntityFactory::Standalone )
            gamemodepolicy += " Standalone ";

        if ( p_factory->getCreationPolicy() & yaf3d::BaseEntityFactory::Client )
            gamemodepolicy += " Client ";

        if ( p_factory->getCreationPolicy() & yaf3d::BaseEntityFactory::Server )
            gamemodepolicy += " Server ";

        if ( gamemodepolicy.length() )
            gamemodepolicy = gamemodepolicy.substr( 1, gamemodepolicy.length() - 2 );

         p_prop = _p_entityProps->Append( new wxStringProperty( "Game modes", wxPG_LABEL, gamemodepolicy ) );
        _p_entityProps->DisableProperty( p_prop );

        std::string initpolicy;

        if ( p_factory->getInitializationPolicy() & yaf3d::BaseEntityFactory::OnLoadingLevel )
            initpolicy += " OnLoadingLevel ";

        if ( p_factory->getInitializationPolicy() & yaf3d::BaseEntityFactory::OnRunningLevel )
            initpolicy += " OnRunningLevel ";

        if ( initpolicy.length() )
            initpolicy = initpolicy.substr( 1, initpolicy.length() - 2 );

         p_prop = _p_entityProps->Append( new wxStringProperty( "Initialization", wxPG_LABEL, initpolicy ) );
        _p_entityProps->DisableProperty( p_prop );
    }

    // refresh the grid so the elements get drawn
    _p_entityProps->RefreshGrid();
}

void PanelEntities::setupControls( bool enable, const std::string& levelfilename )
{
    if ( !enable )
    {
        _p_comboEntityTypes->SetSelection( 0 );
        _p_treeEntity->DeleteAllItems();
        Disable();
        return;
    }

    Enable( true );

    // lock the game loop avoiding resource access conflict (the game loop is in an own thread)
    ScopedGameUpdateLock   lockupdate;

    // get all available entities
    {
        _p_comboEntityTypes->Clear();

        std::vector< yaf3d::BaseEntityFactory* > factories;
        yaf3d::EntityManager::get()->getAllEntityFactories( factories );
        std::vector< yaf3d::BaseEntityFactory* >::iterator p_beg = factories.begin(), p_end = factories.end();

        for ( ; p_beg != p_end; ++p_beg )
        {
            // filter out server entities
            if ( ( *p_beg )->getCreationPolicy() & ( yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client ) )
                _p_comboEntityTypes->Append( ( *p_beg )->getType() );
        }

        _p_comboEntityTypes->SetSelection( 0 );
    }

    _treeRootName = levelfilename;

    updateEntityTree();
}

void PanelEntities::selectEntity( yaf3d::BaseEntity* p_entity )
{
    if ( !p_entity || !_p_treeEntity || !_p_treeEntity->GetRootItem().IsOk() )
    {
        // lock game loop
        ScopedGameUpdateLock   lockupdate;

        // remove any selection
        GameNavigator::get()->highlightEntity( NULL );

        return;
    }

    // find the entity in tree
    wxTreeItemId item = _p_treeEntity->GetRootItem();
    wxTreeItemIdValue cookie;
    for ( item = _p_treeEntity->GetFirstChild( item, cookie ); item.IsOk(); item = _p_treeEntity->GetNextSibling( item ) )
    {
        TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( item ) );
        if ( !p_info )
            continue;

        if ( p_info->getEntity() == p_entity )
        {
            _p_treeEntity->SelectItem( item, true );
            break;
        }
    }
}

void PanelEntities::updateEntityTree()
{
    // setup the entity tree
    {
        // set this state in order to skip tree callbacks during setting it up
        _stateSetupTree = true;

        _p_treeEntity->DeleteAllItems();

        std::string rootlable = _treeRootName.length() ? _treeRootName : "<no level loaded>";
        wxTreeItemId treeroot = _p_treeEntity->AddRoot( rootlable, TREE_ICON_ROOT );
        _p_treeEntity->SetItemHasChildren( treeroot, true );
        TreeItemData* p_rootinfo = new TreeItemData( "", rootlable );
        _p_treeEntity->SetItemData( treeroot, p_rootinfo );

        std::vector< yaf3d::BaseEntity* > entities;
        yaf3d::EntityManager::get()->getAllEntities( entities );
        std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();

        std::string  entityname;
        wxTreeItemId item;
        int          icon  = 0;

        for ( ; p_beg != p_end; ++p_beg )
        {
            entityname = ( *p_beg )->getInstanceName();
            if ( !entityname.length() || ( entityname[ 0 ] == '_' ) )
                icon = TREE_ICON_HIDDEN_ENTITY;
            else
                icon = TREE_ICON_ENTITY;

            item = _p_treeEntity->AppendItem( treeroot, createTreeItemLabel( ( *p_beg )->getTypeName(), ( *p_beg )->getInstanceName() ), icon );

            // the the tree item data containing information about the entity
            TreeItemData* p_info = new TreeItemData( ( *p_beg )->getTypeName(), ( *p_beg )->getInstanceName() );
            p_info->setEntity( *p_beg );
            _p_treeEntity->SetItemData( item, p_info );
        }

        // expand tree's root item
        _p_treeEntity->Toggle( treeroot );
        _p_treeEntity->SelectItem( treeroot );
        _stateSetupTree = false;
    }
}

yaf3d::BaseEntity* PanelEntities::findTreeEntity( wxTreeItemId item )
{
    TreeItemData* p_data = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( item ) );
    assert( p_data && "invalid tree data!" );

    yaf3d::BaseEntity* p_entity = p_data->getEntity();
    if ( !p_entity )
        return NULL;

    // get the entity and update the property list
    std::vector< yaf3d::BaseEntity* > entities;
    yaf3d::EntityManager::get()->getAllEntities( entities );
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == p_entity )
            break;
    }
    // found the entity?
    if ( p_beg == p_end )
        return NULL;

    return p_entity;
}

void PanelEntities::swapEntities( wxTreeItemId item1, wxTreeItemId item2 )
{
    yaf3d::BaseEntity* p_entity1 = NULL, *p_entity2 = NULL;

    TreeItemData* p_info1 = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( item1 ) );
    TreeItemData* p_info2 = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( item2 ) );
    if ( !p_info1 || !p_info2 )
        return;

    p_entity1 = p_info1->getEntity();
    p_entity2 = p_info2->getEntity();

    if ( !p_entity1 || !p_entity2 )
        return;

    if ( p_entity1 == p_entity2 )
        return;

    // swap the entities in entity manger's pool
    {
        ScopedGameUpdateLock lock;

        // here we access to original entity pool in entity manager and modify it
        std::vector< yaf3d::BaseEntity* >& entities = yaf3d::EntityManager::get()->getRefAllEntities();
        std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
        std::size_t pos1 = -1, pos2 = -1, cnt = 0;
        for ( ; p_beg != p_end; ++p_beg, cnt++ )
        {
            if ( *p_beg == p_entity1 )
                pos1 = cnt;

            if ( *p_beg == p_entity2 )
                pos2 = cnt;

            if ( ( pos1 != -1 ) && ( pos2 != -1 ) )
            {
                entities[ pos1 ] = p_entity2;
                entities[ pos2 ] = p_entity1;
                break;
            }
        }
    }
}

void PanelEntities::moveEntity( wxTreeItemId itemsrc, wxTreeItemId itemdst )
{
    yaf3d::BaseEntity* p_entitysrc = NULL, *p_entitydst = NULL;

    TreeItemData* p_infosrc = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( itemsrc ) );
    TreeItemData* p_infodst = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( itemdst ) );
    if ( !p_infosrc || !p_infodst )
        return;

    p_entitysrc = p_infosrc->getEntity();
    p_entitydst = p_infodst->getEntity();

    if ( !p_entitysrc || !p_entitydst )
        return;

    if ( p_entitysrc == p_entitydst )
        return;

    // swap the entities in entity manger's pool
    {
        ScopedGameUpdateLock lock;

        // here we access to original entity pool in entity manager and modify it
        std::vector< yaf3d::BaseEntity* >& entities = yaf3d::EntityManager::get()->getRefAllEntities();
        std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
        // first remove the src from list ...
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( *p_beg == p_entitysrc )
            {
                entities.erase( p_beg );
                break;
            }
        }
        // ... now insert it to proper place
        for ( p_beg = entities.begin(), p_end = entities.end(); p_beg != p_end; ++p_beg )
        {
            if ( *p_beg == p_entitydst )
            {
                // move the entity to the position after the given destination entity
                p_beg++;
                if ( p_beg == p_end )
                    entities.push_back( p_entitysrc );
                else
                    entities.insert( p_beg, p_entitysrc );

                break;
            }
        }
    }
}

std::string PanelEntities::createTreeItemLabel( const std::string& entitytype, const std::string& instancename )
{
    std::string label;
    label = instancename + "  [" + entitytype + "]";
    return label;
}

bool PanelEntities::ShowToolTips()
{
    return true;
}

void PanelEntities::onTreectrlSelChanged( wxTreeEvent& event )
{
    // skip callbacks during tree setup
    if ( _stateSetupTree )
        return;

    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() )
        return;

    TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( sel ) );
    assert( p_info && "invalid tree item data!" );

    if ( sel == _p_treeEntity->GetRootItem() )
    {
        updateProperties( NULL );
        return;
    }

    {
        // lock game loop
        ScopedGameUpdateLock   lockupdate;

        yaf3d::BaseEntity* p_selentity = findTreeEntity( sel );
        if ( !p_selentity )
            return;

        // update entity properties
        updateProperties( p_selentity );
        // highlight the entity
        GameNavigator::get()->highlightEntity( p_selentity );
    }
}

void PanelEntities::onButtonAddEntityClick( wxCommandEvent& event )
{
    int sel = _p_comboEntityTypes->GetSelection();
    if ( sel < 0 )
        return;

    EntityProperties* p_dlg = new EntityProperties();
    p_dlg->createFrame( this );

    std::string enttype = _p_comboEntityTypes->GetString( sel );

    // prototype entity for used for creating and initializing a new entiy
    yaf3d::BaseEntity* p_entity = NULL;

    {
        // lock game loop
        ScopedGameUpdateLock   lockupdate;

        std::string instname( enttype );
        std::transform( instname.begin(), instname.end(), instname.begin(), tolower );
        p_entity = yaf3d::EntityManager::get()->createEntity( enttype, instname, false );
    }

    if ( !p_entity )
    {
        log_error << "[Editor] could not create entity of type " << enttype << std::endl;
        return;
    }

    // fill the property dialog with entity properties
    p_dlg->updateProperties( p_entity );

    if ( p_dlg->ShowModal() != EntityProperties::ID_BUTTON_ENTITIY_PROPS_OK )
    {
        delete p_entity;
        return;
    }

    // create an entity using the entity manager and copy over the entity attributes
    wxTreeItemId newitem = 0;

    {
        // lock game loop
        ScopedGameUpdateLock   lockupdate;

        std::string        name        = p_entity->getInstanceName();
        yaf3d::BaseEntity* p_newentity = yaf3d::EntityManager::get()->createEntity( enttype, name, true );

        // set the entity attributes from properties
        p_dlg->updateEntity( p_newentity );

        // initialize the entity considering its init policy
        yaf3d::BaseEntityFactory* p_factory = yaf3d::EntityManager::get()->getEntityFactory( p_entity->getTypeName() );
        assert( p_factory && "entity factory could not be found!" );

        // setup transformation node
        if ( p_newentity->isTransformable() )
        {
            if ( name.length() )
                p_newentity->getTransformationNode()->setName( name );

            yaf3d::LevelManager::get()->getEntityNodeGroup()->addChild( p_newentity->getTransformationNode() );
        }

        // some entities can be initialized only on level loading!
        unsigned int initpolicy = p_factory->getInitializationPolicy();

        if ( ( initpolicy == yaf3d::BaseEntityFactory::AnyTime ) ||
             ( ( initpolicy == yaf3d::BaseEntityFactory::OnRunningLevel ) && ( yaf3d::GameState::get()->getState() == yaf3d::GameState::MainLoop ) )
           )
        {
            // initialization of entity must happen in game thread
            _p_gameInterface->sendCmd( GameInterface::CMD_INIT_ENTITY, p_newentity );
        }

        // add a tree item for new entity
        int         icon       = 0;
        std::string entityname = p_newentity->getInstanceName();

        if ( !entityname.length() || ( entityname[ 0 ] == '_' ) )
            icon = TREE_ICON_HIDDEN_ENTITY;
        else
            icon = TREE_ICON_ENTITY;

        wxTreeItemId selection = _p_treeEntity->GetSelection();
        if ( selection.IsOk() )
            newitem = _p_treeEntity->InsertItem( _p_treeEntity->GetRootItem(), selection, createTreeItemLabel( p_newentity->getTypeName(), entityname ), icon );
        else
            newitem = _p_treeEntity->AppendItem( _p_treeEntity->GetRootItem(), createTreeItemLabel( p_newentity->getTypeName(), entityname ), icon );

        // the tree item data containing information about the entity
        TreeItemData* p_info = new TreeItemData( entityname, p_newentity->getTypeName() );
        p_info->setEntity( p_newentity );
        _p_treeEntity->SetItemData( newitem, p_info );

        // move the entity to proper place in entity manger's pool
        moveEntity( newitem, selection );
    }

    // update tree
    {
        // sleep a while in order to get a fresh list of entities after creating a new one
        //  some entities create dependant entities
        wxMilliSleep( 100 );
        // update the tree
        updateEntityTree();
        _p_treeEntity->SelectItem( newitem, true );
    }

    // remove the prototype entity
    delete p_entity;
}

void PanelEntities::onButtonEntityCopyClick( wxCommandEvent& event )
{
    // get the tree item
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
    {
        _cutCopySelection = 0;
        return;
    }

    _p_treeEntity->SetItemBold( sel, true );
    _cutCopySelection = sel;
    _copyOperation    = true;
}

void PanelEntities::onButtonEntityCutClick( wxCommandEvent& event )
{
    if ( _cutCopySelection.IsOk() )
    {
        _p_treeEntity->SetItemBold( _cutCopySelection, false );
        _cutCopySelection = 0;
    }

    // get the tree item
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
    {
        _cutCopySelection = 0;
        return;
    }

    _p_treeEntity->SetItemBold( sel, true );
    _cutCopySelection = sel;
    _copyOperation    = false;
}

void PanelEntities::onButtonEntityPasteClick( wxCommandEvent& event )
{
    // get the tree item
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
        return;

    // check if copy or cut operation is in action
    if ( !_cutCopySelection.IsOk() )
        return;

    _p_treeEntity->SetItemBold( _cutCopySelection, false );

    // copy/paste operation
    if ( _copyOperation )
    {
        TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( _cutCopySelection ) );
        assert( p_info && "invalid tree item data!" );

        // clone entity
        {
            // lock game loop
            ScopedGameUpdateLock   lockupdate;

            // get the entity and update its props
            yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->findEntity( p_info->getEntityType(), p_info->getEntityInstanceName() );

            // first, check if the entity still exists
            if ( !p_entity )
                return;

            // clone it now
            p_entity = p_entity->clone( p_info->getEntityInstanceName() + "_new", yaf3d::LevelManager::get()->getEntityNodeGroup().get() );
            // initialization of entity must happen in game thread
            _p_gameInterface->sendCmd( GameInterface::CMD_INIT_ENTITY, p_entity );

            int icon = _p_treeEntity->GetItemImage( _cutCopySelection );

            // create a cool label for tree item
            std::string itemtext = createTreeItemLabel( p_entity->getTypeName(), p_entity->getInstanceName() );

            wxTreeItemId newitem = _p_treeEntity->InsertItem( _p_treeEntity->GetRootItem(), sel, itemtext, icon );
            TreeItemData* p_newinfo = new TreeItemData( p_entity->getTypeName(), p_entity->getInstanceName() );
            p_newinfo->setEntity( p_entity );
            _p_treeEntity->SetItemData( newitem, p_newinfo );

            // now move also the fresh created entity to proper place in entity manager's entity pool
            moveEntity( newitem, sel );

            // now select the new item (after setting its item data!)
            _p_treeEntity->SelectItem( newitem );
        }
    }
    else // cut/paste operation
    {
        // check if the same item selected
        if ( _cutCopySelection == sel )
            return;

        wxString itemtext = _p_treeEntity->GetItemText( _cutCopySelection );
        int      icon     = _p_treeEntity->GetItemImage( _cutCopySelection );

        // copy over the item info
        TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( _cutCopySelection ) );
        assert( p_info && "invalid tree item data!" );
        TreeItemData* p_newinfo = new TreeItemData( *p_info );

        // attention: the order of tree item operations is important because of the way how item info is evaluated in on tree item selection change callback!
        wxTreeItemId newitem = _p_treeEntity->InsertItem( _p_treeEntity->GetRootItem(), sel, itemtext, icon );
        _p_treeEntity->SetItemData( newitem, p_newinfo );

        // now move the entity to proper place in entity manager's entity pool
        moveEntity( _cutCopySelection, sel );

        _p_treeEntity->SelectItem( newitem );
        _p_treeEntity->Delete( _cutCopySelection );
    }

    _cutCopySelection = 0;
}

void PanelEntities::onButtonEntityMoveUpClick( wxCommandEvent& event )
{
    // get the tree item
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
        return;

    wxTreeItemId previtem = _p_treeEntity->GetPrevSibling( sel );
    if ( !previtem.IsOk() )
        return;

    // swap the entity oder in entity manager
    swapEntities( sel, previtem );

    wxString itemtext = _p_treeEntity->GetItemText( sel );
    int      icon     = _p_treeEntity->GetItemImage( sel );

    previtem = _p_treeEntity->GetPrevSibling( previtem );
    wxTreeItemId newitem = _p_treeEntity->InsertItem( _p_treeEntity->GetRootItem(), previtem, itemtext, icon );

    // copy over the item info
    TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( sel ) );
    assert( p_info && "invalid tree item data!" );
    TreeItemData* p_newinfo = new TreeItemData( *p_info );
    _p_treeEntity->SetItemData( newitem, p_newinfo );

    // first select the new item, then delete the old one, as deletion triggers the on tree item selection change!
    _p_treeEntity->SelectItem( newitem );
    _p_treeEntity->Delete( sel );
}

void PanelEntities::onButtonEntityMoveDownClick( wxCommandEvent& event )
{
    // get the tree item
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
        return;

    wxTreeItemId nextitem = _p_treeEntity->GetNextSibling( sel );
    if ( !nextitem.IsOk() )
        return;

    // swap the entity oder in entity manager
    swapEntities( sel, nextitem );

    wxString itemtext = _p_treeEntity->GetItemText( sel );
    int      icon     = _p_treeEntity->GetItemImage( sel );

    // copy over the item info
    TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( sel ) );
    assert( p_info && "invalid tree item data!" );
    TreeItemData* p_newinfo = new TreeItemData( *p_info );

    // insert the new tree item
    wxTreeItemId newitem = _p_treeEntity->InsertItem( _p_treeEntity->GetRootItem(), nextitem, itemtext, icon );
    // first set the item data, then select the item in tree, as SelectItem triggers the callback for item selection change!
    _p_treeEntity->SetItemData( newitem, p_newinfo );

    // first select the new item, then delete the old one, as deletion triggers the on tree item selection change!
    _p_treeEntity->SelectItem( newitem );
    _p_treeEntity->Delete( sel );
}

void PanelEntities::onButtonEntityDeleteClick( wxCommandEvent& event )
{
    // get the tree item
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
        return;

    TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( sel ) );
    assert( p_info && "invalid tree item data!" );

    if ( wxMessageBox( "You really want to delete the entity?", "Attention", wxYES_NO ) == wxNO )
        return;

    // delete entity
    {
        // lock game loop
        ScopedGameUpdateLock   lockupdate;

        // get the entity and update its props
        yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->findEntity( p_info->getEntityType(), p_info->getEntityInstanceName() );
        if ( !p_entity )
            return;

        yaf3d::EntityManager::get()->deleteEntity( p_entity );
    }

    _p_treeEntity->Delete( sel );

    // sleep a while in order to get a fresh list of entities after deletion
    //  the actual deletion takes place on next game update
    wxMilliSleep( 100 );

    // update the tree
    {
        updateEntityTree();
        wxTreeEvent ev;
        onTreectrlSelChanged( ev );
    }
}

void PanelEntities::onButtonEntityUpdateClick( wxCommandEvent& event )
{
}

void PanelEntities::onCheckboxEntityAutoUpdateClick( wxCommandEvent& event )
{
    if ( _p_checkEntityAutoUpdate->GetValue() )
        _p_btnEntityUpdate->Enable( false );
    else
        _p_btnEntityUpdate->Enable( true );
}

void PanelEntities::onPropertyGridChange( wxPropertyGridEvent& event )
{
    wxTreeItemId sel = _p_treeEntity->GetSelection();
    if ( !sel.IsOk() || ( sel == _p_treeEntity->GetRootItem() ) )
        return;

    TreeItemData* p_info = dynamic_cast< TreeItemData* >( _p_treeEntity->GetItemData( sel ) );
    assert( p_info && "invalid tree item data!" );

    wxPGProperty* p_property = event.GetProperty();
    if ( !p_property )
        return;

    const wxString& name = p_property->GetName();
    wxVariant value      = p_property->GetValue();

    // lock game loop
    ScopedGameUpdateLock   lockupdate;

    // get the entity and update its props
    yaf3d::BaseEntity* p_entity = findTreeEntity( sel );
    if ( !p_entity )
        return;

    if ( name == PROP_INSTANCE_NAME )
    {
        p_entity->setInstanceName( value.GetString().c_str() );

        // update the tree item info
        p_info->setEntityInstanceName( value.GetString().c_str() );

        // get the entity type and instance name
        std::string seltext = createTreeItemLabel( p_info->getEntityType(), p_info->getEntityInstanceName() );
        _p_treeEntity->SetItemText( sel, seltext );
        if ( !p_info->getEntityInstanceName().length() || ( p_info->getEntityInstanceName()[ 0 ] == '_' ) )
            _p_treeEntity->SetItemImage( sel, TREE_ICON_HIDDEN_ENTITY );
        else
            _p_treeEntity->SetItemImage( sel, TREE_ICON_ENTITY );
    }
    else
    {
        if ( !p_entity->getAttributeManager().setAttributeValueByString( name.c_str(), value.GetString().c_str() ) )
        {
            wxMessageBox( "Could not set entity property!", "Attention" );
            return;
        }

        yaf3d::EntityManager::get()->sendNotification( YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED, p_entity );
    }

    // highlight the entity so that its bounding box gets updated
    GameNavigator::get()->highlightEntity( p_entity );
}
