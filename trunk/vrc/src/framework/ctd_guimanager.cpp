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
 # graphical user interface
 #
 #   date of creation:  04/01/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_guimanager.h"
#include "ctd_application.h"
#include "ctd_configuration.h"
#include "ctd_log.h"
#include "ctd_guirenderer.h"

#include <osg/Projection>

using namespace std;
using namespace CEGUI;

namespace CTD
{


//! Post-render callback, here the complete gui is initialized and drawn
class GuiRenderCallback : public Producer::Camera::Callback
{
    public:
                                                    GuiRenderCallback() : _initialized( false ) {}

       virtual void                                 operator()( const Producer::Camera & ) 
                                                    {
                                                        if ( !_initialized )
                                                        {
                                                            GuiManager::get()->doInitialize();
                                                            _initialized = true;
                                                        }
                                                        CEGUI::System::getSingleton().renderGUI();
                                                    }

        virtual                                     ~GuiRenderCallback(){}

    protected:

        bool                                        _initialized;
};

//! Resource loader for gui resource loading
class CTDResourceProvider : public ResourceProvider
{
    public:
                                                    CTDResourceProvider() : ResourceProvider() {}

                                                    ~CTDResourceProvider() {}

        void                                        loadRawDataContainer( const CEGUI::String& filename, RawDataContainer& output, const CEGUI::String& resourceGroup );

};
GuiManager::InputHandler::InputHandler( GuiManager* p_guimgr ) : 
_p_guiMgr( p_guimgr )
{
    // register us in viewer to get mouse and keyboard event callbacks
    osg::ref_ptr< GuiManager::InputHandler > ih( this );
    Application::get()->getViewer()->getEventHandlerList().push_back( ih.get() );
}

void CTDResourceProvider::loadRawDataContainer( const CEGUI::String& filename, RawDataContainer& output, const CEGUI::String& resourceGroup )
{
    auto_ptr< fstream > p_stream( new fstream );

    string fullpath = Application::get()->getMediaPath() + filename.c_str();
    p_stream->open( fullpath.c_str(), std::ios_base::binary | std::ios_base::in );
    // if the file does not exist then return
    if ( !( *p_stream ) )
    {   
        log << Log::LogLevel( Log::L_ERROR ) << "*** CTDResourceProvider: file '" << fullpath << "' does not exist." << endl;
        return;
    }

    // get file size
    p_stream->seekg( 0, ios_base::end );
    int filesize = ( int )p_stream->tellg();
    p_stream->seekg( 0, ios_base::beg );

    // load the file
    unsigned char* p_buf = new unsigned char[ filesize * sizeof( unsigned char ) + 2 ];
    p_stream->read( ( char* )p_buf, filesize );

    // set output
    output.setData( p_buf );
    output.setSize( filesize );

    p_stream->close();
}

CTD_SINGLETON_IMPL( GuiManager );

// implementation of GuiManager
GuiManager::GuiManager() :
_p_renderer( NULL ),
_windowWidth( 600 ),
_windowHeight( 400 ),
_p_root( NULL )
{
}

GuiManager::~GuiManager()
{
}

void GuiManager::shutdown()
{
    
    // remove input handler from viewer's handler list
    osgProducer::Viewer::EventHandlerList& eh = Application::get()->getViewer()->getEventHandlerList();
    osgProducer::Viewer::EventHandlerList::iterator beg = eh.begin(), end = eh.end();
    for ( ; beg != end; beg++ )
    {
        if ( *beg == _inputHandler.get() )
        {
            eh.erase( beg );
            break;
        }
    }

    delete CEGUI::System::getSingletonPtr();
    delete _p_renderer;

    // destroy singleton
    destroy();    
}

void GuiManager::initialize()
{
    // register the post render callback where also the CEGUI initialization happens
    osgProducer::Viewer* p_viewer = Application::get()->getViewer();
    Producer::Camera* p_cam = p_viewer->getCamera( 0 );
    _guiRenderCallback = new GuiRenderCallback;
    p_cam->addPostDrawCallback( _guiRenderCallback.get() );
}

void GuiManager::doInitialize()
{
    unsigned int width, height;
    Configuration::get()->getSettingValue( CTD_GS_SCREENWIDTH,  width );
    Configuration::get()->getSettingValue( CTD_GS_SCREENHEIGHT, height );
    _windowWidth = float( width );
    _windowHeight = float( height );

    // create a renderer
    _p_renderer = new CTDGuiRenderer( 0, width, height );    
    // create the gui
    CTDResourceProvider *p_resLoader = new CTDResourceProvider;
    new CEGUI::System( _p_renderer, p_resLoader );

    string guiScheme;
    Configuration::get()->getSettingValue( CTD_GS_GUISCHEME, guiScheme );

    CEGUI::Imageset* p_taharezImages = ImagesetManager::getSingleton().createImageset( "gui/imagesets/TaharezLook.imageset" );

    System::getSingleton().setDefaultMouseCursor( &p_taharezImages->getImage( "MouseArrow" ) );

    // load font
    FontManager::getSingleton().createFont( string( "gui/fonts/" CTD_GUI_FONT ".font" ) );

    // load scheme
    SchemeManager::getSingleton().loadScheme( string( "gui/schemes/" CTD_GUI_SCHEME ".scheme" ) );

    // create the root window called 'Root'.
    _p_root = static_cast< DefaultWindow* >( WindowManager::getSingleton().createWindow("DefaultWindow", "Root") );
    _p_root->setMetricsMode( CEGUI::Absolute );
    _p_root->setPosition( CEGUI::Point( 0, 0 ) );
    _p_root->setSize( CEGUI::Size( _windowWidth, _windowHeight ) );

    // set the GUI root window (also known as the GUI "sheet"), all layout will be added to this root for showing up
    System::getSingleton().setGUISheet( _p_root );

    // create input handler
    _inputHandler = new InputHandler( this );
}

CEGUI::Window* GuiManager::loadLayout( const string& filename, CEGUI::Window* p_parent, const string& handle )
{
    assert( _p_root && " gui system is not initialized!" );

    stringstream pref;

    if ( !handle.length() )
    {
        // create an auto-incremented prefix, so we can load several layouts
        static int prefix = 0;
        pref << "l" << prefix << "_";
        prefix++;
    }
    else
        pref << handle;

    Window* p_layout = WindowManager::getSingleton().loadWindowLayout( filename.c_str(), pref.str() );

    if ( !p_parent )
        _p_root->addChildWindow( p_layout );
    else
        p_parent->addChildWindow( p_layout );

    p_layout->show();

    return p_layout;
}

void GuiManager::getGuiArea( float& width, float& height )
{
    width  = _p_renderer->getWidth();
    height = _p_renderer->getHeight();
}

void GuiManager::update( float deltaTime )
{
    _p_root->update( deltaTime );
}

bool GuiManager::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    bool         ret         = false;
    unsigned int eventType   = ea.getEventType();
    int          key         = ea.getKey();

    // dispatch key down activity
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        // some keys may be handled via key code and generate those too
        switch ( key )
        {
            case osgGA::GUIEventAdapter::KEY_BackSpace:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Backspace);
                break;

            case osgGA::GUIEventAdapter::KEY_Delete:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Delete);
                break;

            case osgGA::GUIEventAdapter::KEY_Escape:
                //CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Escape);
                break;

            case osgGA::GUIEventAdapter::KEY_Left:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowLeft);
                break;

            case osgGA::GUIEventAdapter::KEY_Right:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowRight);
                break;

            case osgGA::GUIEventAdapter::KEY_Up:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowUp);
                break;

            case osgGA::GUIEventAdapter::KEY_Down:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowDown);
                break;

            case osgGA::GUIEventAdapter::KEY_Home:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Home);
                break;

            case osgGA::GUIEventAdapter::KEY_End:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::End);
                break;

            case osgGA::GUIEventAdapter::KEY_Page_Up:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::PageUp);
                break;

            case osgGA::GUIEventAdapter::KEY_Page_Down:
                CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::PageDown);
                break;
        }    
    }

    // always inject Character even if we have done key-down injection
    CEGUI::System::getSingleton().injectChar( static_cast< CEGUI::utf32 >( key ) );

    // handle mouse
    unsigned int buttonMask = ea.getButtonMask();
    static bool  lbtn_down  = false;
    static bool  rbtn_down  = false;
    static bool  mbtn_down  = false;

    // left mouse button
    if ( buttonMask == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
    {
        if ( !lbtn_down )
        {
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::LeftButton );
            lbtn_down = true;
        }
    }
    else
    {
        CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::LeftButton );
        lbtn_down = false;
    }

    // right mouse button
    if ( buttonMask == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {
        if ( !rbtn_down )
        {
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::RightButton );
            rbtn_down = true;
        }
    }
    else
    {
        CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::RightButton );
        rbtn_down = false;
    }
    
    // middle mouse button
    if ( buttonMask == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON )
    {
        if ( !mbtn_down )
        {
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::MiddleButton );
            mbtn_down = true;
        }
    }
    else
    {
        CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::MiddleButton );
        mbtn_down = false;
    }

    // adjust the pointer position
    float x = ea.getX();
    float y = -ea.getY();
    // we need absolute mouse coords in range [ 0..1 ] for CEGUI
    x = ( 0.5f * x + 0.5f ) * _p_guiMgr->_windowWidth;
    y = ( 0.5f * y + 0.5f ) * _p_guiMgr->_windowHeight;
    CEGUI::System::getSingleton().injectMousePosition( x, y );

    //!TODO: mouse scroll handling

    //!TODO: app window resising and fullscreen toggling does not work as we get no RESIZE event. we have to hook into os
    // handle app's window reshaping
    //if ( eventType == osgGA::GUIEventAdapter::RESIZE )
    //{
        //Producer::Camera *p_cam = Application::get()->getViewer()->getCamera(0);
        //Producer::RenderSurface* p_rs = p_cam->getRenderSurface();    
        //_p_guiMgr->_p_renderer->changeDisplayResolution( float( p_rs->getWindowWidth() ), float( p_rs->getWindowHeight() ) );
    //}
  
     return ret;
}

}
