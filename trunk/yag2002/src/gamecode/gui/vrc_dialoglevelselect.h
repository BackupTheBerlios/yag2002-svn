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
 # dialog constrol for selecting a level
 #
 #   date of creation:  04/26/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_DIALOGLEVELSELECT_H_
#define _VRC_DIALOGLEVELSELECT_H_

#include <vrc_main.h>

namespace vrc
{

class DialogGameSettings;
class EnAmbientSound;
class EnMenu;

namespace gameutils
{
    class LevelFiles;
}

class DialogLevelSelect
{
    public:

        explicit                                    DialogLevelSelect( EnMenu* p_menuEntity );

        virtual                                     ~DialogLevelSelect();

        //! Initialize gui layout, return false if something went wrong.
        bool                                        initialize( const std::string& layoutfile );

        //! Update dialog control
        void                                        update( float deltaTime );

        //! Set the search directory for finding level files
        void                                        setSearchDirectory( const std::string& dir );

        //! Enable / disable dialog, call setSearchDirectory before.
        void                                        enable( bool en );

    protected:

        //! Changes the search directory for finding level files
        void                                        changeToSearchDirectory();

        //! Setup all controls when the dialog is opening ( show( true ) )
        void                                        setupControls();

        //! Release gui resources such as preview pics etc.
        void                                        destroyResources();

        //! Set the preview image for given list item ( is used when the user changes the level selection in list )
        void                                        setPreviewPic( CEGUI::ListboxItem* p_item );

        //! Dialog callback for Start button        
        bool                                        onClickedStart( const CEGUI::EventArgs& arg );

        //! Dialog callback for Return button        
        bool                                        onClickedReturn( const CEGUI::EventArgs& arg );
    
        //! Dialog callback for changed list item selection        
        bool                                        onListItemSelChanged( const CEGUI::EventArgs& arg );
        //  internal variables

        CEGUI::Window*                              _p_levelSelectDialog;

        CEGUI::Listbox*                             _p_listbox;

        CEGUI::StaticImage*                         _p_image;

        CEGUI::ListboxItem*                         _p_lastListSelection;

        //! Lookup for available level file names and their preview pics
        gameutils::LevelFiles*                      _p_levelFiles;

        //! Current selected file in list
        std::string                                 _currentSelection;

        EnMenu*                                     _p_menuEntity;

        std::string                                 _searchDirectory;

        bool                                        _enable;
};

}

#endif // _VRC_DIALOGLEVELSELECT_H_
