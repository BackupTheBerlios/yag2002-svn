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


#include <vrc_main.h>
#include "entityproperties.h"
#include "editorutils.h"
#include "navigation.h"
#include "editor.h"

//! Property categories
#define PROP_INSTANCE_NAME              "_$InstName$_"
#define PROP_POLICIES_GAME_MODES        "_$PolicyGameModes$_"
#define PROP_POLICIES_INITIALIZATION    "_$PolicyInitialization$_"


IMPLEMENT_DYNAMIC_CLASS( EntityProperties, wxDialog )


BEGIN_EVENT_TABLE( EntityProperties, wxDialog )

    EVT_BUTTON( ID_BUTTON_ENTITIY_PROPS_CANCEL, EntityProperties::onButtonEntitiyPropsCancelClick )

    EVT_BUTTON( ID_BUTTON_ENTITIY_PROPS_OK, EntityProperties::onButtonEntitiyPropsOkClick )

END_EVENT_TABLE()


EntityProperties::EntityProperties() :
 _p_entityProps( NULL )
{
}

EntityProperties::~EntityProperties()
{
}

void EntityProperties::createFrame( wxWindow* p_parent )
{
    assert( !_p_entityProps && "gui already created!" );

    wxDialog::Create( p_parent, SYMBOL_ENTITYPROPERTIES_IDNAME, SYMBOL_ENTITYPROPERTIES_TITLE, SYMBOL_ENTITYPROPERTIES_POSITION, SYMBOL_ENTITYPROPERTIES_SIZE, SYMBOL_ENTITYPROPERTIES_STYLE );

    createControls();
}

void EntityProperties::createControls()
{
    wxWindow* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemDialog1, ID_PANEL_ENTITY_PROPS, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 1, wxGROW, 5);


    _p_entityProps = new wxPropertyGrid( itemPanel3, ID_PROPGRID_ENTITY_PROPS, wxDefaultPosition, wxSize(200, 250), /*wxPG_SPLITTER_AUTO_CENTER |*/ wxPG_BOLD_MODIFIED );
    _p_entityProps->SetMinSize( wxSize( 200, 150 ) );
    itemBoxSizer5->Add(_p_entityProps, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer7, 0, wxALIGN_RIGHT|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton8 = new wxButton( itemPanel3, ID_BUTTON_ENTITIY_PROPS_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemPanel3, ID_BUTTON_ENTITIY_PROPS_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void EntityProperties::updateProperties( yaf3d::BaseEntity* p_entity )
{
    assert( _p_entityProps && "gui not created!" );
    assert( p_entity && "invalid entity!" );

    std::vector< std::vector< std::string > > attributes;
    p_entity->getAttributeManager().getAttributesAsString( attributes );

    _p_entityProps->Clear();

    _p_entityProps->Append( new wxPropertyCategory( wxT( "" ) ) );

    _p_entityProps->Append( new wxStringProperty( "Instance name", PROP_INSTANCE_NAME, p_entity->getInstanceName() ) );

    _p_entityProps->Append( new wxPropertyCategory( wxT( "Parameters" ) ) );

    std::vector< std::vector< std::string > >::iterator p_beg = attributes.begin(), p_end = attributes.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        std::string value;
        // set default mesh file as initial one
        if ( ( *p_beg )[ 0 ] == "meshFile" )
        {
            yaf3d::SettingsPtr settings = yaf3d::SettingsManager::get()->getProfile( EDITOR_SETTINGS_PROFILE );
            assert( settings.valid() && "invalid editor configuration!" );
            settings->getValue( ES_DEFAULT_MESH, value );
        }
        else
        {
            value = ( *p_beg )[ 1 ];
        }

        _p_entityProps->Append( new wxStringProperty( ( *p_beg )[ 0 ], wxPG_LABEL, value ) );
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

         p_prop = _p_entityProps->Append( new wxStringProperty( "Game modes", PROP_POLICIES_GAME_MODES, gamemodepolicy ) );
        _p_entityProps->DisableProperty( p_prop );

        std::string initpolicy;

        if ( p_factory->getInitializationPolicy() & yaf3d::BaseEntityFactory::OnLoadingLevel )
            initpolicy += " OnLoadingLevel ";

        if ( p_factory->getInitializationPolicy() & yaf3d::BaseEntityFactory::OnRunningLevel )
            initpolicy += " OnRunningLevel ";

        if ( initpolicy.length() )
            initpolicy = initpolicy.substr( 1, initpolicy.length() - 2 );

         p_prop = _p_entityProps->Append( new wxStringProperty( "Initialization", PROP_POLICIES_INITIALIZATION, initpolicy ) );
        _p_entityProps->DisableProperty( p_prop );
    }

    // refresh the grid so the elements get drawn
    _p_entityProps->RefreshGrid();
}

void EntityProperties::updateEntity( yaf3d::BaseEntity* p_entity )
{
    wxPropertyGridIterator attr = _p_entityProps->GetIterator();
    for ( ; !attr.AtEnd(); ++attr )
    {
        wxPGProperty*   p_attr = *attr;
        const wxString& name   = p_attr->GetName();
        wxVariant       value  = p_attr->GetValue();

        // skip the property categories, they are no entity attributes!
        if ( !name.length() )
            continue;
        else if ( name == PROP_INSTANCE_NAME )
            continue;
        else if ( name == PROP_POLICIES_GAME_MODES )
            continue;
        else if ( name == PROP_POLICIES_INITIALIZATION )
            continue;

        p_entity->getAttributeManager().setAttributeValueByString( name.c_str(), value.GetString().c_str() );
    }

    wxPGProperty* p_instname = _p_entityProps->GetProperty( PROP_INSTANCE_NAME );
    assert( p_instname && "no valid property" );

    std::string instname = p_instname->GetValue().GetString();
    p_entity->setInstanceName( instname );
}

bool EntityProperties::ShowToolTips()
{
    return true;
}

void EntityProperties::onButtonEntitiyPropsOkClick( wxCommandEvent& event )
{
    EndModal( ID_BUTTON_ENTITIY_PROPS_OK );
}

void EntityProperties::onButtonEntitiyPropsCancelClick( wxCommandEvent& event )
{
    EndModal( ID_BUTTON_ENTITIY_PROPS_CANCEL );
}
