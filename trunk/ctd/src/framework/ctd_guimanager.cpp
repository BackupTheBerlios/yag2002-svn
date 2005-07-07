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
#include "ctd_keymap.h"
#include "ctd_log.h"
#include "ctd_guirenderer.h"

#include <osg/Projection>

using namespace std;
using namespace CEGUI;

namespace CTD
{

//! Viewer's realize callback. Here we initialize CEGUI's renderer.
class GuiViewerRealizeCallback : public Producer::RenderSurface::Callback
{
    public:
       
        virtual void                                operator()( const Producer::RenderSurface & )
                                                    {
                                                        GuiManager::get()->doInitialize();
                                                    }

    protected:

        virtual                                     ~GuiViewerRealizeCallback() {}

};

//! Post-render callback, here the complete gui is initialized and drawn
class GuiRenderCallback : public Producer::Camera::Callback
{
    public:

       virtual void                                 operator()( const Producer::Camera & ) 
                                                    {
                                                        GuiManager::get()->doRender();
                                                    }

    protected:

        virtual                                     ~GuiRenderCallback(){}
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
        log << Log::LogLevel( Log::L_WARNING ) << " CTDResourceProvider: file '" << fullpath << "' does not exist." << endl;
        throw CEGUI::Exception( "CTDResourceProvider cannot find file '" + fullpath + "'" );
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
_p_mouseImg( NULL ),
_windowWidth( 600 ),
_windowHeight( 400 ),
_p_root( NULL ),
_active( true ),
_lockMouse( false )
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

    // register a viewer realize callback, here we initialize CEGUI's renderer (using doInitialize)
    _guiRealizeCallback = new GuiViewerRealizeCallback;
    Producer::RenderSurface* p_rs = p_cam->getRenderSurface();
    p_rs->addRealizeCallback( _guiRealizeCallback.get() );
}

void GuiManager::doInitialize()
{
    // first we look if we are in windowed or fullscreen mode
    bool fullscreen;
    Configuration::get()->getSettingValue( CTD_GS_FULLSCREEN,    fullscreen );
    if ( fullscreen )
    {
        Producer::Camera*        p_cam = Application::get()->getViewer()->getCamera( 0 );
        Producer::RenderSurface* p_rs  = p_cam->getRenderSurface();
        unsigned int x, y;
        int dummy; 
        p_rs->getWindowRectangle( dummy, dummy, x, y );
        _windowWidth = float( x );
        _windowHeight = float( y );
    }
    else
    {
        unsigned int width, height;
        Configuration::get()->getSettingValue( CTD_GS_SCREENWIDTH,  width  );
        Configuration::get()->getSettingValue( CTD_GS_SCREENHEIGHT, height );
        _windowWidth = float( width );
        _windowHeight = float( height );
    }

    // create a renderer
    _p_renderer = new CTDGuiRenderer( 0, _windowWidth, _windowHeight );    
    // create the gui
    CTDResourceProvider* p_resourceLoader = new CTDResourceProvider;
    new CEGUI::System( _p_renderer, p_resourceLoader );

    string guiScheme;
    Configuration::get()->getSettingValue( CTD_GS_GUISCHEME, guiScheme );

    CEGUI::Imageset* p_taharezImages = ImagesetManager::getSingleton().createImageset( "gui/imagesets/TaharezLook.imageset" );

    _p_mouseImg = const_cast< CEGUI::Image* >( &p_taharezImages->getImage( "MouseArrow" ) );
    CEGUI::System::getSingleton().setDefaultMouseCursor( _p_mouseImg );

    // create necessary fonts
    CEGUI::Font* p_font = NULL;
    p_font = createFont( string( "gui/fonts/" CTD_GUI_FONT8 ".font" ) );
    p_font = createFont( string( "gui/fonts/" CTD_GUI_FONT10 ".font" ) );
    CEGUI::System::getSingleton().setDefaultFont( p_font ); // set the default font
    p_font = createFont( string( "gui/fonts/" CTD_GUI_CONSOLE ".font" ) );

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

CEGUI::Font* GuiManager::createFont( const std::string& fontname )
{
    CEGUI::Font* p_font = NULL;

    try 
    {
        // load font
        p_font = FontManager::getSingleton().createFont( fontname );
        // add german specific glyphs to font
        CEGUI::String glyphs = p_font->getAvailableGlyphs();
        CEGUI::String germanGlyphs;
        germanGlyphs += ( CEGUI::utf8 )223; // s-zet
        germanGlyphs += ( CEGUI::utf8 )228; // ae
        germanGlyphs += ( CEGUI::utf8 )246; // oe
        germanGlyphs += ( CEGUI::utf8 )252; // ue
        germanGlyphs += ( CEGUI::utf8 )196; // Ae
        germanGlyphs += ( CEGUI::utf8 )214; // Oe
        germanGlyphs += ( CEGUI::utf8 )220; // Ue
        p_font->defineFontGlyphs( glyphs + germanGlyphs );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "GuiManager: cannot create font: '" << fontname << "'" << endl;
        log << Log::LogLevel( Log::L_ERROR ) << " reason: " << e.getMessage().c_str() << "'" << endl;
        return NULL;
    }

    // register the font via its name
    _loadedFonts[ p_font->getName().c_str() ] = p_font;
    return p_font;
}


CEGUI::Font* GuiManager::getFont( const std::string& fontname )
{
    std::map< std::string, CEGUI::Font* >::iterator p_font = _loadedFonts.find( fontname ), p_end = _loadedFonts.end();
    if ( p_font != p_end )
        return p_font->second;

    log << Log::LogLevel( Log::L_ERROR ) << " GuiManager: no font with name '" << fontname << "' exists!" << endl;
    return NULL;
}

CEGUI::Font* GuiManager::loadFont( const std::string& descriptionfile )
{
    return createFont( descriptionfile );
}

void GuiManager::changeDisplayResolution( float width, float height )
{
    assert( _p_root && " gui system is not initialized!" );
    _windowWidth  = width;
    _windowHeight = height;
    _p_root->setMaximumSize( CEGUI::Size( _windowWidth, _windowHeight ) );
    _p_root->setSize( CEGUI::Size( _windowWidth, _windowHeight ) );
    _p_renderer->changeDisplayResolution( _windowWidth, _windowHeight );

    CEGUI::FontManager::getSingleton().notifyScreenResolution( CEGUI::Size( _windowWidth, _windowHeight ) );
	CEGUI::ImagesetManager::getSingleton().notifyScreenResolution( CEGUI::Size( _windowWidth, _windowHeight ) );
    CEGUI::Rect rect( 0, 0, _windowWidth, _windowHeight );
    CEGUI::MouseCursor::getSingleton().setConstraintArea( &rect );
}

void GuiManager::showMousePointer( bool show ) const
{    
    if ( show )
        MouseCursor::getSingleton().show();
    else
        MouseCursor::getSingleton().hide();
}

void GuiManager::lockPointer( float x, float y )
{
    _lockMouse = true;
    // set the fix position
    // we need absolute mouse coords for CEGUI
    x = ( 0.5f * x + 0.5f ) * _windowWidth;
    y = ( 0.5f * y + 0.5f ) * _windowHeight;
    CEGUI::System::getSingleton().injectMousePosition( x, y );
}

void GuiManager::releasePointer()
{
    _lockMouse = false;
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

    Window* p_layout = NULL;
    try
    {
        p_layout = WindowManager::getSingleton().loadWindowLayout( filename.c_str(), pref.str() );

        if ( !p_parent )
            _p_root->addChildWindow( p_layout );
        else
            p_parent->addChildWindow( p_layout );

    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "GuiManager: cannot load layout: '" << filename << "'" << endl;
        log << Log::LogLevel( Log::L_ERROR ) << " reason: " << e.getMessage().c_str() << "'" << endl;
        return NULL;
    }

    p_layout->show();

    return p_layout;
}

void GuiManager::getGuiArea( float& width, float& height ) const
{
    width  = _p_renderer->getWidth();
    height = _p_renderer->getHeight();
}

void GuiManager::activate( bool active )
{
    _active = active;
}

bool GuiManager::isActive() const
{
    return _active;
}

CTDGuiRenderer* GuiManager::getGuiRenderer()
{
    return _p_renderer;
}

CEGUI::DefaultWindow* GuiManager::getRootWindow()
{
    assert( _p_root && " gui system is not initialized!" );
    return _p_root;
}

void GuiManager::update( float deltaTime )
{
    _p_root->update( deltaTime );
}

void GuiManager::doRender()
{
    if ( _active )
        CEGUI::System::getSingleton().renderGUI();
}

bool GuiManager::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    if ( !_p_guiMgr->_active )
        return false;

    unsigned int eventType    = ea.getEventType();
    int          key          = ea.getKey();
 
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

            default:
                ;
        }

        unsigned int transkey = KeyMap::get()->translateKey( key );

        // always inject Character even if we have done key-down injection
        CEGUI::System::getSingleton().injectChar( static_cast< CEGUI::utf32 >( transkey ) );    
    }


    // handle mouse
    unsigned int buttonMask = ea.getButtonMask();
    static bool  lbtn_down  = false;
    static bool  lbtn_up    = true;
    static bool  rbtn_down  = false;
    static bool  rbtn_up    = true;
    static bool  mbtn_down  = false;
    static bool  mbtn_up    = true;

    // left mouse button
    if ( buttonMask == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
    {
        if ( !lbtn_down )
        {
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::LeftButton );
            lbtn_down = true;
            lbtn_up   = false;
        } 
    }
    else
    {
        if ( !lbtn_up )
        {
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::LeftButton );
            lbtn_up   = true;
            lbtn_down = false;
        }
    }

    // right mouse button
    if ( buttonMask == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {
        if ( !rbtn_down )
        {
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::RightButton );
            rbtn_down = true;
            rbtn_up   = false;
        }
    }
    else
    {
        if ( !rbtn_up )
        {
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::RightButton );
            rbtn_down = false;
            rbtn_up   = true;
        }
    }
    
    // middle mouse button
    if ( buttonMask == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON )
    {
        if ( !mbtn_down )
        {
            CEGUI::System::getSingleton().injectMouseButtonDown( CEGUI::MiddleButton );
            mbtn_down = true;
            mbtn_up   = true;
        }
    }
    else
    {
        if ( !mbtn_up )
        {
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::MiddleButton );
            mbtn_down = false;
            mbtn_up   = false;
        }
    }

    if ( !_p_guiMgr->_lockMouse )
    {

        // adjust the pointer position
        float x = ea.getX();
        float y = -ea.getY();
        // we need absolute mouse coords for CEGUI
        x = ( 0.5f * x + 0.5f ) * _p_guiMgr->_windowWidth;
        y = ( 0.5f * y + 0.5f ) * _p_guiMgr->_windowHeight;
        CEGUI::System::getSingleton().injectMousePosition( x, y );
    }

    return false;
}

}
