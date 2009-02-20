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
 # editor's stats window
 #
 #   date of creation:  02/03/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <vrc_main.h>
#include "statswindow.h"
#include "editorutils.h"


IMPLEMENT_DYNAMIC_CLASS( StatsWindow, wxDialog )

BEGIN_EVENT_TABLE( StatsWindow, wxDialog )

END_EVENT_TABLE()


StatsWindow::StatsWindow() :
 _p_textCamSpeed( NULL ),
 _p_textCamPosition( NULL ),
 _p_textCamRotation( NULL ),
 _p_sizerInspector( NULL ),
 _p_textInspPosition( NULL ),
 _p_textInspNormal( NULL ),
 _p_textInspObject( NULL ),
 _currSpeed( 0.0f )
{
    wxDialog::Create( NULL, SYMBOL_STATSWINDOW_IDNAME, SYMBOL_STATSWINDOW_TITLE, SYMBOL_STATSWINDOW_POSITION, SYMBOL_STATSWINDOW_SIZE, SYMBOL_STATSWINDOW_STYLE );

    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );

    SetIcon( BitmapResource::get()->getIcon( EDITOR_RESID_ICON_STATSWND ) );

    createControls();

    if ( GetSizer() )
        GetSizer()->SetSizeHints( this );

    Move( 320, 0 );
}

StatsWindow::~StatsWindow()
{
}

void StatsWindow::createControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( this, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel3, wxID_ANY, _("Camera"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxHORIZONTAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer6, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_TOP|wxRIGHT|wxBOTTOM, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel3, wxID_STATIC, _("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel3, wxID_STATIC, _("Position"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel3, wxID_STATIC, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALL, 6);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer11, 1, wxGROW|wxLEFT, 5);

    _p_textCamSpeed = new wxStaticText( itemPanel3, ID_TEXT_CAM_SPEED, _("100"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(_p_textCamSpeed, 0, wxGROW|wxRIGHT|wxTOP|wxBOTTOM, 6);

    _p_textCamPosition = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_CAM_POS, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemBoxSizer11->Add(_p_textCamPosition, 0, wxGROW, 5);

    _p_textCamRotation = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_CAM_ROT, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemBoxSizer11->Add(_p_textCamRotation, 0, wxGROW|wxTOP, 5);

    _p_sizerInspector = new wxStaticBox(itemPanel3, wxID_ANY, _("Inspector"));
    wxStaticBoxSizer* itemStaticBoxSizer15 = new wxStaticBoxSizer(_p_sizerInspector, wxHORIZONTAL);
    itemBoxSizer4->Add(itemStaticBoxSizer15, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer15->Add(itemBoxSizer16, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer16->Add(itemBoxSizer17, 0, wxALIGN_TOP|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel3, wxID_STATIC, _("Position"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText19 = new wxStaticText( itemPanel3, wxID_STATIC, _("Normal"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel3, wxID_STATIC, _("Object"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALL, 6);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer16->Add(itemBoxSizer21, 1, wxGROW|wxLEFT|wxTOP, 5);

    _p_textInspPosition = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_INSP_POS, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemBoxSizer21->Add(_p_textInspPosition, 0, wxGROW, 5);

    _p_textInspNormal = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_INSP_NORMAL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemBoxSizer21->Add(_p_textInspNormal, 0, wxGROW|wxTOP, 5);

    _p_textInspObject = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_INSP_OBJECT, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemBoxSizer21->Add(_p_textInspObject, 0, wxGROW|wxTOP, 5);
}

void StatsWindow::setFPS( unsigned int fps )
{
    std::string title( SYMBOL_STATSWINDOW_TITLE );
    title += " (" + Conversion::uintToString( fps ) + " fps)";
    SetTitle( title );
}

void StatsWindow::setCameraSpeed( float speed )
{
    // avoid flickering the speed display
    if ( fabs( _currSpeed - speed ) < 0.1f )
        return;

    _currSpeed = speed;
    _p_textCamSpeed->SetLabel( Conversion::floatToString( speed ) );
}

void StatsWindow::setCameraPosition( const osg::Vec3f& pos )
{
    _p_textCamPosition->SetValue( Conversion::vec3ToString( pos, 1 ).c_str() );
}

void StatsWindow::setCameraPitchYaw( float pitch, float yaw )
{
    std::string text = Conversion::floatToString( pitch );
    text += " " + Conversion::floatToString( yaw );
    _p_textCamRotation->SetValue( text );
}

void StatsWindow::enableInspectorDisplay( bool en )
{
    _p_sizerInspector->Enable( en );
}

void StatsWindow::setInspectorPosition( const osg::Vec3f& pos )
{
    _p_textInspPosition->SetValue( Conversion::vec3ToString( pos, 1 ).c_str() );
}

void StatsWindow::setInspectorNormal( const osg::Vec3f& norm )
{
    _p_textInspNormal->SetValue( Conversion::vec3ToString( norm, 1 ).c_str() );
}

void StatsWindow::setInspectorObject( const std::string& object )
{
    _p_textInspObject->SetValue( object.c_str() );
}
