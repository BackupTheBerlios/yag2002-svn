/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
        const std::string&                          getMediaPath();

        //! Get osgProducer's viewer instance for this application
        inline osgProducer::Viewer*                 getViewer();

        //! Get the top node in scenegraph
        inline osg::Node*                           getSceneRootNode();

        //! Set the top node in scenegraph, be carefull using this method!
        inline void                                 setSceneRootNode( osg::Node* p_root );

    protected:


                                                    Application();

        virtual                                     ~Application();

        bool                                        _running;

        EntityManager*                              _entityManager;

        osgAL::SoundManager*                        _p_soundManager;

        Physics*                                    _p_physics;

        osgProducer::Viewer*                        _p_viewer;

        osg::Node*                                  _p_rootSceneNode;

        std::string                                 _mediaPath;

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

inline void Application::setSceneRootNode( osg::Node* p_root )
{
    _p_rootSceneNode = p_root;
}

inline osg::Node* Application::getSceneRootNode()
{
    return _p_rootSceneNode;
}

}
#endif // _CTD_APPLICATION_H_
