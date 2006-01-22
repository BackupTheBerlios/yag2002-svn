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
 # main application class
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <base.h>
#include <singleton.h>

namespace yaf3d
{

class NetworkDevice;
class EntityManager;
class LevelManager;
class SoundManager;
class GuiManager;
class GameState;
class Physics;

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

        //! Set the application window title
        void                                        setWindowTitle( const std::string& title );

        //! Get the application window title
        const std::string&                          getWindowTitle();

    protected:


                                                    Application();

        virtual                                     ~Application();

        void                                        updateStandalone( float deltaTime );

        void                                        updateServer( float deltaTime );

        void                                        updateClient( float deltaTime );

        NetworkDevice*                              _p_networkDevice;

        EntityManager*                              _p_entityManager;

        GuiManager*                                 _p_guiManager;

        SoundManager*                               _p_soundManager;

        Physics*                                    _p_physics;

        GameState*                                  _p_gameState;

        osgSDL::Viewer*                             _p_viewer;

        unsigned int                                _screenWidth;

        unsigned int                                _screenHeight;

        bool                                        _fullScreen;

        osg::ref_ptr< osg::Group >                  _rootSceneNode;

        std::string                                 _mediaPath;

        std::string                                 _fulBinaryPath;

        std::string                                 _appWindowTitle;

    friend class Singleton< Application >;
};

//! Include inline definitions
#include "application.inl"

}

#endif // _APPLICATION_H_
