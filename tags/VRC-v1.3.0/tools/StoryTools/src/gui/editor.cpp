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
 # wxWidget's main app
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "editor.h"
#include "mainframe.h"
#include "core/storage.h"
#include "core/rendermanager.h"
#include "core/elementfactory.h"

#include "core/elements/link.h"

IMPLEMENT_APP( beditor::EditorApp )
IMPLEMENT_CLASS( beditor::EditorApp, wxApp )

BEGIN_EVENT_TABLE( beditor::EditorApp, wxApp )
END_EVENT_TABLE()

namespace beditor
{

EditorApp::EditorApp()
{
    initialize();
}

void EditorApp::initialize()
{
    // currently nothing to do
}

//! TODO: remove this test later
void test()
{
#if 0
    try
    {
#if 0
        // create a top node in render manager
        BaseNodePtr story( new BaseNode( BaseNode::eTypeStory ) );
        story->setName( "TEST" );
        RenderManager::get()->setTopNode( story );

        BaseNodePtr eventnode1 = ElementFactory::get()->createNode( ELEM_TYPE_EVENT );
        BaseNodePtr eventnode2 = ElementFactory::get()->createNode( ELEM_TYPE_EVENT );

        eventnode1->setPosition( Eigen::Vector3f( 0.0f, 0.0f, 0.0f ) );
        eventnode1->setScale( Eigen::Vector3f( 1.0f, 1.0f, 1.0f ) );
        story->addChild( eventnode1.getRef() );

        eventnode2->setPosition( Eigen::Vector3f( 100.0f, 100.0f, 0.0f ) );
        eventnode2->setScale( Eigen::Vector3f( 2.0f, 2.0f, 1.0f ) );
        story->addChild( eventnode2.getRef() );

        // setup a link node
        BaseNodePtr link = ElementFactory::get()->createNode( ELEM_TYPE_LINK );
        NodeLink* p_nodelink = dynamic_cast< NodeLink* >( link.getRef() );
        p_nodelink->setSourceDestination( eventnode1, eventnode2 );
        story->addChild( p_nodelink );
#else
        std::vector< BaseNodePtr > stories;
        Storage::get()->read( "test.sc", stories );
        RenderManager::get()->setTopNode( stories[ 0 ] );

        Storage::get()->write( "C:\\out.sc", stories );

#endif
    }
    catch( const std::exception& e )
    {
        wxMessageBox( std::string( "Cannot create requested node type.\nReason:\n  " ) + e.what(), "Attention", wxOK );
        return;
    }
#endif
}

bool EditorApp::OnInit()
{
#if wxUSE_XPM
    wxImage::AddHandler( new wxXPMHandler );
#endif
#if wxUSE_LIBPNG
    wxImage::AddHandler( new wxPNGHandler );
#endif
#if wxUSE_LIBJPEG
    wxImage::AddHandler( new wxJPEGHandler );
#endif
#if wxUSE_GIF
    wxImage::AddHandler( new wxGIFHandler );
#endif

    // initialize the system
    beditor::Core::get()->initialize();

    // create the main window now
    beditor::MainFrame* p_mainWindow = new beditor::MainFrame( NULL );

    //! TODO: remove the test function later
    test();

    p_mainWindow->Show( true );

    return true;
}

int EditorApp::OnExit()
{
    // shutdown the system
    beditor::Core::get()->shutdown();

    return wxApp::OnExit();
}

} // namespace beditor
