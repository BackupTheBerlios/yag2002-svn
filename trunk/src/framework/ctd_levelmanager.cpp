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
 # neoengine, level manager
 #
 #
 #   date of creation:  10/03/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_levelmanager.h"
#include <ctd_pluginmanager.h>

using namespace std;

namespace CTD
{


LevelManager::LevelManager()
{
    m_uiNumSets = 0;
    memset( m_apkLevelSets, 0, CTD_MAX_LEVELSETS * sizeof( LevelSet* ) );
}

LevelManager::~LevelManager()
{

}

bool LevelManager::Add(  LevelSet* pkSet )
{

    bool bAlreadyExists = false;

    // check whether the name already exists
    for( unsigned int uiSetCnt = 0; uiSetCnt < m_uiNumSets; uiSetCnt++ ) {

        if ( m_apkLevelSets[ uiSetCnt ]->m_strName == pkSet->m_strName ) {

            bAlreadyExists = true;
            break;

        }

    }

    if ( ( bAlreadyExists == true ) && ( m_uiNumSets > 0 ) ) {

        return false;

    }

    m_apkLevelSets[ m_uiNumSets ] = pkSet ;
    m_uiNumSets++;

    return true;

}

bool LevelManager::Destroy( const string& strName )
{

    bool bFound = false;

    // search for set name
   unsigned int uiSetCnt;
   for( uiSetCnt = 0; uiSetCnt < m_uiNumSets; uiSetCnt++ ) {

       if ( m_apkLevelSets[ uiSetCnt ]->m_strName == strName ) {

            bFound = true;
            break;

        }

    }

    if ( bFound == false ) {

        return false;

    }

    delete m_apkLevelSets[ uiSetCnt ]->m_pkPluginMgr;
    delete m_apkLevelSets[ uiSetCnt ]->m_pkRoom;
    m_apkLevelSets[ uiSetCnt ] = 0;

    m_uiNumSets--;

    return true;

}

void LevelManager::DestroyAll()
{

   for( unsigned int uiSetCnt = 0; uiSetCnt < m_uiNumSets; uiSetCnt++ ) {

        delete m_apkLevelSets[ uiSetCnt ]->m_pkPluginMgr;
        delete m_apkLevelSets[ uiSetCnt ]->m_pkRoom;

    }

    m_uiNumSets = 0;

}

LevelSet* LevelManager::GetLevelSet( const string& strName )
{

    bool bFound = false;
    unsigned int uiSetCnt;
    for( uiSetCnt = 0; uiSetCnt < m_uiNumSets; uiSetCnt++ ) {

        if ( m_apkLevelSets[ uiSetCnt ]->m_strName == strName ) {

            bFound = true;
            break;

        }
    }

    if ( bFound == true ) {

        return m_apkLevelSets[ uiSetCnt ];

    }

    return NULL;

}

void LevelManager::SetUpdateFlag( const string& strName, bool bFlag )
{

    bool bFound = false;
    unsigned int uiSetCnt;
    for( uiSetCnt = 0; uiSetCnt < m_uiNumSets; uiSetCnt++ ) {

        if ( m_apkLevelSets[ uiSetCnt ]->m_strName == strName ) {

            bFound = true;
            break;

        }
    }

    if ( bFound == true ) {

        m_apkLevelSets[ uiSetCnt ]->m_bCanUpdate = bFlag;

    }

}

void LevelManager::SetRenderFlag( const string& strName, bool bFlag )
{

    bool bFound = false;
    unsigned int uiSetCnt;
    for( uiSetCnt = 0; uiSetCnt < m_uiNumSets; uiSetCnt++ ) {

        if ( m_apkLevelSets[ uiSetCnt ]->m_strName == strName ) {

            bFound = true;
            break;

        }
    }

    if ( bFound == true ) {

        m_apkLevelSets[ uiSetCnt ]->m_bCanRender = bFlag;

    }

}

LevelSet**  LevelManager::GetAllLevelSets()
{

    return m_apkLevelSets;

}

} // namespace CTD
