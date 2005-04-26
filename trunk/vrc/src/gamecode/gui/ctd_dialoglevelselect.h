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
 # dialog constrol for selecting a level
 #
 #   date of creation:  04/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_DIALOGLEVELSELECT_H_
#define _CTD_DIALOGLEVELSELECT_H_

#include <ctd_base.h>
#include <ctd_guimanager.h>
#include <ctd_utils.h>

namespace CTD
{

class DialogGameSettings;
class EnAmbientSound;


class DialogLevelSelect
{
    public:
                                                    DialogLevelSelect();

        virtual                                     ~DialogLevelSelect();

        //! Initialize gui layout, return false when something went wrong.
        bool                                        initialize( const std::string& layoutfile, CEGUI::Window* p_parent );

        //! Update dialog control
        void                                        update( float deltaTime );

        //! Show/hide the dialog
        void                                        show( bool visible );

        //! Set click sound object
        void                                        setClickSound( EnAmbientSound* p_sound );

    protected:

        //! Setup all controls when the dialog is opening ( show( true ) )
        void                                        setupControls();

        //! Set the preview image for given list item ( is used when the user changes the level selection in list )
        void                                        setPreviewPic( CEGUI::ListboxItem* p_item );

        //! Dialog callback for Start button        
        bool                                        onClickedStart( const CEGUI::EventArgs& arg );

        //! Dialog callback for Return button        
        bool                                        onClickedReturn( const CEGUI::EventArgs& arg );
    
        //! Dialog callback for changed list item selection        
        bool                                        onListItemSelChanged( const CEGUI::EventArgs& arg );
        //  internal variables

        CEGUI::Window*                              _p_parent;

        EnAmbientSound*                             _p_clickSound;

        CEGUI::Window*                              _p_levelSelectDialog;

        CEGUI::Listbox*                             _p_listbox;

        CEGUI::StaticImage*                         _p_image;

        //! Lookup for available level file names and their preview pics
        std::map< std::string, CEGUI::Image* >      _levelFiles;
};

}

#endif // _CTD_DIALOGLEVELSELECT_H_
