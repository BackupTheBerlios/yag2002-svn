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
 # common game utilities
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_GAMEUTILS_H_
#define _CTD_GAMEUTILS_H_

#include <ctd_base.h>

namespace CTD
{
namespace gameutils
{

//! Single instance providing GUI-related utility services
class GuiUtils : public CTD::Singleton< CTD::gameutils::GuiUtils >
{
    public:

                                                    GuiUtils();

        virtual                                     ~GuiUtils();

        //! Returns the main GUI window.
        //! Note: all other GUIs in a level must be attached to this one.
        CEGUI::Window*                              getMainGuiWindow();

        //! Show / hide main window
        void                                        showMainWindow( bool show );

        //! Destroy main window
        void                                        destroyMainWindow();

        //! Hide mouse pointer
        void                                        hidePointer();

        //! Show mouse pointer. Pass 'true' in order to show, otherwise hide the pointer.
        void                                        showMousePointer( bool show );

    protected:

        //! The main window instance
        CEGUI::Window*                              _p_mainWindow;        
        
        //! Application's root window
        CEGUI::Window*                              _p_rootWindow;

    friend public CTD::Singleton< CTD::gameutils::GuiUtils >;
};

//! Single instance providing player-related utility services
class PlayerUtils : public CTD::Singleton< CTD::gameutils::PlayerUtils >
{
    public:

                                                    PlayerUtils();

        //! Retrieve player configuration file path depending on game settings and given game mode ( Server, Client, Standalone ) and
        //! in case of Client the remote flag determines local or remote client.
        //! Returns false if something went wrong.
        bool                                        getPlayerConfig( unsigned int mode, bool remote, std::string& cfgfile );

        //! Stores a pointer to local player entity
        void                                        setLocalPlayer( BaseEntity* p_entity );

        //! Return the previousely set local player entity
        BaseEntity*                                 getLocalPlayer();

        //! Add a new remote player ( ghost ) into internal list
        void                                        addRemotePlayer( BaseEntity* p_entity );

        //! Remove a remote player from internal list
        void                                        removeRemotePlayer( BaseEntity* p_entity );

        //! Return the list of remote players
        inline std::vector< BaseEntity* >&          getRemotePlayers();

    protected:

        BaseEntity*                                 _p_localPlayer;
    
        std::vector< BaseEntity* >                  _remotePlayers;

    friend public CTD::Singleton< CTD::gameutils::PlayerUtils >;
};

inline std::vector< BaseEntity* >& PlayerUtils::getRemotePlayers()
{                                                     
    return _remotePlayers;
}

//! Helper class for getting a lookup table with level files and their preview images
class LevelFiles
{
    public:

        //! Given a directory all preview images are gathered in a lookup table
                                                    LevelFiles( const std::string& dir );

        virtual                                     ~LevelFiles();

        //! Given a file name return its preview image. NULL if the file or preview pic does not exist.
        CEGUI::Image*                               getImage( const std::string& file );

        //! Get look up table
        std::map< std::string, CEGUI::Image* >&     getAllFiles() { return _files; }

        //! Get count of level files
        unsigned int                                count() { return _files.size(); }

    protected:

        std::map< std::string, CEGUI::Image* >      _files;
};

} // namespace gameutils
} // namespace CTD

#endif //_CTD_GAMEUTILS_H_
