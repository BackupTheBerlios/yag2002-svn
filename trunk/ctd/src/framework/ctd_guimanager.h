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

#ifndef _CTD_GUI_H_
#define _CTD_GUI_H_

#include <ctd_base.h>
#include <ctd_singleton.h>
#include <ctd_guirenderer.h>

namespace CTD
{

class Application;
class LevelManager;
class CTDGuiRenderer;
class GuiRenderCallback;
class GuiViewerRealizeCallback;

// some definitions
#define CTD_GUI_FONT            "Commonwealth-10"
#define CTD_GUI_SCHEME          "TaharezLookWidgets"

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
        void                                    getGuiArea( float& width, float& height );

        //! Activate / deactive gui event handling and rendering
        void                                    activate( bool active );

        //! Get active / deactive state
        bool                                    isActive();

        //! Retrieve the gui renderer. One can used it e.g. to create textures for StaticImage elements.
        CTDGuiRenderer*                         getGuiRenderer();

        //! Retrieve the root window. This can be used in conjuction with manually created elements.
        CEGUI::DefaultWindow*                   getRootWindow();

        //! Show / hide mouse pointer
        void                                    showMousePointer( bool show );

    protected:

                                                GuiManager();

        virtual                                 ~GuiManager();

        //! Avoid copy constructor
        GuiManager&                             operator = ( const GuiManager& guimgr );

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

        //! Input handler, inputs are forwarded to gui system
        class InputHandler : public osgGA::GUIEventHandler
        {
            public:

                                                InputHandler( GuiManager* p_guimgr );                                                    
                                                
                virtual                         ~InputHandler() {};


                //! Handle input events
                bool                            handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

            protected:

                GuiManager*                     _p_guiMgr;
        };

        CTDGuiRenderer*                         _p_renderer;

        CEGUI::Image*                           _p_mouseImg;

        osg::ref_ptr< InputHandler >            _inputHandler;

        float                                   _windowWidth;

        float                                   _windowHeight;

        osg::ref_ptr< GuiRenderCallback >       _guiRenderCallback;

        osg::ref_ptr< GuiViewerRealizeCallback > _guiRealizeCallback;

        //! Root window of cegui where all other windows are placed
        CEGUI::DefaultWindow*                   _p_root;

        //! Is gui rendering active?
        bool                                    _active;

    friend class Singleton< GuiManager >;
    friend class GuiRenderCallback;
    friend class GuiViewerRealizeCallback;
    friend class Application;
    friend class LevelManager;
};

} // namespace CTD

#endif //_CTD_GUI_H_
