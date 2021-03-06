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
 # intro played at startup or triggered by main menu
 #
 #   date of creation:  04/27/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_INTRO_H_
#define _VRC_INTRO_H_

#include <vrc_main.h>

namespace vrc
{

class En2DSound;

class IntroControl
{
    public:
                                                    IntroControl();

        virtual                                     ~IntroControl();

        //! Initialize gui layout, return false when something went wrong.
        bool                                        initialize( const std::string& intoImage );

        //! Update
        void                                        update( float deltaTime );

        //! Start intro
        void                                        start();

        //! Stop intro, after stopping call initialize before re-"start"ing.
        void                                        stop();

        //! Set intro sound object, this sound is played on intro start.
        void                                        setIntroSound( En2DSound* p_sound );

        //! Set the time scale for opening intro picture
        void                                        setIntroTimeScale( float scale );

    protected:

        //! Internal states
        enum
        {
            None,
            Running,
            Stopped
        }                                           _introState;

        En2DSound*                                  _p_introSound;

        CEGUI::Window*                              _p_wndIntro;

        CEGUI::StaticImage*                         _p_introImage;

        CEGUI::Size                                 _intoImageSize;

        CEGUI::Imageset*                            _p_imageSet;

        float                                       _introTimer;

        float                                       _introTimeScale;
};

}

#endif // _VRC_INTRO_H_
