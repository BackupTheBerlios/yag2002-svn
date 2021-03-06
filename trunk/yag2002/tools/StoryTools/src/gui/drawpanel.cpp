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
 # draw panel where the display and editing of the blocks happens
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "drawpanel.h"
#include <core/elements/link.h>

BEGIN_EVENT_TABLE( beditor::DrawPanel, wxPanel )
END_EVENT_TABLE()

namespace beditor
{

//! Periodic actions on panel go here
void DrawPanel::RefreshTimer::Notify()
{
    wxPaintEvent paintevent;
    _p_panel->onPaint( paintevent );
}

//! Implementation of the draw panel
DrawPanel::DrawPanel( wxWindow* p_parent ) :
 wxPanel( p_parent, ID_DRAW_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER ),
 _p_timer( NULL ),
 _zoom( 1.0f ),
 _editState( 0 ),
 _p_canvas( NULL )
{
    createControls();
}

DrawPanel::~DrawPanel()
{
    if ( _p_timer )
        delete _p_timer;
}

void DrawPanel::setEditMode( unsigned int mode )
{
    _editState &= 0x00ff;
    _editState |= mode;

    // prepare the create link mode
    if ( mode & eCreateLink )
        _linkNode = NULL;
}

void DrawPanel::resetZoom()
{
    _zoom = 1.0f;
}

void DrawPanel::forceRedraw()
{
    wxPaintEvent paintevent;
    onPaint( paintevent );
}

void DrawPanel::createControls()
{
    wxBoxSizer* p_canvassizer = new wxBoxSizer( wxVERTICAL );
    _p_canvas = new wxGLCanvas( this, ID_DRAW_CANVAS, wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    p_canvassizer->Add( _p_canvas, 1, wxGROW | wxALL, 0 );
    SetSizer( p_canvassizer );

    // setup the canvas in renderer
    RenderManager::get()->setGLCanvas( _p_canvas );

    // setup a timer for periodic display redraws
    _p_timer = new DrawPanel::RefreshTimer( this, 30 ); // update every 30 milliseconds

    // connect events
    _p_canvas->Connect( wxID_ANY, wxEVT_SIZE,         wxSizeEventHandler( DrawPanel::onSize ),   NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_PAINT,        wxPaintEventHandler( DrawPanel::onPaint ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_LEFT_DOWN,    wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_LEFT_UP,      wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_RIGHT_DOWN,   wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_RIGHT_UP,     wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_MOUSEWHEEL,   wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_MOTION,       wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_ENTER_WINDOW, wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_LEAVE_WINDOW, wxMouseEventHandler( DrawPanel::onMouse ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_SET_FOCUS,    wxFocusEventHandler( DrawPanel::onSetFocus ),  NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_KILL_FOCUS,   wxFocusEventHandler( DrawPanel::onKillFocus ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_KEY_DOWN,     wxKeyEventHandler( DrawPanel::onKey ), NULL, this );
    _p_canvas->Connect( wxID_ANY, wxEVT_KEY_UP,       wxKeyEventHandler( DrawPanel::onKey ), NULL, this );
}

void DrawPanel::onSize( wxSizeEvent& event )
{
    wxPaintEvent paintevent;
    onPaint( paintevent );
    event.Skip();
}

void DrawPanel::onKey( wxKeyEvent& event )
{
    bool forceredraw = false;

    int keycode = event.GetKeyCode() ;

    if ( event.ShiftDown() )
    {
        if ( !( _editState & eCreateLink ) )
        {
            _editState |= eStateMultiSel;
        }
    }
    else
    {
        _editState &= ~eStateMultiSel;
    }

    if ( ( keycode == WXK_DELETE ) && !( _editState & eCreateLink ) )
    {
        RenderManager::get()->deleteNodes( _selNodes );
        forceredraw = true;
    }

    if ( forceredraw )
    {
        wxPaintEvent paintevent;
        onPaint( paintevent );
    }
}

void DrawPanel::onPaint( wxPaintEvent& event )
{
    _zoom = 1.0f;

    wxSize winsize = GetClientSize();
    int panx = _pan.x;
    int pany = -_pan.y;

    // render scene
    RenderManager::get()->renderScene( wxPoint( panx , pany ), wxSize( winsize.x, winsize.y ) );

    event.Skip();
}

void DrawPanel::onMouse( wxMouseEvent& event )
{
    // some interactions need a panel redraw
    bool forceredraw = false;

    // exit any edit mode on right click
    if ( event.RightUp() )
        setEditMode( 0 );

    if ( event.RightUp() || event.Leaving() )
    {
        _editState &= ~eStatePanning;
    }
    else if ( event.RightIsDown() )
    {
        _editState |= eStatePanning;
    }

    if ( _editState & eStatePanning )
    {
        _pan.x += event.GetX() - _lastMousePosition.x;
        _pan.y += event.GetY() - _lastMousePosition.y;

        forceredraw = true;
    }

    //! TODO: support for zooming
    if ( event.GetWheelDelta() )
    {
        _zoom = ( event.GetWheelDelta() > 0 ) ? _zoom + 0.1f : _zoom - 0.1f;
    }

    // select nodes by hit test
    if ( event.LeftDown() )
    {
        // check for multi-selection
        if ( !( _editState & eStateMultiSel ) )
        {
            // clear selection
            _selNodes.clear();
        }

        // transform the screen coords to world coords
        wxPoint         hitpoint( event.GetPosition() );
        wxSize          winsize = GetClientSize();
        Eigen::Vector3f hitposition;

        hitposition.x() = -_pan.x + ( float( hitpoint.x ) - float( winsize.x ) * 0.5f );
        hitposition.y() = _pan.y - ( float( hitpoint.y ) - float( winsize.y ) * 0.5f );
        hitposition.z() = 0.0f;

        RenderManager::get()->selectNodesByHit( hitposition, _selNodes );

        // highlight colour in create link mode is different
        Eigen::Vector3f highlightcolour( 1.0f, 0.0f, 0.0f );
        if ( _editState & eCreateLink )
        {
            highlightcolour = Eigen::Vector3f( 0.0f, 0.0f, 1.0f );
            if ( ( _selNodes.size() > 0 ) && !_linkNode.getRef() )
            {
                std::vector< BaseNodePtr > linksrcnode;
                linksrcnode.push_back( _selNodes[ 0 ] );
                RenderManager::get()->highlightNodes( linksrcnode, highlightcolour );
            }
        }
        else
        {
            RenderManager::get()->highlightNodes( _selNodes, highlightcolour );
        }

        if ( _editState & eCreateLink )
        {
            if ( _selNodes.size() > 0 )
            {
                if ( !_linkNode.getRef() )
                {
                    _linkNode = _selNodes[ 0 ];
                }
                else
                {
                    // create a node link
                    BaseNodePtr linknode = ElementFactory::get()->createNode( ELEM_TYPE_LINK );
                    assert( linknode.getRef() );
                    NodeLink* p_linknode = dynamic_cast< NodeLink* >( linknode.getRef() );
                    p_linknode->setName( "" );
                    p_linknode->setSourceDestination( _linkNode, _selNodes[ 0 ] );
                    
                    // append link node to story
                    RenderManager::get()->getTopNode()->addChild( p_linknode );

                    _selNodes.clear();
                    RenderManager::get()->highlightNodes( _selNodes );

                    _linkNode = NULL;
                }
            }
        }

        forceredraw = true;
    }

    // drag selected nodes
    if ( ( _selNodes.size() > 0 ) && event.Dragging() && event.ButtonIsDown( wxMOUSE_BTN_LEFT ) )
    {
        std::vector< BaseNodePtr >::iterator p_node = _selNodes.begin(), p_end = _selNodes.end();
        for ( ; p_node != p_end; ++p_node )
        {
            Eigen::Vector3f pos = ( *p_node )->getPosition();
            pos.x() += event.GetPosition().x - _lastMousePosition.x;
            pos.y() -= event.GetPosition().y - _lastMousePosition.y;
            ( *p_node )->setPosition( pos );
        }

        forceredraw = true;
    }

    // update the last mouse position
    _lastMousePosition = event.GetPosition();

    if ( forceredraw )
    {
        wxPaintEvent paintevent;
        onPaint( paintevent );
    }

    event.Skip();
}

void DrawPanel::onSetFocus( wxFocusEvent& event )
{
}

void DrawPanel::onKillFocus( wxFocusEvent& event )
{
}

} // namespace beditor
