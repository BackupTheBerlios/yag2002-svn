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

#ifndef _CTD_GUI_H_
#define _CTD_GUI_H_

#include <ctd_base.h>
#include <ctd_singleton.h>

namespace CEGUI
{
    class OpenGLRenderer;
}

namespace CTD
{

class Application;
class CTDGuiRenderer;
class GuiRenderCallback;

// Graphical User Interface
/**
   Actually this class is a wrapper around CEGUI system ( http://www.cegui.org.uk )
*/
class GuiManager : public Singleton< GuiManager >
{
    public:

        //! Load a gui layout ( e.g. a dialog ) from given file.
        CEGUI::Window*                          loadLayout( const std::string& filename );

        //! Show up given layout in root window.
        void                                    showLayout( CEGUI::Window* p_layout );

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

        //! Shutdown gui manager
        void                                    shutdown();

        CTDGuiRenderer*                         _p_renderer;

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

        osg::ref_ptr< InputHandler >            _inputHandler;

        float                                   _screenWidth;

        float                                   _screenHeight;

        float                                   _mouseSensivity;

        osg::ref_ptr< GuiRenderCallback >       _guiRenderCallback;

        //! Root window of cegui where all other windows are placed
        CEGUI::DefaultWindow*                  _p_root;

    friend class Singleton< GuiManager >;
    friend class GuiRenderCallback;
    friend class Application;
};

} // namespace CTD

#endif //_CTD_GUI_H_
