/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # game navigation input handler
 #
 #   date of creation:  02/01/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NAVIGATIONINPUT_H_
#define _NAVIGATIONINPUT_H_

#include <vrc_main.h>


class GameNavigator;

//! Input handler for game navigation
class GNInputHandler : public osgGA::GUIEventHandler
{
    public:


                                                        GNInputHandler();

        virtual                                         ~GNInputHandler();

        //! Set navigation mode, one of NavigationMode enums.
        void                                            setMode( unsigned int mode );

        //! Get current navigation mode.
        unsigned int                                    getMode() const;

    protected:

        //! Input handler's callback
        bool                                            handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! Navigation mode
        unsigned int                                    _mode;
};

#endif // _NAVIGATIONINPUT_H_
