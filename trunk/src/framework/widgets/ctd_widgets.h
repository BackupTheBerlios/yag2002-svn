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
 # neoengine, widget system basing on GLO (http://www.fscnation.com/~glo/)
 #
 # this class implements a widget system
 #
 #
 #   date of creation:  09/04/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_WIDGETS_H_
#define _CTD_WIDGETS_H_

#include <ctd_frbase.h>

// include glo headers only if needed, e.g. ctd_frameworkcore.cpp does not need them
#ifndef CTD_NONEED_GLO_HEADERS

// set ./GGCode/Source/GGLib/ in your include path
#include <GUI/guiCallback.h>
#include <GUI/guiCWidget.h>
#include <GUI/guiCEdit.h>
#include <GUI/guiCButton.h>
#include <GUI/guiCScreen.h>
#include <GUI/guiCWindow.h>
#include <GUI/guiCallback.h>
#include <Utility/utlMacros.h>
#include <Utility/utlVector.h>
#include <Utility/utlPulseTypes.h>
#include <Utility/utlARGB.h>
#include <Utility/utlError.h>
#include <Utility/utlString.h>
#include <Render/rendCBase.h>
#include <Render/rendDrawable.h>
#include <Render/rendMesh.h>
#include <Data/dataManager_S.h>
#include <Data/Managed/dataCallback.h>
#include <Data/Managed/dataMesh.h>
#include <Data/Managed/dataFont.h>

#endif // CTD_NONEED_GLO_HEADERS



namespace CTD
{

class GLOAdaptor;


// some usefull macros
//--------------------------------------------------------//

// register a callback function for a widget with name 'widget_name'
// the callback type can be one of following:
/*
     CM_NONE    
     CM_SYSTEM  
     CM_SYSTEM1 
     CM_SYSTEM2 
     CM_SYSTEM3 
     CM_SYSTEM4 
     CM_SYSTEM5 
     CM_SYSTEM6 
     CM_SYSTEM7 
     CM_SYSTEM8  
     CM_INPUT   
     CM_GUI     
     CM_USER    
*/

#define CTD_WIDGET_CALLBACK( widget_name, called_class, function, type )   { new dataCallback( ( widget_name ), new GuiC_Callback< called_class >( this, function, type ) ); }
//--------------------------------------------------------//

//! Windget input   
/**
* Base class for widget input callbacks. Derive your class from this one in order to be able to 
*  be added to input callback list.
*/
class WidgetInput
{

    public:

        virtual void                                    OnInput( const NeoEngine::InputEvent *pkEvent ) = 0;

};

class FrameworkCore;

//! Widgets system
/**
* Widget core
* This core uses GLO's gui system
*/
class Widgets: public NeoEngine::UpdateEntity 
{

    public:

        /** 
        * Initialize the gui system
        * \return                                       true if successfull
        */
        bool                                            Initialize();

        /**
        * Load the gui definitions from given file ( XML format )
        * \return                                       true if successful
        */
        bool                                            Load( const std::string& strFileName );

        /**
        * Update gui system. This must be called in every loop tick as long the gui is active.
        */
        void                                            Update();

        /**
        * Add an input callback object. Its class must inherit from WidgetInput
        */
        void                                            AddInputCallback( WidgetInput* pkCallbackObject );

        /**
        * Remove an input callback function.
        */
        void                                            RemoveInputCallback( WidgetInput* pkCallbackObject );

        /** 
        * Get the singlton object
        */
        static Widgets*                                 Get() 
                                                        {           
                                                            if ( s_pkSingletonWidgets == NULL ) s_pkSingletonWidgets = new Widgets;
                                                            return s_pkSingletonWidgets;
                                                        }

    protected:

                                                        Widgets();

                                                        ~Widgets();

        /** 
        * Shutdown the gui system
        */
        void                                            Shutdown();

        static Widgets                                  *s_pkSingletonWidgets;

        bool                                            m_bInitialized;

        GLOAdaptor                                      *m_pImpGLOAdaptor;

    friend class FrameworkImplClient;
    friend class FrameworkImplStandalone;

};


} // namespace CTD

#endif //_CTD_WIDGETS_H_

