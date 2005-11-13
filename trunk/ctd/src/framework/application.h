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
 # main application class
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _YAF3DAPPLICATION_H_
#define _YAF3DAPPLICATION_H_

#include <ctd_base.h>
#include <ctd_singleton.h>

namespace yaf3d
{

class LevelManager;
class EntityManager;
class GuiManager;
class NetworkDevice;
class Physics;
class GameState;

//! Application
class Application : public Singleton< Application >
{
    public:

        //! Initialize application. Returns false if something goes wrong.
        virtual bool                                initialize( int argc, char **argv );

        //! Start main loop.
        virtual void                                run();

        //! Exit application
        virtual void                                stop();

        //! Shutdown application
        virtual void                                shutdown();

        //! Retrieve the media path root where the multimedia content resides
        inline const std::string&                   getMediaPath() const;

        //! Retrieve the full path of application binary
        inline const std::string&                   getFullBinPath() const;

        //! Get osgProducer's viewer instance for this application
        inline osgSDL::Viewer*                      getViewer();

        //! Get the scene view for given view port ( the default is 0 )
        inline osgUtil::SceneView*                  getSceneView( int num = 0 );

        //! Get the top node in scenegraph
        inline osg::Group*                          getSceneRootNode();

        //! Set the top node in scenegraph, be careful using this method!
        inline void                                 setSceneRootNode( osg::Group* p_root );

        //! Get screen size in width and height
        inline void                                 getScreenSize( unsigned int& x, unsigned int& y ) const;

    protected:


                                                    Application();

        virtual                                     ~Application();

        void                                        updateStandalone( float deltaTime );

        void                                        updateServer( float deltaTime );

        void                                        updateClient( float deltaTime );

        NetworkDevice*                              _p_networkDevice;

        EntityManager*                              _p_entityManager;

        GuiManager*                                 _p_guiManager;

        Physics*                                    _p_physics;

        GameState*                                  _p_gameState;

        osgSDL::Viewer*                             _p_viewer;

        unsigned int                                _screenWidth;

        unsigned int                                _screenHeight;

        bool                                        _fullScreen;

        osg::ref_ptr< osg::Group >                  _rootSceneNode;

        std::string                                 _mediaPath;

        std::string                                 _fulBinaryPath;

    friend class Singleton< Application >;
};

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

}
#endif // _YAF3DAPPLICATION_H_
