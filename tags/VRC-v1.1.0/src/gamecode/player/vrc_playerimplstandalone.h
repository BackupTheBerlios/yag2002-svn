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

#ifndef _VRC_PLAYERIMPLSTANDALONE_H_
#define _VRC_PLAYERIMPLSTANDALONE_H_

#include <vrc_main.h>
#include "vrc_playerimpl.h"

namespace vrc
{

template< class PlayerImplT > class PlayerIHCharacterCameraCtrl;

//! Player implementation for game mode Standalone ( see framework class yaf3d::GameState )
class PlayerImplStandalone : public BasePlayerImplementation
{
    public:

        explicit                                    PlayerImplStandalone( EnPlayer* player );

        virtual                                     ~PlayerImplStandalone();


        //! Initialize
        void                                        initialize();

        //! Post-initialize
        void                                        postInitialize();

        //! Update
        void                                        update( float deltaTime );

        //! Implementation's notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

    protected:

        //! Get the player relevant configuration settings
        void                                        getConfiguration();

        //! Input handler
        PlayerIHCharacterCameraCtrl< PlayerImplStandalone >* _p_inputHandler;

    friend class PlayerIHCharacterCameraCtrl< PlayerImplStandalone >;
};

} // namespace vrc

#endif // _VRC_PLAYERIMPLSTANDALONE_H_
