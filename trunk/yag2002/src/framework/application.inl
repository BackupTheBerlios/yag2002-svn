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
 # main application class' inline header
 #
 #   date of creation:  01/22/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _APPLICATION_H_
    #error "do not include this file directly, include application.h instead"
#endif

inline osgSDL::Viewer* Application::getViewer()
{
    return _p_viewer;
}

inline osgUtil::SceneView* Application::getSceneView( int num )
{
    assert( _p_viewer && "viewer is not created!" );
    return _p_viewer->getViewport( num )->getSceneView();
}

inline const std::string& Application::getMediaPath() const
{
    return _mediaPath;
}

inline const std::string& Application::getFullBinPath() const
{
    return _fulBinaryPath;
}

inline void Application::setSceneRootNode( osg::Group* p_root )
{
    _rootSceneNode = p_root;
    getSceneView()->setSceneData( _rootSceneNode.get() );
}

inline osg::Group* Application::getSceneRootNode()
{
    return _rootSceneNode.get();
}
        
inline void Application::getScreenSize( unsigned int& x, unsigned int& y ) const
{
    x = _screenWidth;
    y = _screenHeight;
}

inline void Application::setWindowTitle( const std::string& title )
{
    _appWindowTitle = title;
    SDL_WM_SetCaption( _appWindowTitle.c_str(), NULL );
}

inline const std::string& Application::getWindowTitle()
{
    return _appWindowTitle;
}

inline void Application::setAppWindowMinimized( bool en )
{
    _appWindowMinimized = en;
}

inline void Application::handleCtrlC()
{
    _ctrlCPressed = true;
}
