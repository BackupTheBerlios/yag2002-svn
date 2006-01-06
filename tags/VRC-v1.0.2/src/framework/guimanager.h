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

#ifndef _GUI_H_
#define _GUI_H_

#include <base.h>
#include <singleton.h>
#include <guirenderer.h>
#include <utils.h>

namespace yaf3d
{

class Application;
class LevelManager;
class GuiRenderer;
class GuiPostDrawCallback;
class GuiViewerInitCallback;

// some definitions
#define YAF3D_GUI_FONT8           "yaf3d-8"
#define YAF3D_GUI_FONT10          "yaf3d-10"
#define YAF3D_GUI_CONSOLE         "yaf3d-Console"
#define YAF3D_GUI_SCHEME          "TaharezLookWidgets"

// Graphical User Interface
/**
   Actually this class is a wrapper around CEGUI system ( http://www.cegui.org.uk )
*/
class GuiManager : public Singleton< GuiManager >
{
    public:

        //! Load a gui layout ( e.g. a dialog ) from given file and handle.
        /**
        * If handle is empty then a handle is automaticaly genereated, if so you cannot access the elements in layout later!
        * example: 
        *   CEGUI::Window* p_wnd = loadlayout( mylayoutfile, "myhanlde" )
        *   CEGUI::WIndow* p_btn = p_wnd->getChild( "myhandle" + "OK_BUTTON" );
        * where OK_BUTTON is defined in layout file.
        * If p_parent is NULL then the layout will be appended to the root window.
        */
        CEGUI::Window*                          loadLayout( const std::string& filename, CEGUI::Window* p_parent = NULL, const std::string& handle = "" );

        //! Show a message dialog
        void                                    messageBox( const std::string& title, const std::string& message );

        //! Get the render area of gui.
        void                                    getGuiArea( float& width, float& height ) const;

        //! Activate / deactive gui event handling and rendering
        void                                    activate( bool active );

        //! Get active / deactive state
        bool                                    isActive() const;

        //! Retrieve the gui renderer. One can used it e.g. to create textures for StaticImage elements.
        GuiRenderer*                            getGuiRenderer();

        //! Retrieve a pre-loaded font resource given its name ( e.g. yaf3d-8 or yaf3d-10 )
        /**
         * If there is no font with that name then the return value is NULL.
         */
        CEGUI::Font*                            getFont( const std::string& fontname );

        //! Load and register a custom font given its description file (in CEGUI's .font format)
        /*
         * You can retrieve the font using getFont method with appropriate font name (defined in .font file)
         * If the file does not exist then the return value is NULL.
         */
        CEGUI::Font*                            loadFont( const std::string& descriptionfile );

        //! Change the client area, use this e.g. when the main app windows shape changed
        void                                    changeDisplayResolution( float width, float height );

        //! Retrieve the root window. This can be used in conjuction with manually created elements.
        CEGUI::DefaultWindow*                   getRootWindow();

        //! Show / hide mouse pointer
        void                                    showMousePointer( bool show ) const;

        //! Lock the mouse to given x and y position. The mouse movement handling will be disabled for pointer positioning.
        /*!
         * x and x must be in range [ -1 ... 1 ]
         */
        void                                    lockPointer( float x, float y );

        //! Enable the mouse handling evaluating mouse movement.
        void                                    releasePointer();

    protected:

                                                GuiManager();

        virtual                                 ~GuiManager();

        //! Prepare initializing the gui system
        void                                    initialize();

        //! Initialize the gui system
        void                                    doInitialize();

        //! Update the gui system for time-based elements
        void                                    update( float deltaTime );

        //! Render gui if active
        void                                    doRender();

        //! Shutdown gui manager
        void                                    shutdown();

        //! Used internally to create fonts
        CEGUI::Font*                            createFont( const std::string& fontname );

        //! Input handler, inputs are forwarded to gui system
        class InputHandler : public osgGA::GUIEventHandler
        {
            public:

                explicit                        InputHandler( GuiManager* p_guimgr );

                virtual                         ~InputHandler();


                //! Handle input events
                bool                            handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

            protected:

                GuiManager*                     _p_guiMgr;
        };

        GuiRenderer*                            _p_renderer;

        CEGUI::Image*                           _p_mouseImg;

        osg::ref_ptr< InputHandler >            _inputHandler;

        float                                   _windowWidth;

        float                                   _windowHeight;

        osg::ref_ptr< GuiPostDrawCallback >     _guiPostDrawCallback;

        osg::ref_ptr< GuiViewerInitCallback >   _guiViewerInitCallback;

        //! Root window of cegui where all other windows are placed
        CEGUI::DefaultWindow*                   _p_root;

        //! Loaded fonts
        std::map< std::string, CEGUI::Font* >   _loadedFonts;

        //! Is gui rendering active?
        bool                                    _active;

        //! Disable the mouse positioning
        bool                                    _lockMouse;

    friend class Singleton< GuiManager >;
    friend class GuiViewerInitCallback;
    friend class GuiPostDrawCallback;
    friend class LevelManager;
    friend class Application;
};

} // namespace yaf3d

#endif //_GUI_H_
