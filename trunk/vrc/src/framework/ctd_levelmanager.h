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
 # level manager is responsible for loading level configuration 
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_LEVELMANAGER_H_
#define _CTD_LEVELMANAGER_H_

#include "ctd_singleton.h"

namespace CTD
{

class Application;

//! Level manager used only by class Application
class LevelManager : public Singleton< LevelManager >
{
    public:

        //! Load a level given its file name. Returns top most node in scenegraph.
        osg::ref_ptr< osg::Group >                  load( const std::string& levelName );

        //! Load mesh. Return NULL if somethings goes wrong.
        osg::Node*                                  loadMesh( const std::string& fileName, bool useCache = true );

    protected:


                                                    LevelManager();

                                                    LevelManager( const LevelManager& );

        virtual                                     ~LevelManager();

        LevelManager&                               operator = ( const LevelManager& );

        //! Shutdown level manager
        void                                        shutdown();

        //! Load static world geometry. Return NULL if somethings goes wrong.
        osg::Node*                                  loadStaticWorld( const std::string& fileName );

        //! Cache for loaded meshes ( filename / node pairs )
        std::map< std::string, osg::Node* >         _meshCache;

    friend class Singleton< LevelManager >;
    friend class Application;
};

}

#endif // _CTD_LEVELMANAGER_H_
