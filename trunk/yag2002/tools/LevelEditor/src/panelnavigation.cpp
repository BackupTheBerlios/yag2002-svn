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
 # navigation panel
 #
 #   date of creation:  02/02/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "guibase.h"
#include "panelnavigation.h"
#include "editor.h"
#include "navigation.h"
#include "editorutils.h"


IMPLEMENT_DYNAMIC_CLASS( PanelNavigation, wxPanel )


BEGIN_EVENT_TABLE( PanelNavigation, wxPanel )

    EVT_CHECKBOX( ID_CHECKBOX_NAV_ENABLE, PanelNavigation::onCheckboxNavEnableClick )

    EVT_BUTTON( ID_BUTTON_NAV_DEFAULTS, PanelNavigation::onButtonNavDefaultsClick )

    EVT_BUTTON( ID_BUTTON_NAV_APPLY, PanelNavigation::onButtonNavApplyClick )

END_EVENT_TABLE()


PanelNavigation::PanelNavigation( wxWindow* p_parent ) :
 _p_checkboxNavEnable( NULL ),
 _p_textNavSpeed( NULL ),
 _p_textNavPosition( NULL ),
 _p_textNavRotation( NULL ),
 _p_textNavFOV( NULL ),
 _p_textNavNearClip( NULL ),
 _p_textNavFarClip( NULL ),
 _p_textNavBkgColor( NULL )
{
    wxPanel::Create( p_parent, SYMBOL_PANELNAVIGATION_IDNAME, SYMBOL_PANELNAVIGATION_POSITION, SYMBOL_PANELNAVIGATION_SIZE, SYMBOL_PANELNAVIGATION_STYLE );

    createControls();
    if ( GetSizer() )
        GetSizer()->Fit(this);
}

PanelNavigation::~PanelNavigation()
{
}

void PanelNavigation::createControls()
{
    PanelNavigation* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Navigation"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

    _p_checkboxNavEnable = new wxCheckBox( itemPanel1, ID_CHECKBOX_NAV_ENABLE, _("Enable Navigation"), wxDefaultPosition, wxDefaultSize, 0 );
    _p_checkboxNavEnable->SetValue(false);
    itemBoxSizer4->Add(_p_checkboxNavEnable, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer6, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer7, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Speed (units/s)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Initial Position"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("Initial Rotation (Pitch/Yaw)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALL, 6);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer12, 1, wxGROW|wxALL, 5);

    _p_textNavSpeed = new wxTextCtrl( itemPanel1, ID_TEXTCTRL_NAV_SPEED, _("100"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(_p_textNavSpeed, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    _p_textNavPosition = new wxTextCtrl( itemPanel1, ID_TEXTCTRL_NAV_POS, _("0.0 0.0 0.0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(_p_textNavPosition, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    _p_textNavRotation = new wxTextCtrl( itemPanel1, ID_TEXTCTRL_NAV_ROTATION, _("0.0 0.0 0.0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(_p_textNavRotation, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Camera Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemStaticBoxSizer17, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer17->Add(itemBoxSizer18, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemPanel1, wxID_STATIC, _("FOV (degree)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Near Clip Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText21 = new wxStaticText( itemPanel1, wxID_STATIC, _("Far Clip Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText21, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText22 = new wxStaticText( itemPanel1, wxID_STATIC, _("Background Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALL, 6);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer17->Add(itemBoxSizer23, 1, wxGROW|wxALL, 5);

    _p_textNavFOV = new wxTextCtrl( itemPanel1, ID_TEXTCTRL_NAV_FOV, _("45.0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(_p_textNavFOV, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    _p_textNavNearClip = new wxTextCtrl( itemPanel1, ID_TEXTCTRL_NAV_NEARCLIP, _("0.5"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(_p_textNavNearClip, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    _p_textNavFarClip = new wxTextCtrl( itemPanel1, ID_TEXTCTRL_NAV_FARCLIP, _("10000.0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(_p_textNavFarClip, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    _p_textNavBkgColor = new wxColourPickerCtrl( itemPanel1, ID_COLPICKERCTRL, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemBoxSizer23->Add(_p_textNavBkgColor, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticBox* itemStaticBoxSizer28Static = new wxStaticBox(itemPanel1, wxID_ANY, _T(""));
    wxStaticBoxSizer* itemStaticBoxSizer28 = new wxStaticBoxSizer(itemStaticBoxSizer28Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer28, 0, wxGROW|wxALL, 5);

    wxButton* itemButton29 = new wxButton( itemPanel1, ID_BUTTON_NAV_DEFAULTS, _("Reset To Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer28->Add(itemButton29, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton30 = new wxButton( itemPanel1, ID_BUTTON_NAV_APPLY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer28->Add(itemButton30, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void PanelNavigation::loadSettings()
{
    yaf3d::SettingsPtr settings = yaf3d::SettingsManager::get()->getProfile( EDITOR_SETTINGS_PROFILE );
    assert( settings.valid() && "invalid editor configuration!" );

    bool            navEnable = true;
    unsigned int    navSpeed  = 100;
    osg::Vec3f      navPos;
    osg::Vec2f      navRot;
    float           navFOV      = 45.0f;
    float           navNearClip = 0.5f;
    float           navFarClip  = 1000.0f;
    osg::Vec3f      navBkgColor;

    settings->getValue( ES_NAV_ENABLE,   navEnable );
    settings->getValue( ES_NAV_SPEED,    navSpeed );
    settings->getValue( ES_NAV_POSITION, navPos );
    settings->getValue( ES_NAV_ROTATION, navRot );
    settings->getValue( ES_NAV_FOV,      navFOV );
    settings->getValue( ES_NAV_NEARCLIP, navNearClip );
    settings->getValue( ES_NAV_FARCLIP,  navFarClip );
    settings->getValue( ES_NAV_BKGCOLOR, navBkgColor );

    _p_checkboxNavEnable->SetValue( navEnable );
    _p_textNavSpeed->SetValue( Conversion::floatToString( navSpeed ) );
    _p_textNavPosition->SetValue( Conversion::vec3ToString( navPos ) );
    _p_textNavRotation->SetValue( Conversion::vec2ToString( navRot ) );
    _p_textNavFOV->SetValue( Conversion::floatToString( navFOV ) );
    _p_textNavNearClip->SetValue( Conversion::floatToString( navNearClip ) );
    _p_textNavFarClip->SetValue( Conversion::floatToString( navFarClip ) );
    _p_textNavBkgColor->SetColour( Conversion::vecToColour( navBkgColor ) );

    // update the navigator instance
    GameNavigator::get()->enable( navEnable );
    GameNavigator::get()->setSpeed( navSpeed );
    GameNavigator::get()->setFOV( navFOV );
    GameNavigator::get()->setNearFarClip( navNearClip, navFarClip );
    GameNavigator::get()->setBackgroundColor( navBkgColor / 255.0f );
}

void PanelNavigation::onCheckboxNavEnableClick( wxCommandEvent& event )
{
    GameNavigator::get()->enable( _p_checkboxNavEnable->GetValue() );
}

void PanelNavigation::onButtonNavApplyClick( wxCommandEvent& event )
{
    yaf3d::SettingsPtr settings = yaf3d::SettingsManager::get()->getProfile( EDITOR_SETTINGS_PROFILE );
    assert( settings.valid() && "invalid editor configuration!" );

    bool            navEnable   = _p_checkboxNavEnable->GetValue();
    unsigned int    navSpeed    = Conversion::stringToUint( _p_textNavSpeed->GetValue().c_str() );
    osg::Vec3f      navPos      = Conversion::stringToVec3( _p_textNavPosition->GetValue().c_str() );
    osg::Vec2f      navRot      = Conversion::stringToVec2( _p_textNavRotation->GetValue().c_str() );;
    float           navFOV      = Conversion::stringToFloat( _p_textNavFOV->GetValue().c_str() );
    float           navNearClip = Conversion::stringToFloat( _p_textNavNearClip->GetValue().c_str() );
    float           navFarClip  = Conversion::stringToFloat( _p_textNavFarClip->GetValue().c_str() );
    osg::Vec3f      navBkgColor = Conversion::colorToVec( _p_textNavBkgColor->GetColour() );

    settings->setValue( ES_NAV_ENABLE,   navEnable );
    settings->setValue( ES_NAV_SPEED,    navSpeed );
    settings->setValue( ES_NAV_POSITION, navPos );
    settings->setValue( ES_NAV_ROTATION, navRot );
    settings->setValue( ES_NAV_FOV,      navFOV );
    settings->setValue( ES_NAV_NEARCLIP, navNearClip );
    settings->setValue( ES_NAV_FARCLIP,  navFarClip );
    settings->setValue( ES_NAV_BKGCOLOR, navBkgColor );

    // store the settings
    settings->store();
    // let the settings take place
    loadSettings();
}

void PanelNavigation::onButtonNavDefaultsClick( wxCommandEvent& event )
{
    yaf3d::SettingsPtr settings = yaf3d::SettingsManager::get()->getProfile( EDITOR_SETTINGS_PROFILE );
    assert( settings.valid() && "invalid editor configuration!" );

    unsigned int    navSpeed  = 100;
    osg::Vec3f      navPos;
    osg::Vec2f      navRot;
    float           navFOV      = 45.0f;
    float           navNearClip = 0.5f;
    float           navFarClip  = 1000.0f;
    osg::Vec3f      navBkgColor;

    settings->setValue( ES_NAV_SPEED,    navSpeed );
    settings->setValue( ES_NAV_POSITION, navPos );
    settings->setValue( ES_NAV_ROTATION, navRot );
    settings->setValue( ES_NAV_FOV,      navFOV );
    settings->setValue( ES_NAV_NEARCLIP, navNearClip );
    settings->setValue( ES_NAV_FARCLIP,  navFarClip );
    settings->setValue( ES_NAV_BKGCOLOR, navBkgColor );

    // let the settings take place
    loadSettings();
}
