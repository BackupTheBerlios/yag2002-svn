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
 # player implementation for standalone mode
 #
 #   date of creation:  05/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/28/2005 boto       creation of PlayerImplStandalone
 #
 ################################################################*/

#ifndef _CTD_PLAYERIMPLSTANDALONE_H_
#define _CTD_PLAYERIMPLSTANDALONE_H_

#include <ctd_main.h>
#include "ctd_playerimpl.h"

namespace CTD
{

class EnPlayerSound;
class PlayerIHStandalone;
class PlayerChatGui;

//! Player implementation for game mode Standalone ( see framework class GameState )
class BasePlayerImplStandalone : public BasePlayerImplementation
{
    public:

                                                    BasePlayerImplStandalone( EnPlayer* player );

        virtual                                     ~BasePlayerImplStandalone();


        //! Initialize
        void                                        initialize();

        //! Post-initialize
        void                                        postInitialize();

        //! Update
        void                                        update( float deltaTime );

        //! Implementation's notification callback
        void                                        handleNotification( EntityNotification& notify );

        //! Set camera mode to Spheric or Ego
        void                                        setCameraMode( unsigned int mode );

        //! Set next available camera mode
        void                                        setNextCameraMode();

        //! Set camera's pitch and yaw angles
        void                                        setCameraPitchYaw( float pitch, float yaw );

        //! Enable / disable input processing and control
        void                                        enableControl( bool en );

    protected:

        //! Get the configuration settings
        void                                        getConfiguration();

        //! Chat gui
        std::auto_ptr< PlayerChatGui >              _p_chatGui;

        //! Input handler
        PlayerIHStandalone*                         _p_inputHandler;

        //! Enable / disable input processing
        bool                                        _enabledControl;

    friend PlayerIHStandalone;
};

} // namespace CTD

#endif // _CTD_PLAYERIMPLSTANDALONE_H_
