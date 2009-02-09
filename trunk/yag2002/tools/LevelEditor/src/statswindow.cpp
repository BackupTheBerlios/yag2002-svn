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
 _p_textFPS( NULL ),
 _p_texCameraSpeed( NULL ),
 _p_textPosition( NULL ),
 _p_textRotation( NULL )
{
    wxDialog::Create( NULL, SYMBOL_STATSWINDOW_IDNAME, SYMBOL_STATSWINDOW_TITLE, SYMBOL_STATSWINDOW_POSITION, SYMBOL_STATSWINDOW_SIZE, SYMBOL_STATSWINDOW_STYLE );

    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );

    SetIcon( BitmapResource::get()->getIcon( EDITOR_RESID_ICON_STATSWND ) );

    wxBoxSizer* p_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer( p_sizer );

    wxPanel* itemPanel3 = new wxPanel( this, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL );
    p_sizer->Add(itemPanel3, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel3, wxID_STATIC, _("FPS"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText61 = new wxStaticText( itemPanel3, wxID_STATIC, _("Camera Speed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText61, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel3, wxID_STATIC, _("Position"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 6);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel3, wxID_STATIC, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL, 6);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer9, 0, wxALIGN_TOP|wxALL, 5);

    _p_textFPS = new wxStaticText( itemPanel3, ID_TEXT_FPS, _T(""), wxDefaultPosition, wxSize(160, -1), wxTE_READONLY );
    itemBoxSizer9->Add(_p_textFPS, 0, wxALIGN_LEFT|wxALL, 6);

    _p_texCameraSpeed = new wxStaticText( itemPanel3, ID_TEXT_CAMSPEED, _T(""), wxDefaultPosition, wxSize(160, -1), wxTE_READONLY );
    itemBoxSizer9->Add(_p_texCameraSpeed, 0, wxALIGN_LEFT|wxALL, 6);

    _p_textPosition = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_POS, _T(""), wxDefaultPosition, wxSize(160, -1), wxTE_READONLY );
    itemBoxSizer9->Add(_p_textPosition, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    _p_textRotation = new wxTextCtrl( itemPanel3, ID_TEXTCTRL_ROT, _T(""), wxDefaultPosition, wxSize(160, -1), wxTE_READONLY );
    itemBoxSizer9->Add(_p_textRotation, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    p_sizer->SetSizeHints( this );

    Move( 320, 0 );
}

StatsWindow::~StatsWindow()
{
}

void StatsWindow::setFPS( unsigned int fps )
{
    _p_textFPS->SetLabel( Conversion::uintToString( fps ) );
}

void StatsWindow::setCameraSpeed( float speed )
{
    _p_texCameraSpeed->SetLabel( Conversion::floatToString( speed ) );
}

void StatsWindow::setPosition( const osg::Vec3f& pos )
{
    _p_textPosition->SetValue( Conversion::vec3ToString( pos ).c_str() );
}

void StatsWindow::setPitchYaw( float pitch, float yaw )
{
    std::string text = Conversion::floatToString( pitch );
    text += " " + Conversion::floatToString( yaw );
    _p_textRotation->SetValue( text );
}
