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
 # neoengine, gui system basing on GLO
 #
 # this class implements gui system's initialization functions
 #
 #
 #   date of creation:  08/22/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_frbase.h>

#include "ctd_widgets.h"
#include "ctd_glo-adaptor.h"

namespace CTD
{

Widgets *Widgets::s_pkSingletonWidgets = NULL;


Widgets::Widgets()
{

    m_bInitialized      = false;
    
    m_pImpGLOAdaptor    = new GLOAdaptor;

    // the default is gui system not active
    Deactivate();

}

Widgets::~Widgets()
{
}

bool Widgets::Initialize()
{

    m_pImpGLOAdaptor->Initialize();

    m_bInitialized = true;

    return true;

}

void Widgets::Shutdown()
{

    if ( m_bInitialized == false ) {

        return;

    }

    m_pImpGLOAdaptor->Shutdown();

    delete s_pkSingletonWidgets;
    s_pkSingletonWidgets = NULL;

}

void Widgets::Update()
{

    if ( IsActive() ) {

        m_pImpGLOAdaptor->Update();

    }

}

bool Widgets::Load( const std::string& strFileName )
{

    return m_pImpGLOAdaptor->LoadXmlFile( strFileName );

}

void Widgets::AddInputCallback( WidgetInput* pkCallbackObject )
{

    m_pImpGLOAdaptor->GetInputManager()->AddInputCallback( pkCallbackObject );

}

void Widgets::RemoveInputCallback( WidgetInput* pkCallbackObject )
{

    m_pImpGLOAdaptor->GetInputManager()->RemoveInputCallback( pkCallbackObject );

}

} // namespace CTD

