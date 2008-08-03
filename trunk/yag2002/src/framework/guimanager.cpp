/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "guimanager.h"
#include "application.h"
#include "configuration.h"
#include "guirenderer.h"
#include "filesystem.h"
#include "keymap.h"
#include "log.h"
#include <CEGUILogger.h>

namespace yaf3d
{

// used for copy and paste functionality
#define TYPE_EDITBOX                    "TaharezLook/Editbox"
#define TYPE_MULITLINEBOX               "TaharezLook/MultiLineEditbox"

// maximal length of the string getting from clipboard
#define MAX_GET_CLIPBOARD_TEXT_SIZE     255

//! Viewer's init callback. Here we initialize CEGUI's renderer.
class GuiViewerInitCallback : public osgSDL::Viewer::ViewerInitCallback
{
    public:

        virtual void                                operator()( const osgSDL::Viewport* /*p_vp*/ )
                                                    {
                                                        GuiManager::get()->doInitialize();
                                                    }

    protected:

        virtual                                     ~GuiViewerInitCallback() {}

};

//! Resource loader for gui resource loading
class GuiResourceProvider : public CEGUI::ResourceProvider
{
    public:
                                                    GuiResourceProvider() : ResourceProvider() {}

                                                    ~GuiResourceProvider() {}

        void                                        loadRawDataContainer( const CEGUI::String& filename, CEGUI::RawDataContainer& output, const CEGUI::String& resourceGroup );

};

//! OSG Drawable object for rendering the GUI
class CEGUIDrawable : public osg::Drawable
{
    public:

        META_Object( osg,CEGUIDrawable );


                                                    CEGUIDrawable()
                                                    {
                                                        // setup drawable
                                                        setSupportsDisplayList( false );
                                                        _activeContextID = 0;
                                                        // create a gui renderbin and make the gui drawable to be drawn in this render bin ( last stage of rendering )
                                                        osg::ref_ptr< osgUtil::RenderBin > renderbin = new osgUtil::RenderBin( osgUtil::RenderBin::SORT_BACK_TO_FRONT );
                                                        osg::StateSet* p_stateset = new osg::StateSet;
                                                        p_stateset->setRenderBinDetails( 1000, "RenderBin", osg::StateSet::USE_RENDERBIN_DETAILS );
                                                        setStateSet( p_stateset );
                                                    }

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
                                                    CEGUIDrawable( const CEGUIDrawable& drawable, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                                                        Drawable( drawable, copyop )
                                                    {
                                                    }

        //! Draw implementation
        void                                        drawImplementation( osg::RenderInfo& renderInfo ) const
                                                    {
                                                        osg::State* p_state = renderInfo.getState();

                                                        if ( p_state->getContextID()!=_activeContextID )
                                                            return;

                                                        p_state->disableAllVertexArrays();
                                                        CEGUI::System::getSingleton().renderGUI();
                                                    }

    protected:

        virtual                                    ~CEGUIDrawable() {};

        //! Opengl context
        unsigned int                               _activeContextID;
};



GuiManager::InputHandler::InputHandler( GuiManager* p_guimgr ) :
_p_guiMgr( p_guimgr )
{
    // register us in viewer to get event callbacks
    Application::get()->getViewer()->addEventHandler( this );
}

GuiManager::InputHandler::~InputHandler()
{
    // remove this handler from viewer's handler list
    Application::get()->getViewer()->removeEventHandler( this );
}

void GuiResourceProvider::loadRawDataContainer( const CEGUI::String& filename, CEGUI::RawDataContainer& output, const CEGUI::String& /*resourceGroup*/ )
{
    FilePtr file = FileSystem::get()->getFile( filename.c_str() );
    if ( !file.valid() )
    {
        log_warning << "GuiResourceProvider: cannot find file: " << filename << std::endl;
        return;
    }
    output.setData( reinterpret_cast< unsigned char* >( file->releaseBuffer() ) );
    output.setSize( file->getSize() );
}


YAF3D_SINGLETON_IMPL( GuiManager )

// implementation of GuiManager
GuiManager::GuiManager() :
_p_renderer( NULL ),
_p_mouseImg( NULL ),
_windowWidth( 600 ),
_windowHeight( 400 ),
_p_root( NULL ),
_p_resourceLoader( NULL ),
_active( true ),
_lockMouse( false )
{
}

GuiManager::~GuiManager()
{
}

void GuiManager::shutdown()
{
    log_info << "GuiManager: shutting down" << std::endl;

    delete CEGUI::System::getSingletonPtr();
    delete _p_renderer;

    if ( _p_resourceLoader )
        delete _p_resourceLoader;

    // destroy singleton
    destroy();
}

void GuiManager::initialize()
{
    // append the gui node into scenegraph
    osgSDL::Viewer* p_viewer = Application::get()->getViewer();
    osg::ref_ptr< osg::Geode > p_geode = new osg::Geode;
    p_geode->setName( "_GUI_" );
    osg::ref_ptr< CEGUIDrawable > p_drawble = new CEGUIDrawable();
    p_geode->addDrawable( p_drawble.get() );
    p_geode->setDataVariance( osg::Geode::DYNAMIC );
    p_geode->setCullingActive( false );
    Application::get()->getSceneRootNode()->addChild( p_geode.get() );

    // register a viewer realize callback, here we initialize CEGUI's renderer (using doInitialize)
    _guiViewerInitCallback = new GuiViewerInitCallback;
    p_viewer->addViewerInitCallback( _guiViewerInitCallback.get() );
}

void GuiManager::doInitialize()
{
    // get window size
    unsigned int width = 0, height = 0;
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENWIDTH,  width  );
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENHEIGHT, height );
    _windowWidth = float( width );
    _windowHeight = float( height );

    try
    {
        // create a renderer
        _p_renderer = new GuiRenderer( 0, int( _windowWidth ), int( _windowHeight ) );

        // create the gui
        _p_resourceLoader = new GuiResourceProvider;
        new CEGUI::System( _p_renderer, _p_resourceLoader );

        // set logging level
        CEGUI::Logger::getSingleton().setLoggingLevel( CEGUI::Insane );

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

    }
    catch( const CEGUI::Exception& e )
    {
        log_error << "GuiManager: problem occured on initializing the gui system." << std::endl;
        log_error << " reason: " << e.getMessage() << std::endl;
        Application::get()->stop();
    }

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
        log_error << "GuiManager: cannot create font: '" << fontname << "'" << std::endl;
        log_error << " reason: " << e.getMessage().c_str() << "'" << std::endl;
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

    log_error << " GuiManager: no font with name '" << fontname << "' exists!" << std::endl;
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
        ++prefix;
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
        log_error << "GuiManager: cannot load layout: '" << filename << "'" << std::endl;
        log_error << " reason: " << e.getMessage().c_str() << "'" << std::endl;
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

GuiRenderer* GuiManager::getGuiRenderer()
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

bool GuiManager::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
{
    if ( !_p_guiMgr->_active )
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

            case SDLK_LSHIFT:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::LeftShift );
                break;

            case SDLK_RSHIFT:
                CEGUI::System::getSingleton().injectKeyDown( CEGUI::Key::RightShift );
                break;

            default:
                ;
        }
        const SDL_Event& event = p_eventAdapter->getSDLEvent();
        // inject the unicode character
        CEGUI::System::getSingleton().injectChar( static_cast< CEGUI::utf32 >( event.key.keysym.unicode ) );
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {
        switch ( key )
        {
            case SDLK_BACKSPACE:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::Backspace );
                break;

            case SDLK_RETURN:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::Return );
                break;

            case SDLK_DELETE:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::Delete );
                break;

            case SDLK_ESCAPE:
                //CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::Escape );
                break;

            case SDLK_LEFT:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::ArrowLeft );
                break;

            case SDLK_RIGHT:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::ArrowRight );
                break;

            case SDLK_UP:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::ArrowUp );
                break;

            case SDLK_DOWN:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::ArrowDown );
                break;

            case SDLK_HOME:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::Home );
                break;

            case SDLK_END:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::End );
                break;

            case SDLK_PAGEUP:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::PageUp );
                break;

            case SDLK_PAGEDOWN:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::PageDown );
                break;

            case SDLK_LSHIFT:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::LeftShift );
                break;

            case SDLK_RSHIFT:
                CEGUI::System::getSingleton().injectKeyUp( CEGUI::Key::RightShift );
                break;

            default:
                ;
        }
    }

    // dispatch Ctrl+C anc Ctrl+X / Ctrl+V
    static bool _ctrl   = false;
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        bool cuttext = false;

        switch ( key )
        {
            case SDLK_LCTRL:
            case SDLK_RCTRL:
            {
                _ctrl = true;
            }
            break;

            case SDLK_x:
                cuttext = true;

            case SDLK_c:
            {
                if ( _ctrl )
                {
                    if ( _p_guiMgr->_p_root )
                    {
                        CEGUI::Window* p_active = _p_guiMgr->_p_root->getActiveChild();
                        if ( p_active )
                        {
                            CEGUI::String wintype = p_active->getType();
                            if ( wintype == TYPE_EDITBOX )
                            {
                                // get the selected text
                                CEGUI::Editbox* p_editbox = static_cast< CEGUI::Editbox* >( p_active );
                                CEGUI::String::size_type beg = p_editbox->getSelectionStartIndex();
                                CEGUI::String::size_type len = p_editbox->getSelectionLength();

                                // copy the selected text to clipboard
                                {
                                    std::wstring seltext;
                                    CEGUI::String cpytext( p_editbox->getText().substr( beg, len ) ); // get selected text
                                    // convert the text to std::wstring
                                    std::size_t stringlen = cpytext.length();
                                    for ( std::size_t cnt = 0; cnt < stringlen; ++cnt )
                                        seltext += cpytext[ cnt ];

                                    copyToClipboard( seltext );
                                }

                                // are we cutting or just copying?
                                if ( cuttext && !p_editbox->isReadOnly() )
                                {
                                    CEGUI::String newtext = p_editbox->getText();
                                    p_editbox->setText( newtext.erase( beg, len ) );
                                }
                            }
                            else if ( wintype == TYPE_MULITLINEBOX )
                            {
                                // get the selected text
                                CEGUI::MultiLineEditbox* p_mleditbox = static_cast< CEGUI::MultiLineEditbox* >( p_active );
                                CEGUI::String::size_type beg = p_mleditbox->getSelectionStartIndex();
                                CEGUI::String::size_type len = p_mleditbox->getSelectionLength();

                                // copy the selected text to clipboard
                                {
                                    std::wstring seltext;
                                    CEGUI::String cpytext( p_mleditbox->getText().substr( beg, len ) ); // get selected text
                                    // convert the text to std::wstring
                                    std::size_t stringlen = cpytext.length();
                                    for ( std::size_t cnt = 0; cnt < stringlen; ++cnt )
                                        seltext += cpytext[ cnt ];

                                    copyToClipboard( seltext );
                                }

                                // are we cutting or just copying?
                                if ( cuttext && !p_mleditbox->isReadOnly() )
                                {
                                    CEGUI::String newtext = p_mleditbox->getText();
                                    p_mleditbox->setText( newtext.erase( beg, len ) );
                                }
                            }
                        }
                    }
                }
            }
            break;

            case SDLK_v:
            {
                if ( _ctrl )
                {
                    std::wstring text;
                    if ( getFromClipboard( text, MAX_GET_CLIPBOARD_TEXT_SIZE ) )
                    {
                        std::size_t stringlen = text.length();
                        for ( std::size_t cnt = 0; cnt < stringlen; ++cnt )
                            CEGUI::System::getSingleton().injectChar( static_cast< CEGUI::utf32 >( text[ cnt ] ) );
                    }
                }
            }
            break;

            default:
                ;
        }
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {
        if ( ( key == SDLK_LCTRL ) || ( key == SDLK_RCTRL ) )
            _ctrl = false;
    }

    // handle mouse
    unsigned int buttonMask = p_eventAdapter->getButtonMask();
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

    if ( !_p_guiMgr->_lockMouse )
    {
        if ( ( eventType == osgGA::GUIEventAdapter::MOVE ) || ( eventType == osgGA::GUIEventAdapter::DRAG ) )
        {
            // adjust the pointer position
            float x = p_eventAdapter->getX();
            float y = p_eventAdapter->getY();
            CEGUI::System::getSingleton().injectMousePosition( x, y );
        }
    }

    return false;
}

}
