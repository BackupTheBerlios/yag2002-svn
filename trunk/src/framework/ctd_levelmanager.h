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
 # neoengine, level manager
 #
 #
 #   date of creation:  10/03/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  10/03/2004 boto       creation of LevelManager and LevelSet
 #
 ################################################################*/


#ifndef _CTD_LEVELMANAGER_H_
#define _CTD_LEVELMANAGER_H_

#include <ctd_frbase.h>

namespace CTD
{

class PluginManager;
class LevelManager;
class FrameworkImpl;

//! Maximal count of level sets
#define CTD_MAX_LEVELSETS   10

//! LevelSet contains a named set of plugin manager, room, and camera.
class LevelSet
{

    public:

        /**
        * Get the name of level set
        * \return                               Name of this level set
        */
        std::string                             GetName() { return m_strName; }

        /**
        * Checks level set whether it wants to be updated in game loop.
        * \return                               true if level set should be updated, otherwise flase.
        */
        bool                                    GetUpdateFlag() { return m_bCanUpdate; }

        /**
        * Set the update flag
        * \param bUpdate                        If true then the level set will get updated in game loop, otherwise i won't be updated.
        */
        void                                    SetUpdateFlag( bool bUpdate ) { m_bCanUpdate = bUpdate; }

        /**
        * Checks level set whether it wants to be rendered in game loop.
        * \return                               true if level set should be rendered, otherwise flase.
        */
        bool                                    GetRenderFlag() { return m_bCanRender; }

        /**
        * Set the render flag
        * \param bRender                        If true then the level set will get rendered in game loop, otherwise i won't be rendered.
        */
        void                                    SetRenderFlag( bool bRender ) { m_bCanRender = bRender; }

        /**
        * Set level set's camera used in game loop. This must be done by a plugin.
        * \param pkCamera                       Level set's camera
        */
        void                                    SetCamera( NeoEngine::Camera* pkCamera ) { m_pkCamera = pkCamera; }

        /**
        * Get camera
        * \return                               Level set's camera
        */
        NeoEngine::Camera*                      GetCamera() { return m_pkCamera; }

        /**
        * Get room
        * \return                               Level set's room
        */
        NeoEngine::Room*                        GetRoom() { return m_pkRoom; }

        /**
        * Get plugin manager
        * \return                               Level set's plugin manager
        */
        PluginManager*                          GetPluginManager() { return m_pkPluginMgr; }

    protected:

                                                LevelSet() { m_pkPluginMgr = NULL; m_pkRoom = NULL; m_pkCamera = NULL; m_bCanUpdate = m_bCanRender = true; }
                                                
                                                LevelSet( const std::string &strName, PluginManager* pkPluginMgr, NeoEngine::Room *pkRoom )
                                                { 
                                                    m_strName       = strName;
                                                    m_pkPluginMgr   = pkPluginMgr;
                                                    m_pkRoom        = pkRoom; 
                                                }

                                                ~LevelSet() {};

        //! Level set name
        std::string                             m_strName;

        //! Plugin manager instance
        PluginManager                           *m_pkPluginMgr;

        //! Room instance
        NeoEngine::Room                         *m_pkRoom;

        //! Camera instance
        NeoEngine::Camera                       *m_pkCamera;

        //! Should the set be updated?
        bool                                    m_bCanUpdate;

        //! Should the set be rendered?
        bool                                    m_bCanRender;

    friend class LevelManager;
    friend class LevelLoader;
    friend class FrameworkImpl;

};

//! Class for managing level sets
class LevelManager
{

    public:

        /**
        * Set the update flag of a level set given its name
        * \param strName                        Level set name
        * \param bFlag                          If true then update the level set in game loop.
        */
        void                                    SetUpdateFlag( const std::string &strName, bool bFlag );

        /**
        * Set the render flag of a level set given its name
        * \param strName                        Level set name
        * \param bFlag                          If true then render the level set in game loop.
        */
        void                                    SetRenderFlag( const std::string &strName, bool bFlag );

        /**
        * Get a set given its name
        * \param strName                        Set's name
        * \return                               LevelSet, if the name does not exists then NULL is returned.
        */
        LevelSet*                               GetLevelSet( const std::string &strName );

        /** 
        * Get all existing sets
        * \return                               Pointer array containing all sets. Use GetLevelSetCount() to get total count of level sets in the array.
        */
        LevelSet**                              GetAllLevelSets();

        /**
        * Get total count of level sets.
        * \return                               Count of level sets
        */
        unsigned int                            GetLevelSetCount() { return m_uiNumSets; }

    protected:

                                                LevelManager();

                                                ~LevelManager();

        /**
        * Add a new level set
        * \param pkSet                          Level set to be added
        * \return                               false if the name already exists. In this case the new set is not added.
        */
        bool                                    Add( LevelSet* pkSet );

        /**
        * Destroy an existing set
        * \param strName                        Name of set to be destroyed
        * \return                               false if the name does not exist, otherwise the set is destroyed and true is returned.
        */
        bool                                    Destroy( const std::string &strName );

        /**
        * Destroy all existing sets
        */
        void                                    DestroyAll();

        //! Array of level sets
        /**
        * Note: We don't use a std vector here as there would exist problems during query of the vector and manupulating it 
        *  in the same time. This happens in main loop when we load a level during the game runs.
        */
        LevelSet                                *m_apkLevelSets[ CTD_MAX_LEVELSETS ];

        //! Total count of level sets
        unsigned int                            m_uiNumSets;

    friend class LevelLoader;
    friend class FrameworkImpl;

};


} // namespace CTD

#endif // _CTD_LEVELMANAGER_H_

