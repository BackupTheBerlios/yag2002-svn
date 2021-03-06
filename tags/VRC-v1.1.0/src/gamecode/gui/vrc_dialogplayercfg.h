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
 # dialog control for configuring the player
 #
 #   date of creation:  06/07/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_DIALOGPLAYERCFG_H_
#define _VRC_DIALOGPLAYERCFG_H_

#include <vrc_main.h>

namespace vrc
{

class EnAmbientSound;
class DialogGameSettings;

class DialogPlayerConfig
{
    public:
                                                    DialogPlayerConfig( DialogGameSettings* p_menuEntity );

        virtual                                     ~DialogPlayerConfig();

        //! Initialize gui layout, return false when something went wrong.
        bool                                        initialize( const std::string& layoutfile );

        //! Update dialog control
        void                                        update( float deltaTime );

        //! Show/hide the dialog
        void                                        show( bool visible );

    protected:

        //! Setup all controls when the dialog is opening ( show( true ) )
        void                                        setupControls();

        //! Set the preview image for given list item ( is used when the user changes the player selection in list )
        void                                        setPreviewPic( const CEGUI::ListboxItem* p_item );

        //! Dialog callback for Ok button        
        bool                                        onClickedOk( const CEGUI::EventArgs& arg );

        //! Dialog callback for Cancel button        
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );
    
        //! Dialog callback for changed list item selection        
        bool                                        onListItemSelChanged( const CEGUI::EventArgs& arg );
       
        //  internal variables
        CEGUI::Window*                              _p_playerConfigDialog;

        CEGUI::Listbox*                             _p_listbox;

        CEGUI::Editbox*                             _p_playerName;

        CEGUI::StaticImage*                         _p_image;

        CEGUI::ListboxItem*                         _p_lastListSelection;

        //! Lookup for available player types and their preview pics
        std::map< std::string, CEGUI::Image* >      _players;

        //! Lookup for player type names and their associated configuration files < type name, cfg file >
        std::map< std::string, std::string >        _cfgFiles;

        //! Current selected player in list
        std::string                                 _currentSelection;

        DialogGameSettings*                         _p_settingsDialog;
};

}

#endif // _VRC_DIALOGPLAYERCFG_H_
