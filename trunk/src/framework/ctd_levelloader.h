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
 # neoengine, level loader
 #
 # this class implements reading level file '.lvl' and initializing
 #  all static and active objects in level
 #
 #
 #   date of creation:  11/16/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  11/16/2003 boto       creation of CTDLevelLoader
 #
 #  12/05/2003 boto       implementation of plugin loading and initializing
 #
 #  04/09/2004 boto       added enabling networking for entities
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_LEVEL_LOADER_H_
#define _CTD_LEVEL_LOADER_H_


#include <ctd_frbase.h>
#include "ctd_pluginmanager.h"

#include <string>


namespace CTD 
{

class FrameworkImpl;
class LevelSet;

//! Level loader
/**
* This class loads a level configuration file ( xml ) containing map and plugin/entity definitions.
*/
class LevelLoader : public NeoEngine::LoadableEntity
{

    protected:

        /**
        * Construct the level loader
        */
                                    LevelLoader( LevelSet* pkSet, FrameworkImpl* pkFrameworkImpl );

                                    ~LevelLoader();

        /**
        * Overloaded node loading function
        */
        bool                        LoadNode( unsigned int uiFlags = 0 );

        //! Show up loading picture
        void                        ShowLoadingPicture( const std::string &strPicture );
        
        //! Instance of level set where loaded room and plugin manager are stored
        LevelSet                   *m_pkLevelSet;

        //! Instance of framework implementation
        FrameworkImpl              *m_pkFrameworkImpl;

    private:

        //! Avoid default and copy constructors 
                                    LevelLoader();

                                    LevelLoader( LevelLoader &rkLevelLoader );

        LevelLoader&                operator = ( LevelLoader& );

    friend class FrameworkImpl;

};

} // namespace CTD 

#endif //_CTD_LEVEL_LOADER_H_
