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
 # neoengine, framework builder for all game modes
 #
 #
 #   date of creation:  09/25/2004
 #
 #
 #   date of creation:  11/15/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#ifndef _CTD_FRAMEWORK_BUILDER_H_
#error "do not include this file! this is an inline header."
#endif

template< class FrameworkImplementationT >
FrameworkBuilder< FrameworkImplementationT >*   FrameworkBuilder< FrameworkImplementationT >::s_pkSingletonFrameworkBuilder = NULL;


template< class FrameworkImplementationT >
FrameworkBuilder< FrameworkImplementationT >::FrameworkBuilder()
{

    m_pkFrameworkImpl       = NULL;
    m_pkFramework           = NULL;
    m_stateBuild            = eNone;

}

template< class FrameworkImplementationT >
FrameworkBuilder< FrameworkImplementationT >::~FrameworkBuilder()
{
}

template< class FrameworkImplementationT >
inline void FrameworkBuilder< FrameworkImplementationT >::Create()
{

    if ( m_stateBuild != eNone ) {

        throw( FrameworkException( "The builder is already in building process!" ) );
        return;
    }

    m_pkFrameworkImpl = new FrameworkImplementationT();

    m_stateBuild = eCreated;

}

template< class FrameworkImplementationT >
inline void FrameworkBuilder< FrameworkImplementationT >::Initialize( int iArgc, char **ppArgv )
{

    if ( m_stateBuild != eCreated ) {

        throw( FrameworkException( "You can initialize the framework only after creation!" ) );
        return;

    }

    m_pkFrameworkImpl->Initialize( iArgc, ppArgv );

    m_stateBuild = stateInitialized;

}

template< class FrameworkImplementationT >
inline Framework* FrameworkBuilder< FrameworkImplementationT >::Finalize()
{

    if ( m_stateBuild != stateInitialized ) {

        throw( FrameworkException( "Framework is not initialized!" ) );
        return NULL;

    }
    
    // create the frame work bridge and return it
    m_pkFramework = new Framework( m_pkFrameworkImpl );
    Framework::s_pkSingletonFramework = m_pkFramework;

    return m_pkFramework;

}

// shutdown game
template< class FrameworkImplementationT >
inline void FrameworkBuilder< FrameworkImplementationT >::Shutdown()
{
    
    delete m_pkFrameworkImpl;
    m_pkFrameworkImpl               = NULL;
    delete m_pkFramework;
    m_pkFramework                   = NULL;

    delete s_pkSingletonFrameworkBuilder;
    s_pkSingletonFrameworkBuilder   = NULL;

}
