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
 # main application class
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_APPLICATION_H_
#define _CTD_APPLICATION_H_

#include <ctd_singleton.h>

namespace CTD
{

class LevelManager;
class EntityManager;
class GuiManager;
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
        const std::string&                          getMediaPath();

        //! Get osgProducer's viewer instance for this application
        inline osgProducer::Viewer*                 getViewer();

        //! Get the top node in scenegraph
        inline osg::Group*                          getSceneRootNode();

        //! Set the top node in scenegraph, be carefull using this method!
        inline void                                 setSceneRootNode( osg::Group* p_root );

        //! Get screen size in width and height
        inline void                                 getScreenSize( unsigned int& x, unsigned int& y );

    protected:


                                                    Application();

        virtual                                     ~Application();

        EntityManager*                              _entityManager;

        osgAL::SoundManager*                        _p_soundManager;

        GuiManager*                                 _p_guiManager;

        Physics*                                    _p_physics;

        GameState*                                  _p_gameState;

        osgProducer::Viewer*                        _p_viewer;

        unsigned int                                _screenWidth;

        unsigned int                                _screenHeight;

        osg::Group*                                 _p_rootSceneNode;

        std::string                                 _mediaPath;

        float                                       _mouseSensivity;

    friend class Singleton< Application >;
};

inline osgProducer::Viewer* Application::getViewer()
{
    return _p_viewer;
}

inline const std::string& Application::getMediaPath()
{
    return _mediaPath;
}

inline void Application::setSceneRootNode( osg::Group* p_root )
{
    _p_rootSceneNode = p_root;
}

inline osg::Group* Application::getSceneRootNode()
{
    return _p_rootSceneNode;
}
        
inline void Application::getScreenSize( unsigned int& x, unsigned int& y )
{
    x = _screenWidth;
    y = _screenHeight;
}

}
#endif // _CTD_APPLICATION_H_
