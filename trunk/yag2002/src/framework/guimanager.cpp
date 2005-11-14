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

#include <base.h>
#include "guimanager.h"
#include "application.h"
#include "configuration.h"
#include "guirenderer.h"
#include "keymap.h"
#include "log.h"
#include <CEGUILogger.h>


namespace yaf3d
{

//! Viewer's init callback. Here we initialize CEGUI's renderer.
class GuiViewerInitCallback : public osgSDL::Viewer::ViewerInitCallback
{
    public:
       
        virtual void                                operator()( const osgSDL::Viewport* p_vp )
                                                    {
                                                        GuiManager::get()->doInitialize();
                                                    }

    protected:

        virtual                                     ~GuiViewerInitCallback() {}

};

//! Post-render callback, here the complete gui is initialized and drawn
class GuiPostDrawCallback : public osgSDL::Viewer::DrawCallback
{
    public:

       virtual void                                 operator()( const osgSDL::Viewport* p_vp ) 
                                                    {
                                                        GuiManager::get()->doRender();
                                                    }

    protected:

        virtual                                     ~GuiPostDrawCallback(){}
};

//! Resource loader for gui resource loading
class CTDResourceProvider : public CEGUI::ResourceProvider
{
    public:
                                                    CTDResourceProvider() : ResourceProvider() {}

                                                    ~CTDResourceProvider() {}

        void                                        loadRawDataContainer( const CEGUI::String& filename, CEGUI::RawDataContainer& output, const CEGUI::String& resourceGroup );

};
GuiManager::InputHandler::InputHandler( GuiManager* p_guimgr ) : 
GenericInputHandler< GuiManager >( p_guimgr )
{
}

void CTDResourceProvider::loadRawDataContainer( const CEGUI::String& filename, CEGUI::RawDataContainer& output, const CEGUI::String& resourceGroup )
{
    std::auto_ptr< std::fstream > p_stream( new std::fstream );

    std::string fullpath = Application::get()->getMediaPath() + filename.c_str();
    p_stream->open( fullpath.c_str(), std::ios_base::binary | std::ios_base::in );
    // if the file does not exist then return
    if ( !( *p_stream ) )
    {   
        log << Log::LogLevel( Log::L_WARNING ) << " CTDResourceProvider: file '" << fullpath << "' does not exist." << std::endl;
        throw CEGUI::Exception( "CTDResourceProvider cannot find file '" + fullpath + "'" );
    }

    // get file size
    p_stream->seekg( 0, std::ios_base::end );
    int filesize = ( int )p_stream->tellg();
    p_stream->seekg( 0, std::ios_base::beg );

    // load the file
    unsigned char* p_buf = new unsigned char[ filesize + 1 ];
    p_buf[ filesize ] = 0;
    p_stream->read( ( char* )p_buf, filesize );
 
    // set output
    output.setData( p_buf );
    output.setSize( filesize );

    p_stream->close();
}


YAF3D_SINGLETON_IMPL( GuiManager );

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
    delete CEGUI::System::getSingletonPtr();
    delete _p_renderer;

    // destroy singleton
    destroy();    
}

void GuiManager::initialize()
{
    // register the post render callback where also the CEGUI initialization happens
    osgSDL::Viewer* p_viewer = Application::get()->getViewer();
    _guiPostDrawCallback = new GuiPostDrawCallback;
    p_viewer->addPostDrawCallback( _guiPostDrawCallback.get() );

    // register a viewer realize callback, here we initialize CEGUI's renderer (using doInitialize)
    _guiViewerInitCallback = new GuiViewerInitCallback;
    p_viewer->addViewerInitCallback( _guiViewerInitCallback.get() );
}

void GuiManager::doInitialize()
{
    // get window size
    unsigned int width, height;
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENWIDTH,  width  );
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENHEIGHT, height );
    _windowWidth = float( width );
    _windowHeight = float( height );

    // create a renderer
    _p_renderer = new CTDGuiRenderer( 0, int( _windowWidth ), int( _windowHeight ) );    
    // create the gui
    CTDResourceProvider* p_resourceLoader = new CTDResourceProvider;
    new CEGUI::System( _p_renderer, p_resourceLoader );

    // set logging level
//#ifdef _DEBUG
    CEGUI::Logger::getSingleton().setLoggingLevel( CEGUI::Insane );
//#endif

    std::string guiScheme;
    Configuration::get()->getSettingValue( YAF3D_GS_GUISCHEME, guiScheme );

    CEGUI::Imageset* p_images = CEGUI::ImagesetManager::getSingleton().createImageset( "gui/imagesets/TaharezLook.imageset" );

    _p_mouseImg = const_cast< CEGUI::Image* >( &p_images->getImage( "MouseArrow" ) );
    CEGUI::System::getSingleton().setDefaultMouseCursor( _p_mouseImg );

    // create necessary fonts
    CEGUI::Font* p_font = NULL;
    p_font = createFont( std::string( "gui/fonts/" YAF3D_GUI_FONT8 ".font" ) );
    p_font = createFont( std::string( "gui/fonts/" YAF3D_GUI_FONT10 ".font" ) );
    CEGUI::System::getSingleton().setDefaultFont( p_font ); // set the default font
    p_font = createFont( std::string( "gui/fonts/" YAF3D_GUI_CONSOLE ".font" ) );

    // load scheme
    CEGUI::SchemeManager::getSingleton().loadScheme( std::string( "gui/schemes/" YAF3D_GUI_SCHEME ".scheme" ) );

    // create the root window called 'Root'.
    _p_root = static_cast< CEGUI::DefaultWindow* >( CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "Root") );
    _p_root->setMetricsMode( CEGUI::Absolute );
    _p_root->setPosition( CEGUI::Point( 0, 0 ) );
    _p_root->setSize( CEGUI::Size( _windowWidth, _windowHeight ) );

    // set the GUI root window (also known as the GUI "sheet"), all layout will be added to this root for showing up
    CEGUI::System::getSingleton().setGUISheet( _p_root );

    // create input handler
    _inputHandler = new InputHandler( this );
}

CEGUI::Font* GuiManager::createFont( const std::string& fontname )
{
    CEGUI::Font* p_font = NULL;

    try 
    {
        // load font
        p_font = CEGUI::FontManager::getSingleton().createFont( fontname );
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
        log << Log::LogLevel( Log::L_ERROR ) << "GuiManager: cannot create font: '" << fontname << "'" << std::endl;
        log << Log::LogLevel( Log::L_ERROR ) << " reason: " << e.getMessage().c_str() << "'" << std::endl;
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

    log << Log::LogLevel( Log::L_ERROR ) << " GuiManager: no font with name '" << fontname << "' exists!" << std::endl;
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
        CEGUI::MouseCursor::getSingleton().show();
    else
        CEGUI::MouseCursor::getSingleton().hide();
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

CEGUI::Window* GuiManager::loadLayout( const std::string& filename, CEGUI::Window* p_parent, const std::string& handle )
{
    assert( _p_root && " gui system is not initialized!" );

    std::stringstream pref;

    if ( !handle.length() )
    {
        // create an auto-incremented prefix, so we can load several layouts
        static int prefix = 0;
        pref << "l" << prefix << "_";
        prefix++;
    }
    else
        pref << handle;

    CEGUI::Window* p_layout = NULL;
    try
    {
        p_layout = CEGUI::WindowManager::getSingleton().loadWindowLayout( filename.c_str(), pref.str() );

        if ( !p_parent )
            _p_root->addChildWindow( p_layout );
        else
            p_parent->addChildWindow( p_layout );

    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "GuiManager: cannot load layout: '" << filename << "'" << std::endl;
        log << Log::LogLevel( Log::L_ERROR ) << " reason: " << e.getMessage().c_str() << "'" << std::endl;
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
    if ( _active )
        CEGUI::System::getSingleton().injectTimePulse( deltaTime );    
}

void GuiManager::doRender()
{
    if ( _active )
        CEGUI::System::getSingleton().renderGUI();
}

bool GuiManager::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    if ( !getUserObject()->_active )
        return false;

    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int eventType    = p_eventAdapter->getEventType();
    int          key          = p_eventAdapter->getSDLKey();
 
    // dispatch key down activity
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        // some keys may be handled via key code and generate those too
        switch ( key )
        {
            case SDLK_BACKSPACE:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::Backspace );
                break;

            case SDLK_RETURN:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::Return );
                break;

            case SDLK_DELETE:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::Delete );
                break;

            case SDLK_ESCAPE:
                //CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::Escape );
                break;

            case SDLK_LEFT:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::ArrowLeft );
                break;

            case SDLK_RIGHT:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::ArrowRight );
                break;

            case SDLK_UP:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::ArrowUp );
                break;

            case SDLK_DOWN:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::ArrowDown );
                break;

            case SDLK_HOME:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::Home );
                break;

            case SDLK_END:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::End );
                break;

            case SDLK_PAGEUP:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::PageUp );
                break;

            case SDLK_PAGEDOWN:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::PageDown );
                break;

            default:
                ;
        }
        const SDL_Event& event = p_eventAdapter->getSDLEvent();
        // inject the unicode character
        CEGUI::System::getSingleton().injectChar( static_cast< CEGUI::utf32 >( event.key.keysym.unicode ) );
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
            mbtn_down = false;
            mbtn_up   = true;
        }
    }
    else
    {
        if ( !mbtn_up )
        {
            CEGUI::System::getSingleton().injectMouseButtonUp( CEGUI::MiddleButton );
            mbtn_down = false;
            mbtn_up   = true;
        }
    }

    if ( !getUserObject()->_lockMouse )
    {
        if ( ( eventType == osgGA::GUIEventAdapter::MOVE ) || ( eventType == osgGA::GUIEventAdapter::DRAG ) )
        {
            // adjust the pointer position
            float x = ea.getX();
            float y = ea.getY();
            CEGUI::System::getSingleton().injectMousePosition( x, y );
        }
    }

    return false;
}

}
